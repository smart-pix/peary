# Syncing with Kernel

This interface has dependency on Linux kernel headers (linux/v4l2-subdev.h).

User needs to make sure, that [linux/v4l2-subdev.h](linux/v4l2-subdev.h) is up-to-date with the kernel on which this interface will be used.
To do that, at the Kernel source tree level, call:
```
 $ make headers_install INSTALL_HDR_PATH=usr/
```
and use the created `usr/include/linux/v4l2-subdev.h` file to replace [linux/v4l2-subdev.h](linux/v4l2-subdev.h) header in this directory.
