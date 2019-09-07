#ifndef _V4L2_H264_DEV_UNICAST_SERVER_MEDIA_SUBSESSION_H_
#define _V4L2_H264_DEV_UNICAST_SERVER_MEDIA_SUBSESSION_H_
#include <string>

#include "DevUnicastServerMediaSubsession.h"

class V4l2H264DevUnicastServerMediaSubsession : public DevUnicastServerMediaSubsession
{
public:
    static V4l2H264DevUnicastServerMediaSubsession* createNew(UsageEnvironment& env, char const* dev,
                                Boolean reuseFirstSource,
                                int width=320,
                                int height=240,
                                int fps=15,
                                portNumBits initialPortNum = 6970,
                                Boolean multiplexRTCPWithRTP = False);

    void checkForAuxSDPLine1();
    void afterPlayingDummy1();

protected:
    V4l2H264DevUnicastServerMediaSubsession(UsageEnvironment& env, char const* dev,
                Boolean reuseFirstSource,
                int width,
                int height,
                int fps,
    			portNumBits initialPortNum = 6970,
    			Boolean multiplexRTCPWithRTP = False);

    virtual ~V4l2H264DevUnicastServerMediaSubsession();

    void setDoneFlag() { mDoneFlag = ~0; }

    virtual char const* getAuxSDPLine(RTPSink* rtpSink,
    		        FramedSource* inputSource);
    virtual FramedSource* createNewStreamSource(unsigned clientSessionId,
    			      unsigned& estBitrate);
    virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
                    unsigned char rtpPayloadTypeIfDynamic,
    		        FramedSource* inputSource);

private:
    char* mAuxSDPLine;
    char mDoneFlag;
    RTPSink* mDummyRTPSink;

    int mWidth;
    int mHeight;
    int mFps;
};

#endif //_V4L2_H264_DEV_UNICAST_SERVER_MEDIA_SUBSESSION_H_
