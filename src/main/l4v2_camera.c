#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
  
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <asm/types.h>
#include <linux/videodev2.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "l4v2_camera.h"

const char* TAG = "l4v2_camera";

static int
l4v2_camera_set_format(l4v2_camera_t* cam)
{
  struct v4l2_format format;

  memset(&format, 0, sizeof format);

  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  format.fmt.pix.width = cam->width;
  format.fmt.pix.height = cam->height;
  format.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
  format.fmt.pix.field = V4L2_FIELD_NONE;

  if(ioctl(cam->fd, VIDIOC_S_FMT, &format) == -1){
    LOGE(TAG, "setting pixel format failed\n");
    return -1;
  }
  return 0;
}

static int
l4v2_camera_set_request_buffers(l4v2_camera_t* cam)
{
  struct v4l2_requestbuffers req;

  memset(&req, 0, sizeof req);

  req.count = cam->buffer_count;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;

  if(ioctl(cam->fd, VIDIOC_REQBUFS, &req) == -1)
  {
    LOGE(TAG, "VIDIOC_REQBUFS, failed\n");
    return -1;
  }
  cam->buffers = calloc(req.count, sizeof (l4v2_camera_buffer_t));

  size_t buf_max = 0;

  for(size_t i = 0; i < cam->buffer_count; i++)
  {
    cam->buffers[i].start = MAP_FAILED;
  }

  for(size_t i = 0; i < cam->buffer_count; i++)
  {
    struct v4l2_buffer buf;

    memset(&buf, 0, sizeof buf);
    buf.type    = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory  = V4L2_MEMORY_MMAP;
    buf.index   = i;

    if(ioctl(cam->fd, VIDIOC_QUERYBUF, &buf) == -1)
    {
      LOGE(TAG, "VIDIOC_QUERYBUF failed\n");
      return -1;
    }

    if(buf.length > buf_max)
    {
      buf_max = buf.length;
    }

    cam->buffers[i].length = buf.length;
    cam->buffers[i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, cam->fd, buf.m.offset);
    if(cam->buffers[i].start == MAP_FAILED)
    {
      LOGE(TAG, "mmap failed: %ld\n", i);
      return -1;
    }
  }
  cam->head.start = malloc(buf_max);
  return 0;
}

static int
l4v2_camera_init(l4v2_camera_t* cam)
{
  struct v4l2_capability cap;
  struct v4l2_cropcap cropcap;

  if(ioctl(cam->fd, VIDIOC_QUERYCAP, &cap) == -1)
  {
    return -1;
  }

  if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
  {
    return -1;
  }

  if(!(cap.capabilities & V4L2_CAP_STREAMING))
  {
    return -1;
  }

  memset(&cropcap, 0, sizeof cropcap);
  cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if(ioctl(cam->fd, VIDIOC_CROPCAP, &cropcap) == 0)
  {
    struct v4l2_crop crop;

    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    crop.c = cropcap.defrect;
    if(ioctl(cam->fd, VIDIOC_S_CROP, &crop) == -1)
    {
      LOGI(TAG, "cropping not supported\n");
    }
  }

  if(l4v2_camera_set_format(cam) != 0 ||
     l4v2_camera_set_request_buffers(cam) != 0)
  {
    return -1;
  }

  return 0;
}

int
l4v2_camera_open(l4v2_camera_t* cam, const char* device, uint32_t width, uint32_t height, size_t buffer_count)
{
  int   fd;

  fd = open(device, O_RDWR | O_NONBLOCK, 0);
  if(fd == -1)
  {
    LOGE(TAG, "failed to open\n");
    return -1;
  }

  cam->fd           = fd;
  cam->width        = width;
  cam->height       = height;
  cam->buffer_count = buffer_count;
  cam->buffers      = NULL;
  cam->head.length  = 0;
  cam->head.start   = NULL;

  if(l4v2_camera_init(cam) != 0)
  {
    l4v2_camera_close(cam);
    return -1;
  }
  return 0;
}

void
l4v2_camera_close(l4v2_camera_t* cam)
{
  if(cam->buffers != NULL)
  {
    for(size_t i = 0; i < cam->buffer_count; i++)
    {
      if(cam->buffers[i].start != MAP_FAILED)
      {
        munmap(cam->buffers[i].start, cam->buffers[i].length);
      }
    }

    free(cam->buffers);
  }

  cam->buffer_count = 0;
  cam->buffers = NULL;

  if(cam->head.start != NULL)
  {
    free(cam->head.start);
  }

  cam->head.length = 0;
  cam->head.start = NULL;
}

int
l4v2_camera_start(l4v2_camera_t* cam)
{
  for(size_t i = 0; i < cam->buffer_count; i++)
  {
    struct v4l2_buffer buf;

    memset(&buf, 0, sizeof buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;
    
    if(ioctl(cam->fd, VIDIOC_QBUF, &buf) == -1)
    {
      LOGE(TAG, "VIDIOC_QBUF failed %ld\n", i);
      return -1;
    }
  }

  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if(ioctl(cam->fd, VIDIOC_STREAMON, &type) == -1)
  {
    LOGE(TAG, "VIDIOC_STREAMON failed\n");
    return -1;
  }
  return 0;
}

int
l4v2_camera_stop(l4v2_camera_t* cam)
{
  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if(ioctl(cam->fd, VIDIOC_STREAMOFF, &type) == -1)
  {
    LOGE(TAG, "VIDIOC_STREAMOFF failed\n");
    return -1;
  }
  return 0;
}

int
l4v2_camera_capture(l4v2_camera_t* cam)
{
  struct v4l2_buffer buf;

  memset(&buf, 0, sizeof buf);

  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;

  if(ioctl(cam->fd, VIDIOC_DQBUF, &buf) == -1)
  {
    LOGE(TAG, "VIDIOC_DQBUF failed\n");
    return -1;
  }
  
  memcpy(cam->head.start, cam->buffers[buf.index].start, buf.bytesused);
  cam->head.length = buf.bytesused;

  if(ioctl(cam->fd, VIDIOC_QBUF, &buf) == -1)
  {
    LOGE(TAG, "VIDIOC_QBUF failed\n");
    return -1;
  }

  return 0;
}
