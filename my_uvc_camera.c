/*使用V4L2框架打开一个UVC摄像头
*/
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <sys/ioctl.h>

#include "uvc_camera.h"
#include "server.h"

#define camera_path "/dev/video0"

int main(int argc, char *argv[])
{
	Cam cam;
	cam.cam_path = camera_path;
	cam.format = V4L2_PIX_FMT_YUYV;//YUYV格式的摄像头
	cam.width = 640;
	cam.height = 480;
	cam.nbufs = V4L_BUFFERS_DEFAULT;

	int ret;
	FILE *file;


	unsigned int i;//用于计数

	//打开摄像头并开始捕捉视频
	ret = open_camera(&cam);
	if (ret < 0) {
		printf("Error opening camera %s.\n",
			cam.cam_path);
		close(cam.dev);
		return ret;
	}

	//查看摄像头支持的图像格式
	//video_list_formats(cam.dev);

	ret = grab_frame(&cam);
	if (ret < 0) {
		printf("Error grab frame.\n");
		close(cam.dev);
		return ret;
	}	

	file = fopen("./1.jpg", "wb");
	if (file != NULL) {
		fwrite(cam.jpgframe, cam.framebuffer_size, 1, file);
		fclose(file);
	}else{
		printf("Error open a file.\n");
		return file;
	}	

	//打开socket,等待客户端连接
	//server_up();
	

	close_camera(&cam);

}
