#include <stdlib.h>

#include "common_def.h"
#include "io_driver.h"
#include "v4l2_camera.h"
#include "camera_driver.h"

extern io_driver_t* main_io_driver(void);

static const char* TAG = "cam_driver";

static v4l2_camera_t  _cam;
static io_driver_watcher_t    _cam_watcher;
static struct list_head       _listeners;

static void
camera_capture_callback(io_driver_watcher_t* watcher, io_driver_event event)
{
  camera_driver_listener_t* l;

  v4l2_camera_capture(&_cam);

  list_for_each_entry(l, &_listeners, le)
  {
    l->cb(_cam.head.start, _cam.head.length);
  }
}

void
camera_driver_init(void)
{
  if(v4l2_camera_open(&_cam, "/dev/video0", 640, 480, 8))
  {
    LOGE(TAG, "v4l2_camera_open failed\n");
    exit(-1);
  }

  io_driver_watcher_init(&_cam_watcher);

  _cam_watcher.fd       = _cam.fd;
  _cam_watcher.callback = camera_capture_callback;

  INIT_LIST_HEAD(&_listeners);
}

void
camera_driver_start(void)
{
  io_driver_watch(main_io_driver(), &_cam_watcher, IO_DRIVER_EVENT_RX);
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
camera_driver_get_brightness(void)
{
  return v4l2_camera_get_brightness(&_cam);
}

void
camera_driver_set_brightness(int v)
{
  v4l2_camera_set_brightness(&_cam, v);
}

int
camera_driver_get_saturation(void)
{
  return v4l2_camera_get_saturation(&_cam);
}

void
camera_driver_set_saturation(int v)
{
  v4l2_camera_set_saturation(&_cam, v);
}

int
camera_driver_get_hue(void)
{
  return v4l2_camera_get_hue(&_cam);
}

void
camera_driver_set_hue(int v)
{
  v4l2_camera_set_hue(&_cam, v);
}

int
camera_driver_get_exposure(void)
{
  return v4l2_camera_get_exposure(&_cam);
}

void
camera_driver_set_exposure(int v)
{
  v4l2_camera_set_exposure(&_cam, v);
}
