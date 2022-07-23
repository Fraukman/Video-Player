#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <inttypes.h>
};

#include <iostream>

struct VideoReaderState {
	int width;
	int height;

	AVFormatContext* avFormatCtx;
	AVCodecContext* avCodecCtx;
	AVFrame* avFrame;
	AVPacket* avPacket;
	SwsContext* swsScalerCtx = nullptr;
	int videoStreamIndex;
};

bool videoReaderOpen(VideoReaderState* state, const char* filename);
bool videoReaderReadFrame(VideoReaderState* state, uint8_t* frameBuffer);
void videoReaderClose(VideoReaderState* state);