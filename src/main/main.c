#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include "generic_list.h"
#include "cli.h"
#include "io_driver.h"
#include "webserver.h"

#define CONFIG_TELNET_PORT        11050

static void cli_command_cam_feed(cli_intf_t* intf, int argc, const char** argv);

static io_driver_t      _io_driver;
static cli_command_t    _app_commands[] =
{
  {
    "cam_feed_test",
    "cam feed test command",
    cli_command_cam_feed,
  },
};

io_driver_t*
cli_io_driver(void)
{
  return &_io_driver;
}

static void
cli_command_cam_feed(cli_intf_t* intf, int argc, const char** argv)
{
  webserver_feed_cam_data();
}

int
main(int argc, char** argv)
{
  webserver_init();
  io_driver_init(&_io_driver);
  
  cli_init(_app_commands, NARRAY(_app_commands), CONFIG_TELNET_PORT);
  
  while(1)
  {
    io_driver_run(&_io_driver);
  }

  return 0;
}
