#include <math.h>

#include <cv.h>
#include <highgui.h>
#include <cxcore.h>

#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>

#include "camera.h"

#define INBUF_SIZE 4096
#define DEBUG 0
#define DISPLAY 1
#define DEBUG_CAMBUF 0

/*
 * Video decoding example
 */

static void pgm_save(unsigned char *buf, int wrap, int xsize, int ysize,
                     char *filename)
{
    FILE *f;
    int i;

    f = fopen(filename,"w");
    fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);
    for(i = 0;i < ysize;i++)
        fwrite(buf + i * wrap, 1, xsize, f);
    fclose(f);
}

static int decode_write_frame(const char *outfilename, AVCodecContext *avctx,
                              AVFrame *frame, int *frame_count, AVPacket *pkt, int last)
{
    int len, got_frame;
    char buf[1024];

    len = avcodec_decode_video2(avctx, frame, &got_frame, pkt);
    if (len < 0) {
        fprintf(stderr, "Error while decoding frame %d\n", *frame_count);
        return len;
    }
    if (got_frame) {
        printf("Saving %sframe %3d\n", last ? "last " : "", *frame_count);
        fflush(stdout);

        /* the picture is allocated by the decoder, no need to free it */
        snprintf(buf, sizeof(buf), outfilename, *frame_count);
        pgm_save(frame->data[0], frame->linesize[0],
                 avctx->width, avctx->height, buf);
        (*frame_count)++;
    }
    if (pkt->data) {
        pkt->size -= len;
        pkt->data += len;
    }
    return 0;
}

//通过查找0x000001或者0x00000001找到下一个数据包的头部  
static int _find_head(unsigned char *buffer, int len)  
{  
    int i;  
   
    for(i = 512;i < len;i++)  
    {  
        if(buffer[i] == 0 && buffer[i+1] == 0 && buffer[i+2] == 0 && buffer[i+3] == 1)  
            break;  
        if(buffer[i] == 0 && buffer[i+1] == 0 && buffer[i+2] == 1)  
            break;  
    }  
    if (i == len)  
        return 0;
  
    if (i == 512)  
        return 0;

    return i;  
}  

//将文件中的一个数据包转换成AVPacket类型以便ffmpeg进行解码  
#define FILE_READING_BUFFER (1*1024*1024)  
static void build_avpkt(AVPacket *avpkt, FILE *fp)  
{  
    static unsigned char buffer[1*1024*1024];  
    static int readptr = 0;  
    static int writeptr = 0;  
    int len,toread;  
   
    int nexthead;  
   
    if(writeptr- readptr < 200 * 1024)  
    {  
        memmove(buffer, &buffer[readptr], writeptr - readptr);  
        writeptr -= readptr;  
        readptr = 0;  
        toread = FILE_READING_BUFFER - writeptr;  
        len = fread(&buffer[writeptr], 1, toread, fp);  
        writeptr += len;  
    }  
  
#if DEBUG_CAMBUF
	int tmp_i = 0;
	for(tmp_i = 0; tmp_i < buffer_dec_len; tmp_i++)
        printf("0x%02x ", buffer_dec[tmp_i]);
	printf("\n");
	printf("len = %d buffer_dec_len = %d\n", len, buffer_dec_len);
#endif

    nexthead = _find_head(&buffer[readptr], writeptr-readptr);  
    if (nexthead == 0)  
    {  
        printf("failedfind next head...\n");  
        nexthead = writeptr - readptr;  
    }  
   
    avpkt->size = nexthead;  
    avpkt->data = &buffer[readptr];  
    readptr += nexthead; 
}

