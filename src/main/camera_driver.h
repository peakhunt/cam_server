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

typedef enum
{
  CAMERA_CONTROL_BRIGHTNESS,
  CAMERA_CONTROL_SATURATION,
  CAMERA_CONTROL_HUE,
  CAMERA_CONTROL_EXPOSURE,
  CAMERA_CONTROL_HFLIP,
} camera_control_t;

extern int camera_driver_get_control(camera_control_t c);
extern int camera_driver_set_control(camera_control_t c, int v);

#endif /* !__CAMERA_DRIVER_H__ */
