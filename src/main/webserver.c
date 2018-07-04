#include <pthread.h>
#include "common_def.h"
#include "mongoose.h"
#include "completion.h"
#include "camera_driver.h"

typedef struct
{
  uint8_t*    buf;
  size_t      len;
} camera_frame_t;

struct mg_mgr                     _mongoose_mgr;
static pthread_t                  _webserver_thread;
static struct mg_serve_http_opts  s_http_server_opts = 
{
  .document_root  = "./",
  .enable_directory_listing = "yes",
};
static struct mg_connection*      _server_conn;

const static char*    TAG = "WEB";

static camera_driver_listener_t   _cam_listener;

static void
cam_feed_response_begin(struct mg_connection* nc)
{
   mg_printf(nc, "%s",
       "HTTP/1.0 200 OK\r\n" "Cache-Control: no-cache, private\r\n"
       "Pragma: no-cache\r\n"
       "Content-Type: multipart/x-mixed-replace; boundary=FRAME\r\n\r\n");
}

static void
cam_feed_response_frame(struct mg_connection* nc, camera_frame_t* frame)
{
  mg_printf(nc, "%s",
      "--FRAME\r\n"
      "Content-Type: image/jpeg\r\n");
  mg_printf(nc, "Content-Length: %ld\r\n\r\n", frame->len);
  mg_send(nc, frame->buf, frame->len);
}

static void
ev_handler(struct mg_connection* nc, int ev, void* ev_data)
{   
  struct http_message* hm = (struct http_message*)ev_data;

  switch(ev)
  {
  case MG_EV_HTTP_REQUEST:
    if(mg_vcmp(&hm->uri, "/") == 0)
    {
      mg_http_send_redirect(nc, 302, mg_mk_str("/index.html"), mg_mk_str(NULL));
    }
    else if(mg_vcmp(&hm->uri, "/cam_feed.mjpg") == 0)
    {
      nc->flags |= MG_F_USER_6;
      cam_feed_response_begin(nc);
    }
    else
    {
      mg_serve_http(nc, hm, s_http_server_opts);
    }
    break;

  default:
    break;
  }
} 

static void
cam_feed_handler(struct mg_connection* nc, int ev, void* ev_data)
{
  camera_frame_t*   frame = (camera_frame_t*)ev_data;
  
  if(ev != MG_EV_POLL)
  {
    return;
  }

  if(nc == _server_conn)
  {
    return;
  }

  if((nc->flags & MG_F_USER_6) == 0)
  {
    return;
  }

  // LOGI(TAG, "cam feed event: %p\n", nc);
  cam_feed_response_frame(nc, frame);
}


static void*
__webserver_thread(void* arg)
{
  completion_t* bc_comp = (completion_t*)arg;

  LOGI(TAG, "starting up web server\n"); 
  mg_mgr_init(&_mongoose_mgr, NULL);

  _server_conn = mg_bind(&_mongoose_mgr, "8080", ev_handler);
  if(_server_conn == NULL)
  {
    LOGE(TAG, "mg_bind failed\n");
    exit(-1);
  }

  mg_set_protocol_http_websocket(_server_conn);

  completion_signal(bc_comp);

  while(1)
  {
    mg_mgr_poll(&_mongoose_mgr, 1000);
  }

  mg_mgr_free(&_mongoose_mgr);
}

static void
camera_frame_event(uint8_t* buf, size_t length)
{
  camera_frame_t    frame;

  frame.buf = buf;
  frame.len = length;

  mg_broadcast(&_mongoose_mgr, cam_feed_handler, &frame, sizeof(frame));
}

void
webserver_init(void)
{
  static completion_t bootup_complete;

  completion_init(&bootup_complete);

  _cam_listener.cb = camera_frame_event;
  camera_driver_listen(&_cam_listener);

  pthread_create(&_webserver_thread, NULL, __webserver_thread, &bootup_complete);

  completion_wait(&bootup_complete);
  completion_deinit(&bootup_complete);
}

void
webserver_feed_cam_data(void)
{
#if 0
  static int c = 1;

  c++;
  mg_broadcast(&_mongoose_mgr, cam_feed_handler, &c, sizeof(c));
#endif
}
