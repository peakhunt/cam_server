#ifndef __CAMERA_DRIVER_H__
#define __CAMERA_DRIVER_H__

#include "common_def.h"
#include "generic_list.h"

typedef void (*camera_driver_callback)(uint8_t* buf, size_t length);

typedef struct
{
  struct list_head          le;
  camera_driver_callback    cb;
} camera_driver_listener_t;

extern void camera_driver_init(void);
extern void camera_driver_start(void);
extern void camera_driver_stop(void);
extern void camera_driver_listen(camera_driver_listener_t* listener);

#endif /* !__CAMERA_DRIVER_H__ */
