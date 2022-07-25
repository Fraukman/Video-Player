#include "VideoReader.h"

bool VideoReader::videoReaderOpen(const char* filename)
{
	avFormatCtx = avformat_alloc_context();
	if (!avFormatCtx)
	{
		std::cerr << "Couldn't created AVFormatContext" << std::endl;
		return false;
	}
	if (avformat_open_input(&avFormatCtx, filename, nullptr, nullptr) != 0) {
		std::cerr << "Couldn't open the video file" << std::endl;
		return false;
	}

	videoStreamIndex = -1;
	AVCodecParameters* avCodecParams = nullptr;
	AVCodec* avCodec = nullptr;

	for (int i = 0; i < avFormatCtx->nb_streams; i++) {
		avCodecParams = avFormatCtx->streams[i]->codecpar;
		avCodec = avcodec_find_decoder(avCodecParams->codec_id);

		if (!avCodec)
		{
			continue;
		}

		if (avCodecParams->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoStreamIndex = i;
			width = avCodecParams->width;
			height = avCodecParams->height;
			break;
		}
	}

	if (videoStreamIndex == -1)
	{
		std::cerr << "Couldn't find video stream inside file" << std::endl;
		return false;
	}

	//set up a codec context for the decoder
	avCodecCtx = avcodec_alloc_context3(avCodec);
	if (!avCodecCtx)
	{
		std::cerr << "Couldn't create AVCodecContext" << std::endl;
		return false;
	}

	if (avcodec_parameters_to_context(avCodecCtx, avCodecParams) < 0)
	{
		std::cerr << "Couldn't initialize AVCodecContext" << std::endl;
		return false;
	}
	if (avcodec_open2(avCodecCtx, avCodec, nullptr) < 0)
	{
		std::cerr << "Couldn't open codec" << std::endl;
		return false;
	}

	avFrame = av_frame_alloc();
	if (!avFrame)
	{
		std::cerr << "Couldn't allocate AVFrame" << std::endl;
		return false;
	}
	avPacket = av_packet_alloc();
	if (!avPacket)
	{
		std::cerr << "Couldn't allocate avPacket" << std::endl;
		return false;
	}

	return true;
}


bool VideoReader::videoReaderReadFrame(uint8_t* frameBuffer)
{

	while (av_read_frame(avFormatCtx, avPacket) >= 0) {
		if (avPacket->stream_index != videoStreamIndex)
		{
			continue;
		}
		int response = avcodec_send_packet(avCodecCtx, avPacket);
		if (response < 0)
		{
			std::cerr << "Couldn't decode avPacket: " << std::endl;
			return false;
		}

		response = avcodec_receive_frame(avCodecCtx, avFrame);
		if (response == AVERROR(EAGAIN) || response == AVERROR_EOF)
		{
			continue;
		}
		else if (response < 0) {
			std::cerr << "Couldn't decode avPacket: " << std::endl;
			return false;
		}

		av_packet_unref(avPacket);
		break;
	}

	if (!swsScalerCtx)
	{
		swsScalerCtx = sws_getContext(width, height, avCodecCtx->pix_fmt,
									  width, height, AV_PIX_FMT_RGB0,
									  SWS_BILINEAR, NULL, NULL, NULL);
	}

	if (!swsScalerCtx) {
		std::cerr << "Couldn't initialize swsScaler " << std::endl;
		return false;
	}

	uint8_t* dest[4] = { frameBuffer,NULL,NULL,NULL };
	int destLinesize[4] = { width * 4,0,0,0 };	
	sws_scale(swsScalerCtx, avFrame->data, avFrame->linesize, 0, avFrame->height, dest, destLinesize);

	return true;
}

void VideoReader::videoReaderClose()
{
	avformat_close_input(&avFormatCtx);
	avformat_free_context(avFormatCtx);
	av_frame_free(&avFrame);
	av_packet_free(&avPacket);
	avcodec_free_context(&avCodecCtx);
	sws_freeContext(swsScalerCtx);
}