static void video_decode_example(const char *outfilename, const char *filename)
{

    AVCodec *codec;  
    AVCodecContext *c= NULL;  
    int frame, got_picture, len;  
    FILE *f, *fout;  
    AVFrame *picture;  
    uint8_t inbuf[INBUF_SIZE + FF_INPUT_BUFFER_PADDING_SIZE];  
    char buf[1024];  
    AVPacket avpkt;  
   
    av_init_packet(&avpkt);  
   
    /* set end ofbuffer to 0 (this ensures that no overreading happens for damaged mpeg streams)*/  
    memset(inbuf + INBUF_SIZE, 0, FF_INPUT_BUFFER_PADDING_SIZE);  
   
    printf("Videodecoding\n");  
    //opts = NULL;  
    /* find the h264video decoder */  
    codec = avcodec_find_decoder(CODEC_ID_H264);  
    if (!codec){  
        fprintf(stderr, "codecnot found\n");  
        return ;  
    }  
   
    c = avcodec_alloc_context3(codec);  
    picture= avcodec_alloc_frame();  
   
    if(codec->capabilities&CODEC_CAP_TRUNCATED)  
    c->flags|= CODEC_FLAG_TRUNCATED; /* we do not send complete frames */  
   
    /* For somecodecs, such as msmpeg4 and mpeg4, width and height 
    MUST be initialized there because thisinformation is not 
    available in the bitstream. */  
   
    /* open it */  
    if(avcodec_open2(c, codec, NULL) < 0) {  
        fprintf(stderr, "couldnot open codec\n");  
        exit(1);  
    }  
   
    /* the codec givesus the frame size, in samples */  
   
    f = fopen(filename, "rb");  
    if (!f) {  
        fprintf(stderr, "couldnot open %s\n", filename);  
        exit(1);  
    }  

     //解码与显示需要的辅助的数据结构，需要注意的是，AVFrame必须经过alloc才能使用，不然其内存的缓存空间指针是空的，程序会崩溃  
    AVFrame frameRGB;  
    cvNamedWindow("decode", CV_WINDOW_AUTOSIZE);  
    frame = 0;  
    for(;;) {  
   
        build_avpkt(&avpkt, f);  
   
        if(avpkt.size == 0)  
            break;  
   
        while(avpkt.size > 0) {  
            len = avcodec_decode_video2(c,picture, &got_picture, &avpkt);//解码每一帧  

			IplImage *showImage = cvCreateImage(cvSize(picture->width, picture->height), 8, 3);  
            avpicture_alloc((AVPicture*)&frameRGB, PIX_FMT_RGB24, picture->width, picture->height);  

            if(len < 0) {  
                fprintf(stderr, "Error while decoding frame %d\n",frame);  
                break;  
            }  
            if(got_picture) {  
                printf("savingframe %3d\n", frame);  
                fflush(stdout);  
   
                /* thepicture is allocated by the decoder. no need to free it */  
               //将YUV420格式的图像转换成RGB格式所需要的转换上下文  
                struct SwsContext * scxt = (struct SwsContext *)sws_getContext(picture->width, picture->height, PIX_FMT_YUV420P,  
                                                  picture->width, picture->height, PIX_FMT_RGB24,  
                                                  2,NULL,NULL,NULL);  
                if(scxt != NULL)  
                {  
                    sws_scale(scxt, picture->data, picture->linesize, 0, c->height, frameRGB.data, frameRGB.linesize);//图像格式转换  
                    showImage->imageSize = frameRGB.linesize[0];//指针赋值给要显示的图像  
                    showImage->imageData = (char *)frameRGB.data[0];  
                    cvShowImage("decode", showImage);//显示 
#if DISPLAY	
                    cvWaitKey(50);//设置0.5s显示一帧，如果不设置由于这是个循环，会导致看不到显示出来的图像  
#endif
                }
                //sprintf(buf,outfilename,frame);  
   
                //pgm_save(picture->data[0],picture->linesize[0],  
                //c->width,c->height, buf);  
                //pgm_save(picture->data[1],picture->linesize[1],  
                //c->width/2,c->height/2, fout);  
                //pgm_save(picture->data[2],picture->linesize[2],  
                //c->width/2,c->height/2, fout);   
				sws_freeContext(scxt);
                frame++;  
            }  
            avpkt.size -= len;  
            avpkt.data += len;  
        }  
    }  
   
    fclose(f);  
   
    avcodec_close(c);  
    av_free(c);  
    av_free(picture);  
    printf("\n");  
}
#define V4L_BUFFERS_DEFAULT	8
#define V4L_BUFFERS_MAX		32

