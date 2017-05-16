#include "uvc_camera.h"

int open_camera(Cam *cam) 
{
	/*   打开一个V4L2设备的步骤是:
     *   1.打开设备
     *
     *
     */
	int ret;
	unsigned int i = 0;//用于计数

	cam->dev = open(cam->cam_path, O_RDWR);
	if (cam->dev < 0) {
		printf("Error opening device %s: %d.\n", cam->cam_path, errno);
		return cam->dev;
	}

	memset(&cam->cap, 0, sizeof cam->cap);//准备cap结构体获取摄像头信息
	ret = ioctl(cam->dev, VIDIOC_QUERYCAP, &cam->cap);
	if (ret < 0) {
		printf("Error opening device %s: unable to query cam->device.\n",
			cam->cam_path);
		close(cam->dev);
		return ret;
	}

	if ((cam->cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0) {
		printf("Error opening device %s: video capture not supported.\n",
			cam->cam_path);
		close(cam->dev);
		return -EINVAL;
	}
	printf("Device %s opened: %s.\n", cam->cam_path, cam->cap.card);//如果打开摄像头成功,可以成功输出信息

	ret = ioctl(cam->dev, VIDIOC_G_INPUT, &cam->input);//获取输入
	if (ret < 0) {
		printf("Unable to get current input: %s.\n", strerror(errno));
		return ret;
	}
	printf("Input %d selected\n", cam->input);

	ret = video_set_format(cam->dev, cam->width, cam->height, cam->format);
	if (ret < 0) {
		printf("Error setting format %s.\n",cam->cam_path);
		close(cam->dev);
		return ret;
	}

	ret = video_set_framerate(cam->dev);
	if (ret < 0) {
		printf("Error setting framerate %s.\n",cam->cam_path);
		close(cam->dev);
		return ret;
	}

	/* Allocate buffers. */
	ret = (int)(cam->nbufs = video_reqbufs(cam->dev, cam->nbufs));
	if (ret < 0) {
		close(cam->dev);
		return ret;
	}

	/* Map the buffers. */
	for (i = 0; i < cam->nbufs; ++i) {
		memset(&cam->buf, 0, sizeof cam->buf);
		cam->buf.index = i;
		cam->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		cam->buf.memory = V4L2_MEMORY_MMAP;
		ret = ioctl(cam->dev, VIDIOC_QUERYBUF, &cam->buf);
		if (ret < 0) {
			printf("Unable to query buffer %u (%d).\n", i, errno);
			close(cam->dev);
			return 1;
		}
		printf("length: %u offset: %u\n", cam->buf.length, cam->buf.m.offset);

		cam->mem[i] = mmap(0, cam->buf.length, PROT_READ, MAP_SHARED, cam->dev, cam->buf.m.offset);
		if (cam->mem[i] == MAP_FAILED) {
			printf("Unable to map buffer %u (%d)\n", i, errno);
			close(cam->dev);
			return 1;
		}
		printf("Buffer %u mapped at address %p.\n", i, cam->mem[i]);
	}

	/* Queue the buffers. */
	for (i = 0; i < cam->nbufs; ++i) {
		memset(&cam->buf, 0, sizeof cam->buf);
		cam->buf.index = i;
		cam->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		cam->buf.memory = V4L2_MEMORY_MMAP;
		ret = ioctl(cam->dev, VIDIOC_QBUF, &cam->buf);
		if (ret < 0) {
			printf("Unable to queue buffer (%d).\n", errno);
			close(cam->dev);
			return 1;
		}
	}
	printf("ok!\n");
}

int grab_frame(Cam *cam)
{
	int ret = 0;
	printf("Dequeue!\n");
	/* Dequeue a buffer. */
	memset(&cam->buf, 0, sizeof cam->buf);
	cam->buf.index = 0;
	cam->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	cam->buf.memory = V4L2_MEMORY_MMAP;
	ret = ioctl(cam->dev, VIDIOC_DQBUF, &cam->buf);
	if (ret < 0) {
		printf("Unable to dequeue buffer (%d).\n", errno);
		close(cam->dev);
		return 1;
	}
	printf("Dequeue!\n");
	memcpy(cam->framebuffer, cam->mem[cam->buf.index], (size_t) cam->buf.bytesused);

	/* Requeue the buffer. */
	ret = ioctl(cam->dev, VIDIOC_QBUF, &cam->buf);
	if (ret < 0) {
		printf("Unable to requeue buffer (%d).\n", errno);
		close(cam->dev);
		return 1;
	}	
}

static int video_set_format(int dev, unsigned int w, unsigned int h, unsigned int format)
{
	struct v4l2_format fmt;
	int ret;

	printf("video_set_format: width: %u height: %u\n", w, h);

	memset(&fmt, 0, sizeof fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = w;
	fmt.fmt.pix.height = h;
	fmt.fmt.pix.pixelformat = format;
	fmt.fmt.pix.field = V4L2_FIELD_ANY;

	ret = ioctl(dev, VIDIOC_S_FMT, &fmt);
	if (ret < 0) {
		printf("Unable to set format: %d.\n", errno);
		return ret;
	}

	printf("Video format set: width: %u height: %u buffer size: %u\n",
		fmt.fmt.pix.width, fmt.fmt.pix.height, fmt.fmt.pix.sizeimage);
	return 0;
}

static int video_set_framerate(int dev)
{
	struct v4l2_streamparm parm;
	int ret;

	memset(&parm, 0, sizeof parm);
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = ioctl(dev, VIDIOC_G_PARM, &parm);
	if (ret < 0) {
		printf("Unable to get frame rate: %d.\n", errno);
		return ret;
	}

	printf("Current frame rate: %u/%u\n",
		parm.parm.capture.timeperframe.numerator,
		parm.parm.capture.timeperframe.denominator);

	parm.parm.capture.timeperframe.numerator = 1;
	parm.parm.capture.timeperframe.denominator = 30;

	ret = ioctl(dev, VIDIOC_S_PARM, &parm);
	if (ret < 0) {
		printf("Unable to set frame rate: %d.\n", errno);
		return ret;
	}

	ret = ioctl(dev, VIDIOC_G_PARM, &parm);
	if (ret < 0) {
		printf("Unable to get frame rate: %d.\n", errno);
		return ret;
	}

	printf("Frame rate set: %u/%u\n",
		parm.parm.capture.timeperframe.numerator,
		parm.parm.capture.timeperframe.denominator);

	return 0;
}

static int video_reqbufs(int dev, int nbufs)
{
	struct v4l2_requestbuffers rb;
	int ret;

	memset(&rb, 0, sizeof rb);
	rb.count = nbufs;
	rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	rb.memory = V4L2_MEMORY_MMAP;

	ret = ioctl(dev, VIDIOC_REQBUFS, &rb);
	if (ret < 0) {
		printf("Unable to allocate buffers: %d.\n", errno);
		return ret;
	}

	printf("%u buffers allocated.\n", rb.count);
	return rb.count;
}

int enum_frame_intervals(int dev, __u32 pixfmt, __u32 width, __u32 height)
{
        int ret;
        struct v4l2_frmivalenum fival;

        memset(&fival, 0, sizeof(fival));
        fival.index = 0;
        fival.pixel_format = pixfmt;
        fival.width = width;
        fival.height = height;
        printf("\tTime interval between frame: ");
        while ((ret = ioctl(dev, VIDIOC_ENUM_FRAMEINTERVALS, &fival)) == 0) {
                if (fival.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
                                printf("%u/%u, ",
                                                fival.discrete.numerator, fival.discrete.denominator);
                } else if (fival.type == V4L2_FRMIVAL_TYPE_CONTINUOUS) {
                                printf("{min { %u/%u } .. max { %u/%u } }, ",
                                                fival.stepwise.min.numerator, fival.stepwise.min.numerator,
                                                fival.stepwise.max.denominator, fival.stepwise.max.denominator);
                                break;
                } else if (fival.type == V4L2_FRMIVAL_TYPE_STEPWISE) {
                                printf("{min { %u/%u } .. max { %u/%u } / "
                                                "stepsize { %u/%u } }, ",
                                                fival.stepwise.min.numerator, fival.stepwise.min.denominator,
                                                fival.stepwise.max.numerator, fival.stepwise.max.denominator,
                                                fival.stepwise.step.numerator, fival.stepwise.step.denominator);
                                break;
                }
                fival.index++;
        }
        printf("\n");
        if (ret != 0 && errno != EINVAL) {
                perror("ERROR enumerating frame intervals");
                return errno;
        }

        return 0;
}

static int enum_frame_sizes(int dev, __u32 pixfmt)
{
        int ret;
        struct v4l2_frmsizeenum fsize;

        memset(&fsize, 0, sizeof(fsize));
        fsize.index = 0;
        fsize.pixel_format = pixfmt;
        while ((ret = ioctl(dev, VIDIOC_ENUM_FRAMESIZES, &fsize)) == 0) {
                if (fsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
                        printf("{ discrete: width = %u, height = %u }\n",
                                        fsize.discrete.width, fsize.discrete.height);
                        ret = enum_frame_intervals(dev, pixfmt,
                                        fsize.discrete.width, fsize.discrete.height);
                        if (ret != 0)
                                printf("  Unable to enumerate frame sizes.\n");
                } else if (fsize.type == V4L2_FRMSIZE_TYPE_CONTINUOUS) {
                        printf("{ continuous: min { width = %u, height = %u } .. "
                                        "max { width = %u, height = %u } }\n",
                                        fsize.stepwise.min_width, fsize.stepwise.min_height,
                                        fsize.stepwise.max_width, fsize.stepwise.max_height);
                        printf("  Refusing to enumerate frame intervals.\n");
                        break;
                } else if (fsize.type == V4L2_FRMSIZE_TYPE_STEPWISE) {
                        printf("{ stepwise: min { width = %u, height = %u } .. "
                                        "max { width = %u, height = %u } / "
                                        "stepsize { width = %u, height = %u } }\n",
                                        fsize.stepwise.min_width, fsize.stepwise.min_height,
                                        fsize.stepwise.max_width, fsize.stepwise.max_height,
                                        fsize.stepwise.step_width, fsize.stepwise.step_height);
                        printf("  Refusing to enumerate frame intervals.\n");
                        break;
                }
                fsize.index++;
        }
        if (ret != 0 && errno != EINVAL) {
                perror("ERROR enumerating frame sizes");
                return errno;
        }

        return 0;
}

void video_list_formats(int dev)
{
        struct v4l2_fmtdesc fmt;
	int ret;

        memset(&fmt, 0, sizeof(fmt));
        fmt.index = 0;
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        while ((ret = ioctl(dev, VIDIOC_ENUM_FMT, &fmt)) == 0) {
                printf("{ pixelformat = '%c%c%c%c', description = '%s' }\n",
                        fmt.pixelformat & 0xFF, (fmt.pixelformat >> 8) & 0xFF,
                        (fmt.pixelformat >> 16) & 0xFF, (fmt.pixelformat >> 24) & 0xFF,
                        fmt.description);
                        ret = enum_frame_sizes(dev, fmt.pixelformat);
                        if(ret != 0)
                                printf("  Unable to enumerate frame sizes.\n");

                fmt.index++;
        }

        if (errno != EINVAL) {
                perror("ERROR enumerating frame formats");
        }
}

static void video_enum_inputs(int dev)
{
	struct v4l2_input input;
	unsigned int i;
	int ret;

	for (i = 0; ; ++i) {
		memset(&input, 0, sizeof input);
		input.index = i;
		ret = ioctl(dev, VIDIOC_ENUMINPUT, &input);
		if (ret < 0)
			break;

		if (i != input.index)
			printf("Warning: driver returned wrong input index "
				"%u.\n", input.index);

		printf("Input %u: %s.\n", i, input.name);
	}
}

static int video_get_input(int dev)
{
	__u32 input;
	int ret;

	ret = ioctl(dev, VIDIOC_G_INPUT, &input);
	if (ret < 0) {
		printf("Unable to get current input: %s.\n", strerror(errno));
		return ret;
	}

	return input;
}

static int video_set_input(int dev, unsigned int input)
{
	__u32 _input = input;
	int ret;

	ret = ioctl(dev, VIDIOC_S_INPUT, &_input);
	if (ret < 0)
		printf("Unable to select input %u: %s.\n", input,
			strerror(errno));

	return ret;
}