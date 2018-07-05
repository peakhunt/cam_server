#ifndef __V4L2_CAMERA_DEF_H__
#define __V4L2_CAMERA_DEF_H__

#include "common_def.h"

typedef struct
{
  uint8_t*    start;
  size_t      length;
} v4l2_camera_buffer_t;

typedef struct
{
  int                     fd;
  uint32_t                width;
  uint32_t                height;
  size_t                  buffer_count;
  v4l2_camera_buffer_t*   buffers;
  v4l2_camera_buffer_t    head;
} v4l2_camera_t;

extern int v4l2_camera_open(v4l2_camera_t* cam, const char* device, uint32_t width, uint32_t height, size_t buffer_count);
extern void v4l2_camera_close(v4l2_camera_t* cam);
extern int v4l2_camera_start(v4l2_camera_t* cam);
extern int v4l2_camera_stop(v4l2_camera_t* cam);
extern int v4l2_camera_capture(v4l2_camera_t* cam);

#endif /* !__V4L2_CAMERA_DEF_H__ */
