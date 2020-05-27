LDDFLAGS= -lopencv_highgui -lopencv_core -lopencv_videoio -lrockchip_drm -lcam_engine_cifisp -lcam_ia -lpthread -lrockchip_rga
CPPFLAG = -I./common/include -I/usr/include/libdrm

SRC = example.cpp \
      ./common/src/rockchip_rga.cpp \
      v4l2_camera.cpp
all:
	g++ ${SRC} ${LDDFLAGS} ${CPPFLAG} -o example

clean:
	rm -rf example
