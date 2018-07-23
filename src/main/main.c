#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include "generic_list.h"
#include "cli.h"
#include "io_driver.h"
#include "webserver.h"
#include "camera_driver.h"
#include "frame_converter.h"

#define CONFIG_TELNET_PORT        11050

static void cli_command_cam_feed(cli_intf_t* intf, int argc, const char** argv);
static void cli_command_cam_control(cli_intf_t* intf, int argc, const char** argv);
static void cli_command_fps(cli_intf_t* intf, int argc, const char** argv);
static void cli_command_last_frame_size(cli_intf_t* intf, int argc, const char** argv);

#ifdef RPI
static void cli_command_quality(cli_intf_t* intf, int argc, const char** argv);
#endif

static io_driver_t      _io_driver;
static cli_command_t    _app_commands[] =
{
  {
    "cam_feed_test",
    "cam feed test command",
    cli_command_cam_feed,
  },
  {
    "control",
    "camera control command",
    cli_command_cam_control,
  },
  {
    "fps",
    "show fps",
    cli_command_fps,
  },
  {
    "last_frame_size",
    "show last frame size",
    cli_command_last_frame_size,
  },
#ifdef RPI
  {
    "quality",
    "set quality",
    cli_command_quality,
  }
#endif
};

io_driver_t*
main_io_driver(void)
{
  return &_io_driver;
}

io_driver_t*
cli_io_driver(void)
{
  return main_io_driver();
}

static void
cli_command_cam_feed(cli_intf_t* intf, int argc, const char** argv)
{
  webserver_feed_cam_data();
}

static void
cli_command_cam_control(cli_intf_t* intf, int argc, const char** argv)
{
  static struct
  {
    const char*         name;
    camera_control_t    c;
  } parameters[] = 
  {
    { "brightness", CAMERA_CONTROL_BRIGHTNESS, },
    { "saturation", CAMERA_CONTROL_SATURATION, },
    { "hue", CAMERA_CONTROL_HUE, },
    { "exposure", CAMERA_CONTROL_EXPOSURE, },
    { "hflip", CAMERA_CONTROL_HFLIP, },
    { "jpg_quality", CAMERA_CONTROL_JPEG_QUALITY, },
  };
  int   v;
  int   get = 0;
  camera_control_t    c = -1;

  if(argc < 3) goto error;

  if(strcmp(argv[1], "get") == 0)
  {
    get = 1;
  }
  else if(strcmp(argv[1], "set") == 0)
  {
    if(argc != 4) goto error;
  }
  else
  {
    goto error;
  }

  for(int i = 0; i < NARRAY(parameters); i++)
  {
    if(strcmp(parameters[i].name, argv[2]) == 0)
    {
      c = parameters[i].c;
      break;
    }
  }

  if(c == -1) goto error;

  if(get)
  {
    v = camera_driver_get_control(c);
    cli_printf(intf, "%s %d"CLI_EOL, argv[2], v);
  }
  else
  {
    v = atoi(argv[3]);
    camera_driver_set_control(c, v);
  }

  return;

error:
  cli_printf(intf, "invalid syntax"CLI_EOL);
  for(int i = 0; i < NARRAY(parameters); i++)
  {
    cli_printf(intf, "control [get|set] %s <value>"CLI_EOL, parameters[i].name);
  }
}

static void
cli_command_fps(cli_intf_t* intf, int argc, const char** argv)
{
  cli_printf(intf, "fps: %d"CLI_EOL, camera_driver_get_fps());
}

static void
cli_command_last_frame_size(cli_intf_t* intf, int argc, const char** argv)
{
  extern uint32_t _last_frame_size;

  cli_printf(intf, "last frame size: %d"CLI_EOL, _last_frame_size);
}

#ifdef RPI
static void
cli_command_quality(cli_intf_t* intf, int argc, const char** argv)
{
  extern uint8_t _jpeg_quality;

  if(argc < 2) goto error;

  if(strcmp(argv[1], "get") == 0)
  {
    cli_printf(intf, "jpeg quality: %d"CLI_EOL, _jpeg_quality);
  }
  else if(strcmp(argv[1], "set") == 0)
  {
    if(argc != 3) goto error;

    _jpeg_quality = atoi(argv[2]);

    cli_printf(intf, "set quality to %d"CLI_EOL, _jpeg_quality);
  }
  else
  {
    goto error;
  }

  return;

error:
  cli_printf(intf, "invalid syntax"CLI_EOL);
  cli_printf(intf, "quality get"CLI_EOL);
  cli_printf(intf, "quality set <value>"CLI_EOL);
}
#endif

int
main(int argc, char** argv)
{
  camera_driver_init();

#ifdef USE_FRAME_CONVERTER
  frame_converter_init(640, 480);
#endif
  webserver_init();
  io_driver_init(&_io_driver);
  
  cli_init(_app_commands, NARRAY(_app_commands), CONFIG_TELNET_PORT);

  camera_driver_start();
  
  while(1)
  {
    io_driver_run(&_io_driver);
  }

  return 0;
}
