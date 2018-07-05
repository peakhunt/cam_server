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

extern int camera_driver_get_brightness(void);
extern void camera_driver_set_brightness(int v);
extern int camera_driver_get_saturation(void);
extern void camera_driver_set_saturation(int v);
extern int camera_driver_get_hue(void);
extern void camera_driver_set_hue(int v);
extern int camera_driver_get_exposure(void);
extern void camera_driver_set_exposure(int v);
extern int camera_driver_get_hflip(void);
extern void camera_driver_set_hflip(int v);

#endif /* !__CAMERA_DRIVER_H__ */
