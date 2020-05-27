#include <functional>
#include <iostream>
#include <pthread.h>
#include <sys/syscall.h>
#include <getopt.h>
#include <chrono>
#include <asm/types.h>
#include <assert.h>
#include <fcntl.h>
#include <dirent.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <linux/stddef.h>
#include <linux/videodev2.h>
#include <errno.h>

extern "C" {
    #include <rockchip/rockchip_rga.h>
}

struct rga_crop {
	__u32 x;
	__u32 y;
	__u32 w;
	__u32 h;
};

struct rga_format {
	__u32 width;
	__u32 height;
	__u32 format;
};

class rockchip_rga {
public:
	rockchip_rga(void);
	~rockchip_rga(void);
	int init(void);
	int convert(void *src_buf, void *dst_buf, struct rga_format src_fmt,
		struct rga_format dst_fmt, struct rga_crop crop, RgaRotate rotate);

private:
	RockchipRga *rga;
};
