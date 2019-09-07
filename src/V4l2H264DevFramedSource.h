#ifndef _V4L2_H264_DEV_FRAMED_SOURCE_H_
#define _V4L2_H264_DEV_FRAMED_SOURCE_H_
#include <queue>

#include "V4l2DevFramedSource.h"
#include <x264.h>

class V4l2H264DevFramedSource : public V4l2DevFramedSource
{
public:
    static V4l2H264DevFramedSource* createNew(UsageEnvironment& env, const char* dev,
                                                int width=320, int height=240, int fps=15);

protected:
    struct Nalu
    {
        Nalu(uint8_t* data, uint32_t size, timeval time, uint32_t duration) :
            mData(data), mSize(size), mTime(time), mDurationInMicroseconds(duration)
            {  }

        uint8_t* mData;
        uint32_t mSize;
        timeval mTime;
        uint32_t mDurationInMicroseconds;
    };

    V4l2H264DevFramedSource(UsageEnvironment& env, const char* dev,
                            int width, int height, int fps);
    virtual ~V4l2H264DevFramedSource();
 
    virtual bool getFrame(Frame* frame);
    virtual bool encode(struct v4l2_buf_unit* v4l2BufUnit, Frame* frame);

private:
    bool x264Init();
    void x264Exit();

private:
	x264_nal_t* mNals;
	x264_t* mX264Handle;
	x264_picture_t* mPicIn;
	x264_picture_t* mPicOut;
	x264_param_t* mParam;
    int mCsp;
    int mPts;

    Frame* mSPSFrame;
    Frame* mPPSFrame;

    std::queue<Nalu> mNaluQueue;
};

#endif //_V4L2_DEV_FRAMED_SOURCE_H_