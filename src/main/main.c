#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include "generic_list.h"
#include "cli.h"
#include "io_driver.h"

#define CONFIG_TELNET_PORT        11050

static io_driver_t      _io_driver;
static cli_command_t    _app_commands[] =
{
};

io_driver_t*
cli_io_driver(void)
{
  return &_io_driver;
}

int
main(int argc, char** argv)
{
  io_driver_init(&_io_driver);
  
  cli_init(_app_commands, NARRAY(_app_commands), CONFIG_TELNET_PORT);
  
  while(1)
  {
    io_driver_run(&_io_driver);
  }

  return 0;
}
