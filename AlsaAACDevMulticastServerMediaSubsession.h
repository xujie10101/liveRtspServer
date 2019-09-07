#ifndef _ALSA_AAC_DEV_MULTICAST_SERVER_MEDIA_SUBSESSION_HH_
#define _ALSA_AAC_DEV_MULTICAST_SERVER_MEDIA_SUBSESSION_HH_
#include <string>

#include "DevMulticastServerMediaSubsession.h"

class AlsaAACDevMulticastServerMediaSubsession : public DevMulticastServerMediaSubsession
{
public:
    static AlsaAACDevMulticastServerMediaSubsession* createNew(UsageEnvironment& env,
                                                char const* dev, struct in_addr destinationAddress,
                                                Port rtpPort, Port rtcpPort, int ttl = 255,
                                                int samplingFreq=44100, int numChannel=2, const char* fmt="s16le");
    virtual ~AlsaAACDevMulticastServerMediaSubsession();

protected:
    AlsaAACDevMulticastServerMediaSubsession(UsageEnvironment& env,
                                                char const* dev, struct in_addr destinationAddress,
                                                Port rtpPort, Port rtcpPort, int ttl,
                                                int samplingFreq, int numChannel, const char* fmt);

    virtual FramedSource* createNewStreamSource(unsigned& estBitrate);
    virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
                                        unsigned char rtpPayloadTypeIfDynamic,
                                            FramedSource* inputSource);

private:
    int mSamplingFreq;
    int mNumChannel;
    std::string mFmt;
};

#endif //_ALSA_AAC_DEV_MULTICAST_SERVER_MEDIA_SUBSESSION_HH_