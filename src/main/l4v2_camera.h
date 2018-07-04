#ifndef __L4V2_CAMERA_DEF_H__
#define __L4V2_CAMERA_DEF_H__

#include "common_def.h"

typedef struct
{
  uint8_t*    start;
  size_t      length;
} l4v2_camera_buffer_t;

typedef struct
{
  int                     fd;
  uint32_t                width;
  uint32_t                height;
  size_t                  buffer_count;
  l4v2_camera_buffer_t*   buffers;
  l4v2_camera_buffer_t    head;
} l4v2_camera_t;

extern int l4v2_camera_open(l4v2_camera_t* cam, const char* device, uint32_t width, uint32_t height, size_t buffer_count);
extern void l4v2_camera_close(l4v2_camera_t* cam);
extern int l4v2_camera_start(l4v2_camera_t* cam);
extern int l4v2_camera_stop(l4v2_camera_t* cam);
extern int l4v2_camera_capture(l4v2_camera_t* cam);

#endif /* !__L4V2_CAMERA_DEF_H__ */
