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

#include <rockchip/rockchip_mpp.h>

class mpp_decoder {
public:
	mpp_decoder(void);
	~mpp_decoder(void);
	int init(DecodeType type);
	int enqueue(void *buf, size_t len);
	DecFrame *dequeue(void);
	void freeFrame(DecFrame *frame);
private:
	MppDecoder *dec;
};


class mpp_encoder {
public:
	mpp_encoder(void);
	~mpp_encoder(void);
	int init(EncodeType type, __u32 format, __u32 width, __u32 height, size_t size);
	/* import fds, which are allocated by librockchip_drm */
	int importFds(int *fd, int count);
	/* Get extra data(sps and pps) */ 
	EncPacket *extraData(void);
	int enqueue(int index);
	EncPacket *dequeue(void);
	void freePacket(EncPacket *packet);
private:
	EncCtx ctx;
	MppEncoder *enc;
};
