#include "V4l2H264DevMulticastServerMediaSubsession.h"
#include "V4l2H264DevFramedSource.h"

V4l2H264DevMulticastServerMediaSubsession* V4l2H264DevMulticastServerMediaSubsession::createNew(UsageEnvironment& env,
                                            char const* dev, struct in_addr destinationAddress,
                                            Port rtpPort, Port rtcpPort, int ttl,
                                            int width, int height, int fps)
{
    return new V4l2H264DevMulticastServerMediaSubsession(env, dev, destinationAddress, rtpPort, rtcpPort, ttl,
                                                        width, height, fps);
}

V4l2H264DevMulticastServerMediaSubsession::V4l2H264DevMulticastServerMediaSubsession(UsageEnvironment& env,
                                            char const* dev, struct in_addr destinationAddress,
                                            Port rtpPort, Port rtcpPort, int ttl,
                                            int width, int height, int fps) :
    DevMulticastServerMediaSubsession(env, dev, destinationAddress, rtpPort, rtcpPort, ttl),
    mAuxSDPLine(NULL),
    mDoneFlag(0),
    mDummyRTPSink(NULL),
    mWidth(width),
    mHeight(height),
    mFps(fps)
{
    
}

V4l2H264DevMulticastServerMediaSubsession::~V4l2H264DevMulticastServerMediaSubsession()
{
    delete[] mAuxSDPLine;
}

char const* V4l2H264DevMulticastServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource)
{
    if (mAuxSDPLine != NULL)
        return mAuxSDPLine;

    if (mDummyRTPSink == NULL)
    {
        mDummyRTPSink = rtpSink;

        checkForAuxSDPLine(this);
    }

    envir().taskScheduler().doEventLoop(&mDoneFlag); // 程序在此等待

    return mAuxSDPLine;
}

FramedSource* V4l2H264DevMulticastServerMediaSubsession::createNewStreamSource(unsigned& estBitrate)
{
    V4l2H264DevFramedSource* source = V4l2H264DevFramedSource::createNew(envir(), mDev.c_str(), mWidth, mHeight, mFps);

    return H264VideoStreamDiscreteFramer::createNew(envir(), source);
}

RTPSink* V4l2H264DevMulticastServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock,
                            unsigned char rtpPayloadTypeIfDynamic,
                                    FramedSource* inputSource)
{
    return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}

void V4l2H264DevMulticastServerMediaSubsession::checkForAuxSDPLine(void* clientData)
{
    V4l2H264DevMulticastServerMediaSubsession* subsess = (V4l2H264DevMulticastServerMediaSubsession*)clientData;
    subsess->checkForAuxSDPLine1();
}

void V4l2H264DevMulticastServerMediaSubsession::checkForAuxSDPLine1()
{
    nextTask() = NULL;

    char const* dasl;
    if (mAuxSDPLine != NULL)
    {
        setDoneFlag();
    }
    else if (mDummyRTPSink != NULL && (dasl = mDummyRTPSink->auxSDPLine()) != NULL)
    {
        mAuxSDPLine = strDup(dasl);
        mDummyRTPSink = NULL;
        setDoneFlag();
    }
    else if (!mDoneFlag)
    {
        int uSecsToDelay = 100000; // 100 ms
        nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecsToDelay,
                                                (TaskFunc*)checkForAuxSDPLine, this);
    }
}
