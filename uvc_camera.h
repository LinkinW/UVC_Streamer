#ifndef _UVC_CAMERA_H
#define _UVC_CAMERA_H

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>//存放mmap()有关的关键字
#include <sys/select.h>//定义时间结构体
#include <sys/time.h>
#include <jpeglib.h>
#include <linux/fb.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>

#define V4L_BUFFERS_DEFAULT	8//默认的缓冲帧数
#define V4L_BUFFERS_MAX		32//最大的缓冲帧数

typedef struct _Cam Cam;
struct _Cam {
	int dev;//设备文件句柄
	char *cam_path;//摄像头设备所在路径
	__u32 input;

	unsigned int format;//摄像头的图像格式
	unsigned int width;
	unsigned int height;
	unsigned int nbufs;

	struct v4l2_capability cap;
	struct v4l2_buffer buf;

	void *mem[V4L_BUFFERS_MAX];
};

int opnecamera(Cam *cam);
static int video_set_format(int dev, unsigned int w, unsigned int h, unsigned int format);
static int video_set_framerate(int dev);
static int video_reqbufs(int dev, int nbufs);

#endif