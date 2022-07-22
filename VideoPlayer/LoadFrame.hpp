#pragma once

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <inttypes.h>
};

#include <iostream>

bool LoadFrame(const char* filename,int* width_out, int* height_out,unsigned char** data_out){
	
	AVFormatContext* av_format_ctx = avformat_alloc_context();
	if (!av_format_ctx)
	{
		std::cerr << "Couldn't created AVFormatContext" << std::endl;
		return false;
	}
	if(avformat_open_input(&av_format_ctx,filename,nullptr,nullptr) != 0){
		std::cerr << "Couldn't open the video file" << std::endl;
		return false;
	}

	int videoStreamIndex = -1;
	AVCodecParameters* avCodecParams = nullptr;
	AVCodec* avCodec = nullptr;

	for(int i = 0; i < av_format_ctx->nb_streams;i++){
		auto strem = av_format_ctx->streams[i];

		avCodecParams = av_format_ctx->streams[i]->codecpar;
		avCodec = avcodec_find_decoder(avCodecParams->codec_id);

		if (!avCodec)
		{
			continue;
		}
	
		if (avCodecParams->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoStreamIndex = i;
			break;
		}
	}

	if (videoStreamIndex == -1)
	{
		std::cerr << "Couldn't find video stream inside file" << std::endl;
		return false;
	}

	//set up a codec context for the decoder
	AVCodecContext* avCodecCtx = avcodec_alloc_context3(avCodec);
	if (!avCodecCtx)
	{
		std::cerr << "Couldn't create AVCodecContext" << std::endl;
		return false;
	}

	if (avcodec_parameters_to_context(avCodecCtx,avCodecParams) < 0)
	{
		std::cerr << "Couldn't initialize AVCodecContext" << std::endl;
		return false;
	}
	if (avcodec_open2(avCodecCtx,avCodec,nullptr) < 0)
	{
		std::cerr << "Couldn't open codec" << std::endl;
		return false;
	}

	AVFrame* avFrame = av_frame_alloc();
	if (!avFrame)
	{
		std::cerr << "Couldn't allocate AVFrame" << std::endl;
		return false;

	}
	AVPacket* avPacket = av_packet_alloc();
	if (!avPacket)
	{
		std::cerr << "Couldn't allocate avPacket" << std::endl;
		return false;

	}

	while( av_read_frame(av_format_ctx, avPacket) >= 0){
		if (avPacket->stream_index != videoStreamIndex)
		{
			continue;
		}
		int response = avcodec_send_packet(avCodecCtx, avPacket);
		if (response<0)
		{
			std::cerr << "Couldn't decode avPacket: "<< std::endl;
			return false;
		}

		response = avcodec_receive_frame(avCodecCtx, avFrame);
		if (response==AVERROR(EAGAIN) || response == AVERROR_EOF)
		{
			continue;
		}else if (response < 0 ){
			std::cerr << "Couldn't decode avPacket: "  << std::endl;
			return false;
		}

		av_packet_unref(avPacket);
		break;
	}

	uint8_t* data = new uint8_t[avFrame->width * avFrame->height * 4];

	SwsContext* swsScalerCtx = sws_getContext(avFrame->width,avFrame->height,avCodecCtx->pix_fmt, avFrame->width, avFrame->height,AV_PIX_FMT_RGB0,SWS_BILINEAR,NULL,NULL,NULL);

	if(!swsScalerCtx){
		std::cerr << "Couldn't initialize swsScaler " << std::endl;
		return false;
	}

	uint8_t* dest[4] = { data,NULL,NULL,NULL };
	int destLinesize[4] = { avFrame->width * 4,0,0,0 };
	sws_scale(swsScalerCtx, avFrame->data, avFrame->linesize, 0, avFrame->height, dest, destLinesize);
	sws_freeContext(swsScalerCtx);

	*width_out = avFrame->width;
	*height_out = avFrame->height;
	*data_out = data;

	avformat_close_input(&av_format_ctx);
	avformat_free_context(av_format_ctx);
	av_frame_free(&avFrame);
	av_packet_free(&avPacket);
	avcodec_free_context(&avCodecCtx);

	return true;
}
