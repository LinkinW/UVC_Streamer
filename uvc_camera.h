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
#include <stdlib.h>

#include <linux/videodev2.h>

#include "huffman.h"

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
	unsigned char *framebuffer;//指向抓取的一帧图像
	unsigned char *jpgframe;//转换后的jpg格式图像
	int framebuffer_size;

	struct v4l2_capability cap;
	struct v4l2_buffer buf;

	void *mem[V4L_BUFFERS_MAX];
};

int open_camera(Cam *cam);
int grab_frame(Cam *cam);
int close_camera(Cam *cam);
static int video_enable(int dev, int enable);
static int video_set_format(int dev, unsigned int w, unsigned int h, unsigned int format);
static int video_set_framerate(int dev);
static int video_reqbufs(int dev, int nbufs);
static int enum_frame_sizes(int dev, __u32 pixfmt);
void video_list_formats(int dev);
int enum_frame_intervals(int dev, __u32 pixfmt, __u32 width, __u32 height);
static void video_enum_inputs(int dev);
static int video_get_input(int dev);
static int video_set_input(int dev, unsigned int input);

#endif