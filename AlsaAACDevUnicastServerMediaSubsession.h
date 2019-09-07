#ifndef _ALSA_AAC_DEV_UNICAST_SERVER_MEDIA_SUBSESSION_HH
#define _ALSA_AAC_DEV_UNICAST_SERVER_MEDIA_SUBSESSION_HH
#include <string>

#include "DevUnicastServerMediaSubsession.h"

class AlsaAACDevUnicastServerMediaSubsession: public DevUnicastServerMediaSubsession
{
public:
    static AlsaAACDevUnicastServerMediaSubsession*
        createNew(UsageEnvironment& env, char const* dev, Boolean reuseFirstSource,
                    int samplingFreq=44100, int numChannel=2, const char* fmt="s16le");

protected:
    AlsaAACDevUnicastServerMediaSubsession(UsageEnvironment& env,
                                            char const* dev, Boolean reuseFirstSource,
                                            int samplingFreq, int numChannel, const char* fmt);
    // called only by createNew();
    virtual ~AlsaAACDevUnicastServerMediaSubsession();

protected: // redefined virtual functions
    virtual FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate);
    virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic,
                                        FramedSource* inputSource);

private:
    int mSamplingFreq;
    int mNumChannel;
    std::string mFmt;
};

#endif //_ALSA_AAC_DEV_UNICAST_SERVER_MEDIA_SUBSESSION_HH
