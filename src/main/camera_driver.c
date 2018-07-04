#include <stdlib.h>

#include "common_def.h"
#include "io_driver.h"
#include "l4v2_camera.h"

extern io_driver_t* main_io_driver(void);

static const char* TAG = "cam_driver";

static l4v2_camera_t  _cam;
static io_driver_watcher_t    _cam_watcher;

static void
camera_capture_callback(io_driver_watcher_t* watcher, io_driver_event event)
{
  LOGI(TAG, "camera capture callback\n");
  l4v2_camera_capture(&_cam);
}

void
camera_driver_init(void)
{
  if(l4v2_camera_open(&_cam, "/dev/video0", 640, 480, 8))
  {
    LOGE(TAG, "l4v2_camera_open failed\n");
    exit(-1);
  }

  io_driver_watcher_init(&_cam_watcher);

  _cam_watcher.fd       = _cam.fd;
  _cam_watcher.callback = camera_capture_callback;
}

void
camera_driver_start(void)
{
  io_driver_watch(main_io_driver(), &_cam_watcher, IO_DRIVER_EVENT_RX);
  l4v2_camera_start(&_cam);
}

void
camera_driver_stop(void)
{
  io_driver_no_watch(main_io_driver(), &_cam_watcher, IO_DRIVER_EVENT_RX);
  l4v2_camera_stop(&_cam);
}
