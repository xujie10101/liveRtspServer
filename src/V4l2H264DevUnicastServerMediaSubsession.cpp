#include "V4l2H264DevUnicastServerMediaSubsession.h"
#include "V4l2H264DevFramedSource.h"

V4l2H264DevUnicastServerMediaSubsession* V4l2H264DevUnicastServerMediaSubsession::createNew(UsageEnvironment& env, char const* dev,
                                Boolean reuseFirstSource,
                                int width,
                                int height,
                                int fps,
                                portNumBits initialPortNum,
                                Boolean multiplexRTCPWithRTP)
{
    return new V4l2H264DevUnicastServerMediaSubsession(env, dev, reuseFirstSource, width, height, fps,
                                                        initialPortNum, multiplexRTCPWithRTP);
}

V4l2H264DevUnicastServerMediaSubsession::V4l2H264DevUnicastServerMediaSubsession(UsageEnvironment& env, char const* dev,
                                            Boolean reuseFirstSource,
                                            int width,
                                            int height,
                                            int fps,
                                            portNumBits initialPortNum,
                                            Boolean multiplexRTCPWithRTP) :
    DevUnicastServerMediaSubsession(env, dev, reuseFirstSource, initialPortNum, multiplexRTCPWithRTP),
    mAuxSDPLine(NULL), mDoneFlag(0), mDummyRTPSink(NULL),
    mWidth(width), mHeight(height), mFps(fps)
{
    
}

V4l2H264DevUnicastServerMediaSubsession::~V4l2H264DevUnicastServerMediaSubsession()
{
    delete[] mAuxSDPLine;
}

static void afterPlayingDummy(void* clientData)
{
    V4l2H264DevUnicastServerMediaSubsession* subsess = (V4l2H264DevUnicastServerMediaSubsession*)clientData;
    subsess->afterPlayingDummy1();
}

void V4l2H264DevUnicastServerMediaSubsession::afterPlayingDummy1()
{
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
    setDoneFlag();
}

static void checkForAuxSDPLine(void* clientData)
{
    V4l2H264DevUnicastServerMediaSubsession* subsess = (V4l2H264DevUnicastServerMediaSubsession*)clientData;
    subsess->checkForAuxSDPLine1();
}

void V4l2H264DevUnicastServerMediaSubsession::checkForAuxSDPLine1()
{
    nextTask() = NULL;

    char const* dasl;
    if(mAuxSDPLine != NULL)
    {
        setDoneFlag();
    }
    else if(mDummyRTPSink != NULL && (dasl = mDummyRTPSink->auxSDPLine()) != NULL)
    {
        mAuxSDPLine = strDup(dasl);
        mDummyRTPSink = NULL;

        setDoneFlag();
    }
    else if(!mDoneFlag)
    {
        int uSecsToDelay = 100000; // 100 ms
        nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecsToDelay,
                                                        (TaskFunc*)checkForAuxSDPLine, this);
    }
}

char const* V4l2H264DevUnicastServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource)
{
    if(mAuxSDPLine != NULL) return mAuxSDPLine;

    if(mDummyRTPSink == NULL)
        mDummyRTPSink = rtpSink;

    mDummyRTPSink->startPlaying(*inputSource, afterPlayingDummy, this); // 获取sdp

    checkForAuxSDPLine(this);

    envir().taskScheduler().doEventLoop(&mDoneFlag); // 程序在此等待

    return mAuxSDPLine;
}

FramedSource* V4l2H264DevUnicastServerMediaSubsession::createNewStreamSource(unsigned clientSessionId,
                    unsigned& estBitrate)
{
    V4l2H264DevFramedSource* source = V4l2H264DevFramedSource::createNew(envir(), mDev.c_str(),
                                                                        mWidth, mHeight, mFps);

    return H264VideoStreamDiscreteFramer::createNew(envir(), source);
}

RTPSink* V4l2H264DevUnicastServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock,
                            unsigned char rtpPayloadTypeIfDynamic,
            FramedSource* inputSource)
{
    return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}

