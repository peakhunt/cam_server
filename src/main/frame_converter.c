#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#if 0
static int
convert_yuv_to_rgb_pixel(int y, int u, int v)
{
	unsigned int pixel32 = 0;
	unsigned char *pixel = (unsigned char *)&pixel32;

	int r, g, b;

	r = y + (1.370705 * (v-128));
	g = y - (0.698001 * (v-128)) - (0.337633 * (u-128));
	b = y + (1.732446 * (u-128));

	if(r > 255) r = 255;
	if(g > 255) g = 255;
	if(b > 255) b = 255;
	if(r < 0) r = 0;
	if(g < 0) g = 0;
	if(b < 0) b = 0;

	pixel[0] = r ;
	pixel[1] = g ;
	pixel[2] = b ;

	return pixel32;
}

static void
__set_pixel(uint32_t out, uint8_t r, uint8_t g, uint8_t b)
{
  uint32_t    x, y;
  int         color;
  int         ndx = (int)(out);

  x = ndx % _width;
  y = ndx / _width;

  color = gdImageColorAllocate(_image, r, g, b);
  gdImageSetPixel(_image, x, y, color);
}

static void
__yuv422_to_gd_image(uint8_t* yuv)
{
	unsigned int in, out = 0;
	unsigned int pixel_16;
	unsigned char pixel_24[3];
	unsigned int pixel32;
	int y0, u, y1, v;

	for(in = 0; in < _width * _height * 2; in += 4)
	{
		pixel_16 =
			yuv[in + 3] << 24 |
			yuv[in + 2] << 16 |
			yuv[in + 1] <<  8 |
			yuv[in + 0];

		y0 = (pixel_16 & 0x000000ff);
		u  = (pixel_16 & 0x0000ff00) >>  8;
		y1 = (pixel_16 & 0x00ff0000) >> 16;
		v  = (pixel_16 & 0xff000000) >> 24;

		pixel32 = convert_yuv_to_rgb_pixel(y0, u, v);
		pixel_24[0] = (pixel32 & 0x000000ff);
		pixel_24[1] = (pixel32 & 0x0000ff00) >> 8;
		pixel_24[2] = (pixel32 & 0x00ff0000) >> 16;

    __set_pixel(out, pixel_24[0], pixel_24[1], pixel_24[2]);
    out++;

		pixel32 = convert_yuv_to_rgb_pixel(y1, u, v);
		pixel_24[0] = (pixel32 & 0x000000ff);
		pixel_24[1] = (pixel32 & 0x0000ff00) >> 8;
		pixel_24[2] = (pixel32 & 0x00ff0000) >> 16;

    __set_pixel(out, pixel_24[0], pixel_24[1], pixel_24[2]);
    out++;
	}
}
#endif

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

  ptr = gdImageJpegPtr(_image, &s, -1);

  *size = (int32_t)s;

  return ptr;
}

static void
frame_converter_convert_yuv422(uint8_t* yuv)
{
  uint8_t*          jpg_img;
  int32_t           size;
  frame_listener_t* l;

  //__yuv422_to_gd_image(yuv);
	__yuyv_to_gd_image(yuv);

  // FIXME overlay

  jpg_img = __create_jpeg_image(&size);

  if(jpg_img == NULL)
  {
    LOGI("frame", "failed to convert to jpeg\n");
  }
  else
  {
    LOGI("frame", "converted to jpeg %d bytes\n", size);
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
