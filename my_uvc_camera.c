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

	unsigned int i;//用于计数

	//打开摄像头并开始捕捉视频
	ret = opnecamera(&cam);
	if (ret < 0) {
		printf("Error opening camera %s.\n",
			cam.cam_path);
		close(cam.dev);
		return ret;
	}

	//打开socket,等待客户端连接
	//server_up();
	

	close(cam.dev);

}
