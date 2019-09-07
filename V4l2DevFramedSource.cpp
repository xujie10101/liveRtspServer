#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "V4l2DevFramedSource.h"

V4l2DevFramedSource::V4l2DevFramedSource(UsageEnvironment& env, const char* dev,
                                        int width, int height, uint32_t pixelFmt) :
    DevFramedSource(env, dev),
    mWidth(width),
    mHeight(height),
    mPixelFmt(pixelFmt)
{
    bool ret;
    ret = videoInit();
    assert(ret == true);
}

V4l2DevFramedSource::~V4l2DevFramedSource()
{
    videoExit();
}

bool V4l2DevFramedSource::createFrame(Frame* frame)
{
    bool ret;
    ret = getFrame(frame);
    if(ret == true)
        return true;

    v4l2_poll(mFd);

    mV4l2BufUnit = v4l2_dqbuf(mFd, mV4l2Buf);

    ret = encode(mV4l2BufUnit, frame);

    v4l2_qbuf(mFd, mV4l2BufUnit);

    return ret;
}

bool V4l2DevFramedSource::videoInit()
{
    int ret;
    char devName[100];
    struct v4l2_capability cap;

    mFd = v4l2_open(mDev.c_str(), O_RDWR);
    if(mFd < 0)
        return false;

    ret = v4l2_querycap(mFd, &cap);
    if(ret < 0)
        return false;

    if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
        return false;
    
    ret = v4l2_enuminput(mFd, 0, devName);
    if(ret < 0)
        return false;

    ret = v4l2_s_input(mFd, 0);
    if(ret < 0)
        return false;
    
    ret = v4l2_enum_fmt(mFd, mPixelFmt, V4L2_BUF_TYPE_VIDEO_CAPTURE);
    if(ret < 0)
        return false;
    
    ret = v4l2_s_fmt(mFd, &mWidth, &mHeight, mPixelFmt, V4L2_BUF_TYPE_VIDEO_CAPTURE);
    if(ret < 0)
        return false;
    
    mV4l2Buf = v4l2_reqbufs(mFd, V4L2_BUF_TYPE_VIDEO_CAPTURE, 4);
    if(!mV4l2Buf)
        return false;
    
    ret = v4l2_querybuf(mFd, mV4l2Buf);
    if(ret < 0)
        return false;
    
    ret = v4l2_mmap(mFd, mV4l2Buf);
    if(ret < 0)
        return false;
    
    ret = v4l2_qbuf_all(mFd, mV4l2Buf);
    if(ret < 0)
        return false;

    ret = v4l2_streamon(mFd);
    if(ret < 0)
        return false;
    
    ret = v4l2_poll(mFd);
    if(ret < 0)
        return false;
    
    return true;
}

bool V4l2DevFramedSource::videoExit()
{
    int ret;

    ret = v4l2_streamoff(mFd);
    if(ret < 0)
        return false;

    ret = v4l2_munmap(mFd, mV4l2Buf);
    if(ret < 0)
        return false;

    ret = v4l2_relbufs(mV4l2Buf);
    if(ret < 0)
        return false;

    v4l2_close(mFd);

    return true;
}