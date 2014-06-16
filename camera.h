#ifndef CAMERA_H
#define CAMERA_H

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/time.h>
#include <linux/videodev2.h>

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

#include "v4l2uvc.h"
#include "sonix_xu_ctrls.h"

static int video_open(const char *devname);

#endif
