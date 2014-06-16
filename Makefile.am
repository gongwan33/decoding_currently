AUTOMAKE_OPTIONS=gnu
bin_PROGRAMS=decoding
INCLUDES=-I/usr/local/include/opencv -I/usr/local/include
decoding_SOURCES=decoding_encoding.c \
				 v4l2uvc.c \
				 camera.c \
				 camera.h \
				 v4l2uvc.h \
				 sonix_xu_ctrls.h
decoding_LDADD= /usr/local/lib/libopencv_calib3d.so /usr/local/lib/libopencv_contrib.so /usr/local/lib/libopencv_core.so /usr/local/lib/libopencv_features2d.so /usr/local/lib/libopencv_flann.so /usr/local/lib/libopencv_gpu.so /usr/local/lib/libopencv_highgui.so /usr/local/lib/libopencv_imgproc.so /usr/local/lib/libopencv_legacy.so /usr/local/lib/libopencv_ml.so /usr/local/lib/libopencv_nonfree.so /usr/local/lib/libopencv_objdetect.so /usr/local/lib/libopencv_ocl.so /usr/local/lib/libopencv_photo.so /usr/local/lib/libopencv_stitching.so /usr/local/lib/libopencv_superres.so /usr/local/lib/libopencv_ts.a /usr/local/lib/libopencv_video.so /usr/local/lib/libopencv_videostab.so
