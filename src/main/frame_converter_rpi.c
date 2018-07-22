#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <gd.h>

#include "common_def.h"
#include "camera_driver.h"
#include "frame_converter.h"

#include "brcmjpeg_encode.h"

#define MAX_WIDTH   5000
#define MAX_HEIGHT  5000
#define MAX_ENCODED (15*1024*1024)

typedef struct
{
  uint8_t         ptr[MAX_ENCODED];
  uint32_t        size;
} frame_converter_buffer_t;

#define MAX_FRAME_CONVERTER_BUFFERS   16

static gdImagePtr   _image;
static uint32_t     _width,
                    _height;

static uint32_t*    _image_buf;
static uint32_t**   _tpixels_buf;
static uint32_t**   _tpixels_org;

static frame_converter_buffer_t   _buffers[MAX_FRAME_CONVERTER_BUFFERS];
static uint32_t _buffers_ndx = 0;
uint32_t _last_frame_size = 0;

static camera_driver_listener_t   _cam_listener;

static struct list_head       _listeners;

void
__handle_overlay(void)
{
  char msg[128];
  int color = gdTrueColor(255, 255, 255);
  char* err;
  char* time;
  extern struct tm _current_time;


  time = asctime(&_current_time);
  sprintf(msg, "H & Petra's Studio %s", time);

  err = gdImageStringFT(_image, NULL, color, "/usr/share/fonts/truetype/freefont/FreeMono.ttf", 16.0, 0.0, 30, 30, msg);

  if(err)
  {
    LOGI("frame", "gdImageStringFT failed: %s\n", err);
  }
}

#if 0
static inline void
copy_jpeg_decBuf_to_image(void)
{
  uint8_t   r, g, b;
  int       color;

  for(uint32_t y = 0; y < _height; y++)
  {
    for(uint32_t x = 0; x < _width; x++)
    {
      r = _jpeg_decBuf[x * 4 + 0 + y * _width * 4];
      g = _jpeg_decBuf[x * 4 + 1 + y * _width * 4];
      b = _jpeg_decBuf[x * 4 + 2 + y * _width * 4];

      color = gdTrueColor(r,g,b);
      gdImageSetPixel(_image, x, y, color);
    }
  }
}

static inline void
copy_image_to_jpeg_encBuf(void)
{
  uint8_t   r, g, b;
  int       color;

  for(uint32_t y = 0; y < _height; y++)
  {
    for(uint32_t x = 0; x < _width; x++)
    {
      color = gdImageGetPixel(_image, x, y);
      r = (color >> 16) & 0xff;
      g = (color >> 8) & 0xff;
      b = (color >> 0) & 0xff;

      _jpeg_encBuf[x * 4 + 0 + y * _width * 4] = r;
      _jpeg_encBuf[x * 4 + 1 + y * _width * 4] = g;
      _jpeg_encBuf[x * 4 + 2 + y * _width * 4] = b;
    }
  }
}
#endif

static void
frame_converter_convert(uint8_t* jpeg, uint32_t len)
{
  uint8_t*          jpg_img;
  uint32_t          size;
  frame_listener_t* l;

  brcmjpeg_decode(jpeg, len, (uint8_t*)_image_buf, _width * _height * sizeof(uint32_t));

  __handle_overlay();

  jpg_img = _buffers[_buffers_ndx].ptr;

  size = MAX_ENCODED;
  brcmjpeg_encode((uint8_t*)_image_buf, _width, _height, jpg_img, &size);


  _buffers[_buffers_ndx].size = size;
  _last_frame_size = size;

  list_for_each_entry(l, &_listeners, le)
  {
    l->cb(_buffers[_buffers_ndx].ptr, _buffers[_buffers_ndx].size);
  }

  _buffers_ndx = (_buffers_ndx + 1) % MAX_FRAME_CONVERTER_BUFFERS;
}

static void
camera_frame_event(uint8_t* buf, size_t length)
{
  frame_converter_convert(buf, length);
}

void
frame_converter_init(uint32_t width, uint32_t height)
{
  _width = width;
  _height = height;

  _image = gdImageCreateTrueColor(_width, _height);

  _buffers_ndx = 0;

  INIT_LIST_HEAD(&_listeners);

  _cam_listener.cb = camera_frame_event;
  camera_driver_listen(&_cam_listener);

  brcmjpeg_encode_init();

  //
  // this is a hack for LibGD
  // LibGD's pixel buffer is basically 2 dimensional array
  // and quite inefficient for RPI's H/W encoding/decoding
  //
  _tpixels_org  = (uint32_t**)_image->tpixels;
  _image_buf    = (uint32_t*)calloc(_width*_height, sizeof(uint32_t));
  _tpixels_buf  = (uint32_t**)malloc(_height * sizeof(uint32_t*));

  for(uint32_t i = 0; i < _height; i++)
  {
    _tpixels_buf[i] = &_image_buf[i * _width];
  }
  _image->tpixels = (int**)_tpixels_buf;

  // FIXME
  // _tpixels_org gotta be restored before freeing _image
  //
}

void
frame_converter_listen(frame_listener_t* listener)
{
  INIT_LIST_HEAD(&listener->le);
  list_add_tail(&listener->le, &_listeners);
}
