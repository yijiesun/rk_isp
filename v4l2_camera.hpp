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

#include <rockchip/rockchip_isp.h>

typedef enum __camera_type {
	CAMERA_TYPE_USB,
	CAMERA_TYPE_OV9750,
	CAMERA_TYPE_IMX258,
} CameraType;

struct camera_buffer {
	int fd;
	void *start;
	size_t length;
};

class v4l2_camera {
public:
	v4l2_camera(void);
	~v4l2_camera(void);
	int init(const char *dev, CameraType t);
	int setfmt(__u32 w, __u32 h, __u32 f);
	int reqbufs_userptr(int count, void **ptrs, size_t len);
	int reqbufs_dma(int count, int *fds, size_t len);
	int reqbufs_mmap(int count);
	int start(void);
	int enqueue(int index);
	void *dequeue(int *index, size_t *len);

private:
	int fd;
	__u32 width;
	__u32 height;
	__u32 format;

	CameraType type;
	void* rkengine;

	struct camera_buffer *buffers;
	int n_buffers;
	__u32 memory;
};
