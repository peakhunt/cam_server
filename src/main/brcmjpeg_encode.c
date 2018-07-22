#include "brcmjpeg.h"
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include "common_def.h"

#include "brcmjpeg_encode.h"

static const char* TAG = "codec";
static BRCMJPEG_T* _decoder = 0;
static BRCMJPEG_T* _encoder = 0;

void
brcmjpeg_encode_init(void)
{
  BRCMJPEG_STATUS_T status;

  status = brcmjpeg_create(BRCMJPEG_TYPE_ENCODER, &_encoder);
  status = brcmjpeg_create(BRCMJPEG_TYPE_DECODER, &_decoder);

  (void)status;
}

void
brcmjpeg_encode_deinit(void)
{
  brcmjpeg_release(_encoder);
  brcmjpeg_release(_decoder);
}

void
brcmjpeg_decode(uint8_t* img, uint32_t size, uint8_t* out, uint32_t outsize)
{
  BRCMJPEG_REQUEST_T  dec_request;
  BRCMJPEG_STATUS_T   status;

  memset(&dec_request, 0, sizeof(dec_request));

  dec_request.input             = img;
  dec_request.input_size        = size;
  dec_request.output            = out;
  dec_request.output_handle     = 0;
  dec_request.output_alloc_size = outsize;
  dec_request.pixel_format      = PIXEL_FORMAT_RGBA;

  dec_request.buffer_width = 0;
  dec_request.buffer_height = 0;

  status = brcmjpeg_process(_decoder, &dec_request);
  if(status != BRCMJPEG_SUCCESS)
  {
    LOGE(TAG, "decoding failed\n");
  }
}

void
brcmjpeg_encode(uint8_t* in, uint32_t w, uint32_t h, uint8_t* out, uint32_t* out_size)
{
  BRCMJPEG_REQUEST_T enc_request;
  BRCMJPEG_STATUS_T status;

  memset(&enc_request, 0, sizeof(enc_request));

  enc_request.input             = in;
  enc_request.input_handle      = 0;
  enc_request.output            = out;
  enc_request.output_alloc_size = *out_size;
  enc_request.quality           = 13;
  enc_request.pixel_format      = PIXEL_FORMAT_RGBA;

  enc_request.input_size = w * h * 4;
  enc_request.width = w;
  enc_request.height = h;
  enc_request.buffer_width = 0;
  enc_request.buffer_height = 0;

  status = brcmjpeg_process(_encoder, &enc_request);
  if(status != BRCMJPEG_SUCCESS)
  {
    LOGE(TAG, "encoding failed\n");
  }

  *out_size = enc_request.output_size;
}
