#ifndef _V4L2_DEV_FRAMED_SOURCE_H_
#define _V4L2_DEV_FRAMED_SOURCE_H_

#include "DevFramedSource.h"
#include "V4l2.h"

class V4l2DevFramedSource : public DevFramedSource
{
protected:
    V4l2DevFramedSource(UsageEnvironment& env, const char* dev,
                        int width=640, int height=480,
                        uint32_t pixelFmt=V4L2_PIX_FMT_YUYV);
    virtual ~V4l2DevFramedSource();

    virtual bool createFrame(Frame* frame);

    int width() const { return mWidth; }
    int height() const { return mHeight; }
    uint32_t pixelFmt() const { return mPixelFmt; }

    virtual bool getFrame(Frame* frame) = 0;

    /* v4l2BufUnit：in；frame：out */
    virtual bool encode(struct v4l2_buf_unit* v4l2BufUnit, Frame* frame) = 0;

private:
    bool videoInit();
    bool videoExit();

private:
    int mFd;
    int mWidth;
    int mHeight;
    uint32_t mPixelFmt;

    struct v4l2_buf* mV4l2Buf;
    struct v4l2_buf_unit* mV4l2BufUnit;
};

#endif //_V4L2_DEV_FRAMED_SOURCE_H_