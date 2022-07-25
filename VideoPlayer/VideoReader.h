#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <inttypes.h>
};

#include <iostream>

class VideoReader {
public:
	VideoReader() {};

	bool videoReaderOpen(const char* filename);
	bool videoReaderReadFrame(uint8_t * frameBuffer);
	void videoReaderClose();

public:
	int width;
	int height;

private:
	AVFormatContext* avFormatCtx;
	AVCodecContext* avCodecCtx;
	AVFrame* avFrame;
	AVPacket* avPacket;
	SwsContext* swsScalerCtx = nullptr;
	int videoStreamIndex;
};

