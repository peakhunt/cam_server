# cam_server
This is a simple camera server on linux using V4L2.
Currently it supports live camera streaming via HTTP/MJPEG.
The main purpose of this project is to demonstrate how to use camera on linux and show that it shouldn't be any complicated.

## prerequisites
this S/W depends on libgd.
to build property

 sudo apt install libgd-dev

## build
 make -j4

 on raspberyr PI, to use GPIO LED indication for connection status
 make RPI=yes -j4