static int camera_capture()
{
	int dev = 0;
	unsigned int width = 160;
	unsigned int height = 120;
	unsigned int ret = 0;
	unsigned int pixelformat = V4L2_PIX_FMT_H264;
	unsigned int nbufs = V4L_BUFFERS_DEFAULT;
	struct v4l2_buffer buf;
	int i = 0;
	struct timeval start, end, ts;
	unsigned int delay = 0;	
	FILE *rec_fp = NULL;
	char rec_filename[] = "RecordH264.h264";/*"H264.ts"*/
	void *mem[V4L_BUFFERS_MAX];
	double fps;

	dev = video_open("/dev/video1");
	if (dev < 0)
		return 1;

	v4l2ResetControl (dev, V4L2_CID_BRIGHTNESS);
	v4l2ResetControl (dev, V4L2_CID_CONTRAST);
	v4l2ResetControl (dev, V4L2_CID_SATURATION);
	v4l2ResetControl (dev, V4L2_CID_GAIN);

	ret = video_get_input(dev);
	printf("Input %d selected\n", ret);

	/* Set the video format. */
	if (video_set_format(dev, width, height, pixelformat) < 0) {
		close(dev);
		return 1;
	}

	/* Set the frame rate. */
	if (video_set_framerate(dev) < 0) {
		close(dev);
		return 1;
	}

	/* Allocate buffers. */
	if ((int)(nbufs = video_reqbufs(dev, nbufs)) < 0) {
		close(dev);
		return 1;
	}

	/* Map the buffers. */
	for (i = 0; i < nbufs; ++i) {
		memset(&buf, 0, sizeof buf);
		buf.index = i;
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		ret = ioctl(dev, VIDIOC_QUERYBUF, &buf);
		if (ret < 0) {
			printf("Unable to query buffer %u (%d).\n", i, errno);
			close(dev);
			return 1;
		}
		printf("length: %u offset: %u\n", buf.length, buf.m.offset);

		mem[i] = mmap(0, buf.length, PROT_READ, MAP_SHARED, dev, buf.m.offset);
		if (mem[i] == MAP_FAILED) {
			printf("Unable to map buffer %u (%d)\n", i, errno);
			close(dev);
			return 1;
		}
		printf("Buffer %u mapped at address %p.\n", i, mem[i]);
	}

	/* Queue the buffers. */
	for (i = 0; i < nbufs; ++i) {
		memset(&buf, 0, sizeof(buf));
		buf.index = i;
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		ret = ioctl(dev, VIDIOC_QBUF, &buf);
		if (ret < 0) {
			printf("Unable to queue buffer (%d).\n", errno);
			close(dev);
			return 1;
		}
	}

	/* Start streaming. */
	video_enable(dev, 1);

#if DEBUG
		printf("enable stream\n");
#endif

	for (i = 0; i <= 300; ++i) {
		/* Dequeue a buffer. */

#if DEBUG
		printf("dequeque buffer start\n");
#endif

		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;

#if DEBUG
		printf("dequeque buffer ioctl\n");
#endif

		ret = ioctl(dev, VIDIOC_DQBUF, &buf);
		if (ret < 0) {
			printf("Unable to dequeue buffer (%d).\n", errno);
			close(dev);
			return 1;
		}

#if DEBUG
		printf("dequeque buffer\n");
#endif

		gettimeofday(&ts, NULL);
		printf("%u %u bytes %ld.%06ld %ld.%06ld\n", i, buf.bytesused,
				buf.timestamp.tv_sec, buf.timestamp.tv_usec,
				ts.tv_sec, ts.tv_usec);

		if (i == 0)
			start = ts;

		/* Record the H264 video file */
		if(rec_fp == NULL)
			rec_fp = fopen(rec_filename, "a+b");

#if DEBUG
		printf("open fd\n");
#endif

		if(rec_fp != NULL)
		{
			fwrite(mem[buf.index], buf.bytesused, 1, rec_fp);
		}

		/* fclose(rec_fp); */

#if DEBUG
		printf("fwrite\n");
#endif
			
		/* Requeue the buffer. */
		if (delay > 0)
			usleep(delay * 1000);

		ret = ioctl(dev, VIDIOC_QBUF, &buf);
		if (ret < 0) {
			printf("Unable to requeue buffer (%d).\n", errno);
			close(dev);
			return 1;
		}

#if DEBUG
		printf("queque buffer\n");
#endif

		fflush(stdout);

#if DEBUG
		printf("fflush\n");
		printf("------------------\n");
#endif

	}
	gettimeofday(&end, NULL);
	printf("300 frames completed!!\n");
	sleep(2);

	/* Stop streaming. */
	video_enable(dev, 0);

	fclose(rec_fp);

	end.tv_sec -= start.tv_sec;
	end.tv_usec -= start.tv_usec;
	if (end.tv_usec < 0) {
		end.tv_sec--;
		end.tv_usec += 1000000;
	}
	fps = (i-1)/(end.tv_usec+1000000.0*end.tv_sec)*1000000.0;

	printf("Captured %u frames in %lu.%06lu seconds (%f fps).\n",
			i-1, end.tv_sec, end.tv_usec, fps);

	close(dev);
}



int main(int argc, char **argv)
{

    /* register all the codecs */
    avcodec_register_all();

    camera_capture();
    video_decode_example("test%02d.pgm", "RecordH264.h264");
    return 1;

    return 0;
}
