#ifndef _V4L2_H264_DEV_MULTICAST_SERVER_MEDIA_SUBSESSION_HH_
#define _V4L2_H264_DEV_MULTICAST_SERVER_MEDIA_SUBSESSION_HH_

#include "DevMulticastServerMediaSubsession.h"

class V4l2H264DevMulticastServerMediaSubsession : public DevMulticastServerMediaSubsession
{
public:
    static V4l2H264DevMulticastServerMediaSubsession* createNew(UsageEnvironment& env,
                                                char const* dev, struct in_addr destinationAddress,
                                                Port rtpPort, Port rtcpPort, int ttl = 255,
                                                int width=320, int height=240, int fps=15);
    virtual ~V4l2H264DevMulticastServerMediaSubsession();

protected:
    V4l2H264DevMulticastServerMediaSubsession(UsageEnvironment& env,
                                                char const* dev, struct in_addr destinationAddress,
                                                Port rtpPort, Port rtcpPort, int ttl,
                                                int width, int height, int fps);

    virtual char const* getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource);
    virtual FramedSource* createNewStreamSource(unsigned& estBitrate);
    virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
                                        unsigned char rtpPayloadTypeIfDynamic,
                                            FramedSource* inputSource);
    static void checkForAuxSDPLine(void* clientData);
    void checkForAuxSDPLine1();
    void setDoneFlag() { mDoneFlag = ~0; }
    
private:
    char* mAuxSDPLine;
    char mDoneFlag;
    RTPSink* mDummyRTPSink;

    int mWidth;
    int mHeight;
    int mFps;
};

#endif //_V4L2_H264_DEV_MULTICAST_SERVER_MEDIA_SUBSESSION_HH_