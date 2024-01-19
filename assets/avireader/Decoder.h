#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

#define CODEC_CAP_TRUNCATED (1 << 3)
#define CODEC_FLAG_TRUNCATED (1 << 16)
#define H264_INBUF_SIZE 16384 /// number of bytes we read per chunk.
#define FF_INPUT_BUFFER_PADDING_SIZE 32

namespace H264
{
class Decoder
{

  public:
    Decoder();
    ~Decoder(); /// cleans up the allocated objects and closes the codec context.
    bool     initialize(std::string filepath, float fps = 0.0f); /// load a video file which is encoded with x264.
    bool     readFrame();
    AVFrame* getPictureFrame() const; /// read a frame if necessary.

  private:
    void     decodeFrame(uint8_t* data, int size); /// decode a frame we read from the buffer.
    int      readBuffer();                         /// read a bit more data from the buffer.
    bool     update(bool& needsMoreBytes); /// internally used to update/parse the data we read from the buffer or file.
    uint64_t rxhrtime();

  public:
    std::vector<uint8_t>  mBuffer;       /// buffer we use to keep track of read/unused bitstream data.
    const AVCodec*        mCodec;        /// the AVCodec* which represents the H264 decoder.
    AVCodecContext*       mCodecContext; /// the context; keeps generic state.
    uint64_t              mFrameTimeout; /// timeout when we need to parse a new frame.
    uint64_t              mFrameDelay;   /// delay between frames (in ns).
    FILE*                 mFp;           /// file pointer to the file from which we read the h264 data.
    int                   mFrameNumber;  /// the number of decoded frames.
    uint8_t               mInbuf[H264_INBUF_SIZE + FF_INPUT_BUFFER_PADDING_SIZE]; /// used to read chunks from the file.
    AVCodecParserContext* mParser;  /// parser that is used to decode the h264 bitstream.
    AVFrame*              mPicture; /// will contain a decoded picture.
    float                 mSamplingTime;
};
} // namespace H264
