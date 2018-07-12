#ifndef __FRAME_CONVERTER_DEF_H__
#define __FRAME_CONVERTER_DEF_H__

#include "common_def.h"
#include "generic_list.h"

typedef void (*frame_callback)(uint8_t* buf, size_t length);

typedef struct
{
  struct list_head          le;
  frame_callback            cb;
} frame_listener_t;

extern void frame_converter_init(uint32_t width, uint32_t height);
extern void frame_converter_listen(frame_listener_t* listener);

#endif /* !__FRAME_CONVERTER_DEF_H__ */
