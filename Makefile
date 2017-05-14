CROSS=arm-none-linux-gnueabi-

all: my_uvc_camera

my_uvc_camera: my_uvc_camera.o uvc_camera.o server.o
	$(CROSS)gcc -o my_uvc_camera $^

my_uvc_camera.o: my_uvc_camera.c uvc_camera.h
	$(CROSS)gcc -c my_uvc_camera.c  -ljpeg

uvc_camera.o: uvc_camera.c uvc_camera.h
	$(CROSS)gcc -c uvc_camera.c

server.o: server.c server.h
	$(CROSS)gcc -c server.c -lpthread 

clean:
	@rm -vf my_uvc_camera *.o *~

