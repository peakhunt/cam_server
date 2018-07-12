#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <gd.h>

#include "common_def.h"
#include "camera_driver.h"
#include "frame_converter.h"

typedef struct
{
  uint8_t*        ptr;
  uint32_t        size;
} frame_converter_buffer_t;

#define MAX_FRAME_CONVERTER_BUFFERS   16

static gdImagePtr   _image;
static uint32_t     _width,
                    _height;

static frame_converter_buffer_t   _buffers[MAX_FRAME_CONVERTER_BUFFERS];
static uint32_t _buffers_ndx = 0;

static camera_driver_listener_t   _cam_listener;

static struct list_head       _listeners;

static void
__set_pixel(int x, int y,  uint8_t r, uint8_t g, uint8_t b)
{
  int         color;

  color = gdImageColorAllocate(_image, r, g, b);
  gdImageSetPixel(_image, x, y, color);
}

static void
__yuyv_to_gd_image(uint8_t* yuyv)
{
	#define MIN(a, b)		(a < b ? a : b)
	#define MAX(a, b)		(a > b ? a : b)
	#define clamp(v)		MIN(255, MAX(0,(v)))

	int nStrideSrc = _width * 2;
	uint8_t *pSrc = yuyv;
	uint8_t		r, g, b;
	int nRow, nCol, nColDst, c, d, e;
	int x;

	for ( nRow = 0; nRow < _height; ++nRow )
	{
		x = 0;
		for ( nCol = 0, nColDst = 0; nCol < nStrideSrc; nCol +=4, nColDst += 6 )
		{
			d = (int)pSrc[nCol + 1] - 128;
			e = (int)pSrc[nCol + 3] - 128;

			c = 298 * ( ( int )pSrc[ nCol ] - 16 );

			// Blue
			b = ( uint8_t )clamp( ( c + 516 * d + 128 ) >> 8 );
			// Green
			g = ( uint8_t )clamp( ( c - 100 * d - 208 * e + 128 ) >> 8 );
			// Red
			r = ( uint8_t )clamp( ( c + 409 * e + 128 ) >> 8 );

			c = 298 * ( ( int )pSrc[ nCol + 2 ] - 16 );
			__set_pixel(x, nRow, r, g, b);
			x++;

			// Blue
			b = ( uint8_t )clamp( ( c + 516 * d + 128 ) >> 8 );
			// Green
			g = ( uint8_t )clamp( ( c - 100 * d - 208 * e + 128 ) >> 8 );
			// Red
			r = ( uint8_t )clamp( ( c + 409 * e + 128 ) >> 8 );

			__set_pixel(x, nRow, r, g, b);
			x++;
		}
		pSrc += nStrideSrc;
	}
}

static uint8_t*
__create_jpeg_image(int32_t* size)
{
  uint8_t*  ptr;
  int       s;

  ptr = gdImageJpegPtr(_image, &s, 50);

  *size = (int32_t)s;

  return ptr;
}

static void
__handle_overlay(void)
{
  char msg[128];
  int color = gdImageColorAllocate(_image, 255, 255, 255);
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

static void
frame_converter_convert_yuv422(uint8_t* yuv)
{
  uint8_t*          jpg_img;
  int32_t           size;
  frame_listener_t* l;

	__yuyv_to_gd_image(yuv);

  __handle_overlay();

  jpg_img = __create_jpeg_image(&size);

  if(jpg_img == NULL)
  {
    LOGI("frame", "failed to convert to jpeg\n");
  }
  else
  {
    //LOGI("frame", "converted to jpeg %d bytes\n", size);
  }

  memcpy(_buffers[_buffers_ndx].ptr, jpg_img, size);
  _buffers[_buffers_ndx].size = size;

  gdFree(jpg_img);

  list_for_each_entry(l, &_listeners, le)
  {
    l->cb(_buffers[_buffers_ndx].ptr, _buffers[_buffers_ndx].size);
  }

  _buffers_ndx = (_buffers_ndx + 1) % MAX_FRAME_CONVERTER_BUFFERS;
}

static void
camera_frame_event(uint8_t* buf, size_t length)
{
  frame_converter_convert_yuv422(buf);
}

void
frame_converter_init(uint32_t width, uint32_t height)
{
  _width = width;
  _height = height;

  //_image = gdImageCreate(_width, _height);
  _image = gdImageCreateTrueColor(_width, _height);

  for(uint32_t i = 0; i < MAX_FRAME_CONVERTER_BUFFERS; i++)
  {
    // believe this should be enough for a JPEG image
    _buffers[i].ptr   = malloc(width * height * sizeof(uint32_t));
    _buffers[i].size  = 0;
  }
  _buffers_ndx = 0;

  INIT_LIST_HEAD(&_listeners);

  _cam_listener.cb = camera_frame_event;
  camera_driver_listen(&_cam_listener);
}

void
frame_converter_listen(frame_listener_t* listener)
{
  INIT_LIST_HEAD(&listener->le);
  list_add_tail(&listener->le, &_listeners);
}
