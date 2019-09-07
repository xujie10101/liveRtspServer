#include <assert.h>

#include "V4l2H264DevFramedSource.h"

V4l2H264DevFramedSource* V4l2H264DevFramedSource::createNew(UsageEnvironment& env, const char* dev,
                                                            int width, int height, int fps)
{
    return new V4l2H264DevFramedSource(env, dev, width, height, fps);
}

V4l2H264DevFramedSource::V4l2H264DevFramedSource(UsageEnvironment& env, const char* dev,
                                                int width, int height, int fps) :
    V4l2DevFramedSource(env, dev, width, height),
    mPts(0)
{
    bool ret;

    setFps(fps);

    ret = x264Init();
    assert(ret == true);

    mSPSFrame = new Frame(100);
    mPPSFrame = new Frame(100);
}

V4l2H264DevFramedSource::~V4l2H264DevFramedSource()
{
    x264Exit();
    delete mSPSFrame;
    delete mPPSFrame;
}

bool V4l2H264DevFramedSource::getFrame(Frame* frame)
{
    if(mNaluQueue.empty())
        return false;
    
    Nalu nalu = mNaluQueue.front();
    mNaluQueue.pop();
    memcpy(frame->mFrame, nalu.mData, nalu.mSize);
    frame->mTime = nalu.mTime;
    frame->mDurationInMicroseconds = nalu.mDurationInMicroseconds;
    frame->mFrameSize = nalu.mSize;

    return true;
}

static inline int startCode3(uint8_t* buf)
{
    if(buf[0] == 0 && buf[1] == 0 && buf[2] == 1)
        return 1;
    else
        return 0;
}

static inline int startCode4(uint8_t* buf)
{
    if(buf[0] == 0 && buf[1] == 0 && buf[2] == 0 && buf[3] == 1)
        return 1;
    else
        return 0;
}

bool V4l2H264DevFramedSource::encode(struct v4l2_buf_unit* v4l2BufUnit, Frame* frame)
{
    int nalNum, startCode;
    uint8_t byte;

    switch(pixelFmt())
    {
    case V4L2_PIX_FMT_YUYV:
        memcpy(mPicIn->img.plane[0], v4l2BufUnit->start, v4l2BufUnit->length);   
        break;
    
    default:
        return false;
    }

    mPicIn->i_pts = mPts++;
    x264_encoder_encode(mX264Handle, &mNals, &nalNum, mPicIn, mPicOut);

    if(nalNum <= 0)
        return false;
    
    timeval time;
    gettimeofday(&time, NULL);
    for(int i = 0; i < nalNum; ++i)
    {
        if(startCode3(mNals[i].p_payload))
            startCode = 3;
        else
            startCode = 4;

        byte = *(mNals[i].p_payload+startCode);
        if((byte&0x1F) == 7) //sps
        {
            memcpy(mSPSFrame->mFrame, mNals[i].p_payload+startCode, mNals[i].i_payload-startCode);
            mSPSFrame->mFrameSize = mNals[i].i_payload-startCode;
            mSPSFrame->mDurationInMicroseconds = 0;
            mSPSFrame->mTime = time;
        }
        else if((byte&0x1F) == 8) //pps
        {
            memcpy(mPPSFrame->mFrame, mNals[i].p_payload+startCode, mNals[i].i_payload-startCode);
            mPPSFrame->mFrameSize = mNals[i].i_payload-startCode;
            mPPSFrame->mDurationInMicroseconds = 0;
            mPPSFrame->mTime = time;
        }
        else //I Frame and other
        {
            if((byte&0x1F) == 5)
            {
                mNaluQueue.push(Nalu(mSPSFrame->mFrame, mSPSFrame->mFrameSize, time, 0));
                mNaluQueue.push(Nalu(mPPSFrame->mFrame, mPPSFrame->mFrameSize, time, 0));
            }

            mNaluQueue.push(Nalu(mNals[i].p_payload+startCode, mNals[i].i_payload-startCode, time, 1000000/fps()));
        }
    }

    Nalu nalu = mNaluQueue.front();
    mNaluQueue.pop();
    memcpy(frame->mFrame, nalu.mData, nalu.mSize);
    frame->mFrameSize = nalu.mSize;
    frame->mTime = nalu.mTime;
    frame->mDurationInMicroseconds = nalu.mDurationInMicroseconds;

    return true;
}

bool V4l2H264DevFramedSource::x264Init()
{
    switch (pixelFmt())
    {
    case V4L2_PIX_FMT_YUYV:
        mCsp = X264_CSP_YUYV;
        break;
    
    default:
        envir()<<"can't support this fmt\n";
        return false;
    }

	mNals = NULL;
	mX264Handle = NULL;
	mPicIn = new x264_picture_t;
	mPicOut = new x264_picture_t;
	mParam = new x264_param_t;

    x264_param_default(mParam);
	mParam->i_width = width();
    mParam->i_height = height();
	mParam->i_keyint_max = fps();
	mParam->i_fps_num = fps();
	mParam->i_csp = mCsp;

	mX264Handle = x264_encoder_open(mParam);
	if(!mX264Handle)
	{
		envir()<<"failed to open x264 encoder\n";
		return false;
	}

	x264_picture_init(mPicOut);
    x264_picture_alloc(mPicIn, mCsp, width(), height());

    return true;
}

void V4l2H264DevFramedSource::x264Exit()
{
    x264_picture_clean(mPicIn);
    x264_encoder_close(mX264Handle);

    delete mPicIn;
    delete mPicOut;
    delete mParam;
}