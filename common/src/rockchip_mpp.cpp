#include <rockchip_mpp.hpp>

mpp_decoder::mpp_decoder(void)
{
	dec = NULL;
}

mpp_decoder::~mpp_decoder(void)
{
	if(dec)
		MppDecoderDestroy(dec);
}

int mpp_decoder::init(DecodeType type)
{
	dec = MppDecoderCreate(type);
	if(!dec) {
		fprintf(stderr, "Create mpp decoder failed\n");
		return -EIO;
	}

	return 0;
}

int mpp_decoder::enqueue(void *buf, size_t len)
{
	return dec->ops->enqueue(dec, buf, len);
}

DecFrame *mpp_decoder::dequeue(void)
{
	return dec->ops->dequeue(dec);
}

void mpp_decoder::freeFrame(DecFrame *frame)
{
	dec->ops->freeFrame(frame);
}

/* Mpp Encoder */
mpp_encoder::mpp_encoder(void)
{
}

mpp_encoder::~mpp_encoder(void)
{
	if(enc)
		MppEncoderDestroy(enc);
}

int mpp_encoder::init(EncodeType type, __u32 format, __u32 width, __u32 height, size_t size)
{
	ctx.width = width;
	ctx.height = height;
	ctx.v4l2Format = format;
	ctx.size = size;
	ctx.fps = 30;
	ctx.gop = 60;
	ctx.mode = ENCODE_RC_MODE_CBR;
	ctx.quality = ENCODE_QUALITY_MEDIUM;
	enc = MppEncoderCreate(ctx, type);
	if(!enc) {
		fprintf(stderr, "Create mpp encoder failed\n");
		return -EIO;
	}

	return 0;
}

int mpp_encoder::importFds(int *fd, int count)
{
	int i, ret;

	if(count > ENCODE_BUFFER_COUNT) {
		fprintf(stderr, "buffer count should be less than %d\n", ENCODE_BUFFER_COUNT);
		return -EINVAL;
	}

	for(i = 0; i < count; i++) {
		ret = enc->ops->importBufferFd(enc, fd[i], i);
		if(ret < 0)
			return ret;	
	}

	return 0;
}

EncPacket *mpp_encoder::extraData(void)
{
	return enc->ops->getExtraData(enc);
}

int mpp_encoder::enqueue(int index)
{
	return enc->ops->enqueue(enc, index);
}

EncPacket *mpp_encoder::dequeue(void)
{
	return enc->ops->dequeue(enc);
}

void mpp_encoder::freePacket(EncPacket *packet)
{
	if(packet)
		enc->ops->freePacket(packet);
}
