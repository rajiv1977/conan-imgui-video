#include <iterator>
#include <iostream>
#include <WTypesbase.h>

#include "Decoder.h"

#if defined(_WIN32)
#pragma warning(disable : 4996) /// otherwise deprecated funcs cause compile errors
#endif

H264::Decoder::Decoder()
    : mCodec(NULL)
    , mCodecContext(NULL)
    , mParser(NULL)
    , mFp(NULL)
    , mFrameNumber(0)
    , mFrameTimeout(0)
    , mFrameDelay(0)
    , mSamplingTime(0.0F)
{
    avcodec_register_all();
}

H264::Decoder::~Decoder()
{

    if (mParser)
    {
        av_parser_close(mParser);
        mParser = NULL;
    }

    if (mCodecContext)
    {
        avcodec_close(mCodecContext);
        av_free(mCodecContext);
        mCodecContext = NULL;
    }

    if (mPicture)
    {
        av_free(mPicture);
        mPicture = NULL;
    }

    if (mFp)
    {
        fclose(mFp);
        mFp = NULL;
    }

    mFrameNumber  = 0;
    mFrameTimeout = 0;
}

bool H264::Decoder::initialize(std::string filepath, float fps)
{
    mSamplingTime = fps;
    mCodec        = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!mCodec)
    {
        std::cout << "Error: cannot find the h264 codec: " << filepath.c_str() << std::endl;
        return false;
    }

    mCodecContext = avcodec_alloc_context3(mCodec);

    if (mCodec->capabilities & CODEC_CAP_TRUNCATED)
    {
        mCodecContext->flags |= CODEC_FLAG_TRUNCATED;
    }

    if (avcodec_open2(mCodecContext, mCodec, NULL) < 0)
    {
        std::cout << "Error: could not open codec." << std::endl;
        return false;
    }

    mFp = fopen(filepath.c_str(), "rb");

    if (!mFp)
    {
        std::cout << "Error: cannot open: " << filepath.c_str() << std::endl;
        return false;
    }

    mPicture = av_frame_alloc();
    mParser  = av_parser_init(AV_CODEC_ID_H264);

    if (!mParser)
    {
        std::cout << "Erorr: cannot create H264 parser." << std::endl;
        return false;
    }

    if (fps > 0.0001f)
    {
        mFrameDelay   = (1.0f / fps) * 1000ull * 1000ull * 1000ull;
        mFrameTimeout = rxhrtime() + mFrameDelay;
    }

    readBuffer();

    return true;
}

bool H264::Decoder::readFrame()
{

    uint64_t now = rxhrtime();
    if (now < mFrameTimeout)
    {
        return false;
    }

    bool needs_more = false;

    while (!update(needs_more))
    {
        if (needs_more)
        {
            readBuffer();
            if (feof(mFp) || ferror(mFp))
            {
                return false;
            }
        }
    }

    // it may take some 'reads' before we can set the fps
    if (mFrameTimeout == 0 && mFrameDelay == 0)
    {
        double fps = av_q2d(mCodecContext->time_base);
        if (fps > 0.0)
        {
            mFrameDelay = fps * 1000ull * 1000ull * 1000ull;
        }
    }

    if (mFrameDelay > 0)
    {
        mFrameTimeout = rxhrtime() + mFrameDelay;
    }

    return true;
}

void H264::Decoder::decodeFrame(uint8_t* data, int size)
{

    AVPacket pkt;
    int      got_picture = 0;
    int      len         = 0;

    av_init_packet(&pkt);

    pkt.data = data;
    pkt.size = size;

    len         = avcodec_send_packet(mCodecContext, &pkt);
    got_picture = avcodec_receive_frame(mCodecContext, mPicture);

    if (len < 0)
    {
        std::cout << "Error while decoding a frame." << std::endl;
    }

    if (got_picture < 0)
    {
        return;
    }

    ++mFrameNumber;
    {
        mPicture->display_picture_number = static_cast<int>(mFrameNumber);
        mPicture->sample_rate            = mSamplingTime;
    }
}

AVFrame* H264::Decoder::getPictureFrame() const
{
    return mPicture;
}

int H264::Decoder::readBuffer()
{

    int bytes_read = (int) fread(mInbuf, 1, H264_INBUF_SIZE, mFp);

    if (bytes_read > 0)
    {
        std::copy(mInbuf, mInbuf + bytes_read, std::back_inserter(mBuffer));
    }

    return bytes_read;
}

bool H264::Decoder::update(bool& needsMoreBytes)
{

    needsMoreBytes = false;

    if (!mFp)
    {
        std::cout << "Cannot update .. file not opened..." << std::endl;
        return false;
    }

    if (mBuffer.size() == 0)
    {
        needsMoreBytes = true;
        return false;
    }

    uint8_t* data = NULL;
    int      size = 0;
    int len = av_parser_parse2(mParser, mCodecContext, &data, &size, &mBuffer[0], mBuffer.size(), 0, 0, AV_NOPTS_VALUE);

    if (size == 0 && len >= 0)
    {
        needsMoreBytes = true;
        return false;
    }

    if (len)
    {
        decodeFrame(&mBuffer[0], size);
        mBuffer.erase(mBuffer.begin(), mBuffer.begin() + len);
        return true;
    }

    return false;
}

uint64_t H264::Decoder::rxhrtime()
{
    LARGE_INTEGER timer_freq;
    LARGE_INTEGER timer_time;
    QueryPerformanceCounter(&timer_time);
    QueryPerformanceFrequency(&timer_freq);
    static double freq = (double) timer_freq.QuadPart / (double) 1000000000;
    return (uint64_t) ((double) timer_time.QuadPart / freq);
};
