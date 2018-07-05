#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include "generic_list.h"
#include "cli.h"
#include "io_driver.h"
#include "webserver.h"
#include "camera_driver.h"

#define CONFIG_TELNET_PORT        11050

static void cli_command_cam_feed(cli_intf_t* intf, int argc, const char** argv);
static void cli_command_cam_brightness(cli_intf_t* intf, int argc, const char** argv);
static void cli_command_cam_saturation(cli_intf_t* intf, int argc, const char** argv);
static void cli_command_cam_hue(cli_intf_t* intf, int argc, const char** argv);
static void cli_command_cam_exposure(cli_intf_t* intf, int argc, const char** argv);

static io_driver_t      _io_driver;
static cli_command_t    _app_commands[] =
{
  {
    "cam_feed_test",
    "cam feed test command",
    cli_command_cam_feed,
  },
  {
    "brightness",
    "camera brightness command",
    cli_command_cam_brightness,
  },
  {
    "saturation",
    "camera saturation command",
    cli_command_cam_saturation,
  },
  {
    "hue",
    "camera hue command",
    cli_command_cam_hue,
  },
  {
    "exposure",
    "camera exposure command",
    cli_command_cam_exposure,
  }
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
cli_command_cam_brightness(cli_intf_t* intf, int argc, const char** argv)
{
  int brightness;

  if(argc < 2)
    goto error;

  if(strcmp(argv[1], "get") == 0)
  {
    brightness = camera_driver_get_brightness();
    cli_printf(intf, "brightness %d"CLI_EOL, brightness);
  }
  else if(strcmp(argv[1], "set") == 0)
  {
    if(argc != 3)
      goto error;

    brightness = atoi(argv[2]);
    camera_driver_set_brightness(brightness);
  }
  else
  {
    goto error;
  }
  return;

error:
  cli_printf(intf, "invalid syntax"CLI_EOL);
  cli_printf(intf, "brightness get"CLI_EOL);
  cli_printf(intf, "brightness set <value>"CLI_EOL);
}

static void
cli_command_cam_saturation(cli_intf_t* intf, int argc, const char** argv)
{
  int saturation;

  if(argc < 2)
    goto error;

  if(strcmp(argv[1], "get") == 0)
  {
    saturation = camera_driver_get_saturation();
    cli_printf(intf, "saturation %d"CLI_EOL, saturation);
  }
  else if(strcmp(argv[1], "set") == 0)
  {
    if(argc != 3)
      goto error;

    saturation = atoi(argv[2]);
    camera_driver_set_saturation(saturation);
  }
  else
  {
    goto error;
  }
  return;

error:
  cli_printf(intf, "invalid syntax"CLI_EOL);
  cli_printf(intf, "saturation get"CLI_EOL);
  cli_printf(intf, "saturation set <value>"CLI_EOL);
}

static void
cli_command_cam_hue(cli_intf_t* intf, int argc, const char** argv)
{
  int hue;

  if(argc < 2)
    goto error;

  if(strcmp(argv[1], "get") == 0)
  {
    hue = camera_driver_get_hue();
    cli_printf(intf, "hue %d"CLI_EOL, hue);
  }
  else if(strcmp(argv[1], "set") == 0)
  {
    if(argc != 3)
      goto error;

    hue = atoi(argv[2]);
    camera_driver_set_hue(hue);
  }
  else
  {
    goto error;
  }
  return;

error:
  cli_printf(intf, "invalid syntax"CLI_EOL);
  cli_printf(intf, "hue get"CLI_EOL);
  cli_printf(intf, "hue set <value>"CLI_EOL);
}

static void
cli_command_cam_exposure(cli_intf_t* intf, int argc, const char** argv)
{
  int exposure;

  if(argc < 2)
    goto error;

  if(strcmp(argv[1], "get") == 0)
  {
    exposure = camera_driver_get_exposure();
    cli_printf(intf, "exposure %d"CLI_EOL, exposure);
  }
  else if(strcmp(argv[1], "set") == 0)
  {
    if(argc != 3)
      goto error;

    exposure = atoi(argv[2]);
    camera_driver_set_exposure(exposure);
  }
  else
  {
    goto error;
  }
  return;

error:
  cli_printf(intf, "invalid syntax"CLI_EOL);
  cli_printf(intf, "exposure get"CLI_EOL);
  cli_printf(intf, "exposure set <value>"CLI_EOL);
}

int
main(int argc, char** argv)
{
  camera_driver_init();

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
