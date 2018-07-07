#include <stdlib.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "common_def.h"
#include "io_driver.h"
#include "v4l2_camera.h"
#include "camera_driver.h"

extern io_driver_t* main_io_driver(void);

static const char* TAG = "cam_driver";

static v4l2_camera_t  _cam;
static io_driver_watcher_t    _cam_watcher;
static struct list_head       _listeners;

static int _timerfd;
static io_driver_watcher_t    _timerfd_watcher;

static int _fps = 0;
static int _fps_count = 0;

static void
camera_capture_callback(io_driver_watcher_t* watcher, io_driver_event event)
{
  camera_driver_listener_t* l;

  v4l2_camera_capture(&_cam);

  _fps_count++;

  // LOGI(TAG, "capture: length: %d\n", _cam.head.length);
  list_for_each_entry(l, &_listeners, le)
  {
    l->cb(_cam.head.start, _cam.head.length);
  }
}

static void
_1sec_elapsed_callback(io_driver_watcher_t* watcher, io_driver_event event)
{
  uint64_t      v;

  if(read(_timerfd, &v, sizeof(v)) != sizeof(v))
  {
    LOGE(TAG, "read _timerfd failed\n");
  }

  _fps = _fps_count;
  _fps_count = 0;
  // LOGI(TAG, "fps = %d\n", _fps);
}

static void
init_timerfd(void)
{
  struct itimerspec new_value;

  _timerfd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC);
  if(_timerfd < 0)
  {
    LOGE(TAG, "failed to create timerfd:%s\n", strerror(errno));
    return;
  }

  new_value.it_interval.tv_sec    = 1;
  new_value.it_interval.tv_nsec   = 0;
  new_value.it_value = new_value.it_interval;

  if(timerfd_settime(_timerfd, 0, &new_value, NULL) != 0)
  {
    LOGE(TAG, "timerfd_settime failed: %s\n", strerror(errno));
    close(_timerfd);
    return;
  }

  _timerfd_watcher.fd = _timerfd;
  _timerfd_watcher.callback = _1sec_elapsed_callback;
}

void
camera_driver_init(void)
{
  if(v4l2_camera_open(&_cam, "/dev/video0", 640, 480, 4))
  {
    LOGE(TAG, "v4l2_camera_open failed\n");
    exit(-1);
  }

  io_driver_watcher_init(&_cam_watcher);

  _cam_watcher.fd       = _cam.fd;
  _cam_watcher.callback = camera_capture_callback;

  INIT_LIST_HEAD(&_listeners);

  init_timerfd();
}

void
camera_driver_start(void)
{
  io_driver_watch(main_io_driver(), &_cam_watcher, IO_DRIVER_EVENT_RX);
  io_driver_watch(main_io_driver(), &_timerfd_watcher, IO_DRIVER_EVENT_RX);
  v4l2_camera_start(&_cam);
}

void
camera_driver_stop(void)
{
  io_driver_no_watch(main_io_driver(), &_cam_watcher, IO_DRIVER_EVENT_RX);
  v4l2_camera_stop(&_cam);
}

void
camera_driver_listen(camera_driver_listener_t* listener)
{
  INIT_LIST_HEAD(&listener->le);
  list_add_tail(&listener->le, &_listeners);
}

int
camera_driver_get_control(camera_control_t c)
{
  int ret = -1;

  switch(c)
  {
  case CAMERA_CONTROL_BRIGHTNESS:
    ret = v4l2_camera_get_brightness(&_cam);
    break;

  case CAMERA_CONTROL_SATURATION:
    ret = v4l2_camera_get_saturation(&_cam);
    break;

  case CAMERA_CONTROL_HUE:
    ret = v4l2_camera_get_hue(&_cam);
    break;

  case CAMERA_CONTROL_EXPOSURE:
    ret = v4l2_camera_get_exposure(&_cam);
    break;

  case CAMERA_CONTROL_HFLIP:
    ret = v4l2_camera_get_hflip(&_cam);
    break;
  }
  return ret;
}

int
camera_driver_set_control(camera_control_t c, int v)
{
  int ret = -1;

  switch(c)
  {
  case CAMERA_CONTROL_BRIGHTNESS:
    ret = v4l2_camera_set_brightness(&_cam, v);
    break;

  case CAMERA_CONTROL_SATURATION:
    ret = v4l2_camera_set_saturation(&_cam, v);
    break;

  case CAMERA_CONTROL_HUE:
    ret = v4l2_camera_set_hue(&_cam, v);
    break;

  case CAMERA_CONTROL_EXPOSURE:
    ret = v4l2_camera_set_exposure(&_cam, v);
    break;

  case CAMERA_CONTROL_HFLIP:
    ret = v4l2_camera_set_hflip(&_cam, v);
    break;
  }
  return ret;
}

int
camera_driver_get_fps(void)
{
  return _fps;
}
