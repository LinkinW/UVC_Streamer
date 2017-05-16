CROSS=arm-none-linux-gnueabi-

CFLAGS = -lpthread -ljpeg -L/home/xubuntu/embest/work/jpeg-8b/jpeg/lib/lib

all: my_uvc_camera

my_uvc_camera: my_uvc_camera.o uvc_camera.o server.o jpeg_utils.o
	$(CROSS)gcc -o my_uvc_camera $^ $(CFLAGS)

my_uvc_camera.o: my_uvc_camera.c uvc_camera.h
	$(CROSS)gcc -c my_uvc_camera.c  $(CFLAGS)

uvc_camera.o: uvc_camera.c uvc_camera.h
	$(CROSS)gcc -c uvc_camera.c $(CFLAGS)

server.o: server.c server.h
	$(CROSS)gcc -c server.c  $(CFLAGS)

jpeg_utils.o: jpeg_utils.c jpeg_utils.h
	$(CROSS)gcc -c jpeg_utils.c $(CFLAGS)

clean:
	@rm -vf my_uvc_camera *.o *~


