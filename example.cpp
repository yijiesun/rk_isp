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

#include <opencv2/opencv.hpp>

#include <rockchip_rga.hpp>
#include <v4l2_camera.hpp>

extern "C" {
    #include <rockchip/rockchip_drm.h>
}

using namespace cv;
using namespace std;

#define PLAY_FRAMES	12000
const string winname = "Usb Camera";

unsigned long long nanoTime()
{
    struct timespec t;

    t.tv_sec = t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);

    return (unsigned long long)(t.tv_sec) * 1000000000 + t.tv_nsec;
}

static void usage(char **argv)
{
    printf(
        "Usage: %s [Options]\n\n"
        "Options:\n"
        "-d, --device                 Usb camera device: 0, 1, 2 ...\n"
        "-w, --width                  Destination images's width\n"
        "-h, --height                 Destination images's height\n"
        "-r, --rotate                 Image rotation degree, the value should be 90, 180 or 270\n"
        "-V, --vflip                  Vertical Mirror\n" 
        "-H, --hflip                  Horizontal Mirror\n"
        "-c, --crop                   Crop, format: x,y,w,h\n"
        "\n",
        argv[0]);
}

static const char *short_options = "d:w:h:r:VHc:";

static struct option long_options[] = {
    {"device", required_argument, NULL, 'd'},
    {"width", required_argument, NULL, 'w'},
    {"height", required_argument, NULL, 'h'},
    {"rotate", required_argument, NULL, 'r'},
    {"vflip", no_argument, NULL, 'V'},
    {"hflip", no_argument, NULL, 'H'},
    {"crop", required_argument, NULL, 'c'},
    {NULL, 0, NULL, 0}
};

static void parse_crop_parameters(char *crop, __u32 *cropx, __u32 *cropy, __u32 *cropw, __u32 *croph)
{
    char *p, *buf;
    const char *delims = ".,";
    __u32 v[4] = {0,0,0,0};
    int i = 0;

    buf = strdup(crop);
    p = strtok(buf, delims);
    while(p != NULL) {
        v[i++] = atoi(p);
        p = strtok(NULL, delims);

        if(i >=4)
             break;
    }

    *cropx = v[0];
    *cropy = v[1];
    *cropw = v[2];
    *croph = v[3];
}

int usb_camera(char *dev, __u32 w, __u32 h, RgaRotate r, int V, int H, __u32 cx, __u32 cy, __u32 cw, __u32 ch, __u32 f, CameraType type)
{
    int ret = 0;
    int frames = PLAY_FRAMES;
    v4l2_camera camera;
    rockchip_rga rga;
    Mat mat;

    cv::namedWindow(winname);
    cv::moveWindow(winname, w, h);

    mat.create(cv::Size(RGA_ALIGN(w, 16), RGA_ALIGN(h, 16)), CV_8UC3);

    ret = rga.init();
    if(ret < 0)
	goto exit;

    /* TODO: Init camera deivce */
    ret = camera.init(dev, type);
    if(ret < 0)
	goto exit;

    /* TODO: Set picture format */
    ret = camera.setfmt(w, h, f);
    if(ret < 0)
	goto exit;

    /* TODO: Request and mmap buffers */
    ret = camera.reqbufs_mmap(4);
    if(ret < 0)
	goto exit;

    /* TODO: Start stream capture */
    ret = camera.start();
    if(ret < 0)
	goto exit;

    while(1) {
	int index;
	size_t len = 0;
	void *buf = NULL;
	struct rga_format src_fmt, dst_fmt;
	struct rga_crop crop;
	unsigned long long t1, t2, t3, t4;

	src_fmt.width = w;
	src_fmt.height = h;
	src_fmt.format = f;

	dst_fmt.width = w;
	dst_fmt.height = h;
	dst_fmt.format = V4L2_PIX_FMT_RGB24;

	crop.x = cx;
	crop.y = cy;
	crop.h = ch;
	crop.w = cw;

	t1 = nanoTime();
	/* TODO: Dequeue buffer */
	buf = camera.dequeue(&index, &len);
	t2 = nanoTime();
        ret = rga.convert(buf, mat.data, src_fmt, dst_fmt, crop, r);
	if(ret < 0)
	    goto exit;

	/* TODO: Enqueue buffer */
	camera.enqueue(index);

	t3 = nanoTime();
        imshow(winname, mat);

	cv::waitKey(1);
	t4 = nanoTime();

	printf("Picture size %llu, dequeue time %llums, rga time: %llums, display time: %llums\n",
	        len, (t2 - t1) / 1000000, (t3 - t2) / 1000000, (t4 - t3) / 1000000);

    }
	
exit:

    return ret;
}

int main(int argc, char **argv)
{
    int c, r;
    char index = -1;
    char devname[64];

    __u32 width = 640, height = 480;
    RgaRotate rotate = RGA_ROTATE_NONE;
    __u32 cropx = 0, cropy = 0, cropw = 0, croph = 0;
    int vflip = 0, hflip = 0;

    memset(devname, 0, 64);
    sprintf(devname, "/dev/video%d", 2);

    while((c = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
        switch (c) {
        case 'd':
            sprintf(devname, "/dev/video%d", atoi(optarg));
            break;
        case 'w':
            width = atoi(optarg);
            break;
        case 'h':
            height = atoi(optarg);
            break;
        case 'r':
            r = atoi(optarg);
            switch(r) {
                case 0: rotate = RGA_ROTATE_NONE; break;
                case 90: rotate = RGA_ROTATE_90; break;
                case 180: rotate = RGA_ROTATE_180; break;
                case 270: rotate = RGA_ROTATE_270; break;
                default:
                    printf("roate %d is not supported\n", r);
                    return -1; 
            }
            break;
        case 'V':
            vflip = 1;
            break;
        case 'H':
            hflip = 1;
            break;
        case 'c':
            parse_crop_parameters(optarg, &cropx, &cropy, &cropw, &croph);
            break;
        default:
            usage(argv);
            return 0; 
        }
    }

    printf("width = %u, height = %u, rotate = %u, vflip = %d, hflip = %d, crop = [%u, %u, %u, %u]\n",
            width, height, rotate, vflip, hflip, cropx, cropy, cropw, croph);

    if(vflip == 1)
        rotate = RGA_ROTATE_VFLIP;
    else if(hflip == 1)
        rotate = RGA_ROTATE_HFLIP;

    return usb_camera(devname, width, height, rotate, vflip, hflip, cropx, cropy, cropw, croph, V4L2_PIX_FMT_YUV420, CAMERA_TYPE_OV9750);
    //return usb_camera(devname, width, height, rotate, vflip, hflip, cropx, cropy, cropw, croph, V4L2_PIX_FMT_YUYV, CAMERA_TYPE_USB);
}
