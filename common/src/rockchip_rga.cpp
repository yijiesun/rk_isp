#include <rockchip_rga.hpp>

rockchip_rga::rockchip_rga(void)
{
	rga = NULL;
}


rockchip_rga::~rockchip_rga(void)
{
	if(rga)
		RgaDestroy(rga);
}

int rockchip_rga::init(void)
{
	rga = RgaCreate();
	if(rga == NULL) {
		fprintf(stderr, "Create rga failed\n");
		return -EIO;
	}

	return 0;
}

int rockchip_rga::convert(void *src_buf, void *dst_buf, struct rga_format src_fmt,
		      struct rga_format dst_fmt, struct rga_crop crop, RgaRotate rotate)
{
	rga->ops->initCtx(rga);
	rga->ops->setSrcBufferPtr(rga, src_buf);
	rga->ops->setDstBufferPtr(rga, dst_buf);

	rga->ops->setSrcFormat(rga, src_fmt.format, src_fmt.width, src_fmt.height);
	rga->ops->setDstFormat(rga, dst_fmt.format, dst_fmt.width, dst_fmt.height);

	rga->ops->setRotate(rga, rotate);
	if(crop.w > 0 && crop.h > 0) {
		rga->ops->setSrcCrop(rga, crop.x, crop.y, crop.w, crop.h);
		rga->ops->setDstCrop(rga, 0, 0, dst_fmt.width, dst_fmt.height);
	}

	return rga->ops->go(rga);
}
