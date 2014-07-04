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

int video_open(const char *devname);
int video_get_input(int dev);
int video_set_format(int dev, unsigned int w, unsigned int h, unsigned int format);
int video_set_framerate(int dev);
int video_reqbufs(int dev, int nbufs);
int video_enable(int dev, int enable);

#endif
