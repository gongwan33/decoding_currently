#include "camera.h"
#include "stdio.h"

static int video_open(const char *devname)
{
	struct v4l2_capability cap;
	int dev, ret;

	dev = open(devname, O_RDWR);
	if (dev < 0) {
		printf("Error opening device %s: %d.\n", devname, errno);
		return dev;
	}

	memset(&cap, 0, sizeof cap);
	ret = ioctl(dev, VIDIOC_QUERYCAP, &cap);
	if (ret < 0) {
		printf("Error opening device %s: unable to query device.\n",
				devname);
		close(dev);
		return ret;
	}

#if 0
	if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0) {
		printf("Error opening device %s: video capture not supported.\n",
				devname);
		close(dev);
		return -EINVAL;
	}
#endif

	printf("Device %s opened: %s.\n", devname, cap.card);
	return dev;
}


