#include "AlsaAACDevUnicastServerMediaSubsession.h"
#include "AlsaAACDevFramedSource.h"

AlsaAACDevUnicastServerMediaSubsession*
AlsaAACDevUnicastServerMediaSubsession::createNew(UsageEnvironment& env, char const* dev, Boolean reuseFirstSource,
                                                    int samplingFreq, int numChannel, const char* fmt)
{
    return new AlsaAACDevUnicastServerMediaSubsession(env, dev, reuseFirstSource, samplingFreq, numChannel, fmt);
}

AlsaAACDevUnicastServerMediaSubsession::AlsaAACDevUnicastServerMediaSubsession(UsageEnvironment& env,
                                                        char const* dev, Boolean reuseFirstSource,
                                                        int samplingFreq, int numChannel, const char* fmt) :
    DevUnicastServerMediaSubsession(env, dev, reuseFirstSource),
    mSamplingFreq(samplingFreq),
    mNumChannel(numChannel),
    mFmt(fmt)
{
    
}

AlsaAACDevUnicastServerMediaSubsession::~AlsaAACDevUnicastServerMediaSubsession()
{

}

FramedSource* AlsaAACDevUnicastServerMediaSubsession::createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate)
{
    estBitrate = 96; // kbps, estimate

    return AlsaAACDevFramedSource::createNew(envir(), mDev.c_str(), mSamplingFreq, mNumChannel, mFmt.c_str());
}

RTPSink* AlsaAACDevUnicastServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic,
                                    FramedSource* inputSource)
{
    AlsaAACDevFramedSource* source = (AlsaAACDevFramedSource*)inputSource;
    return MPEG4GenericRTPSink::createNew(envir(), rtpGroupsock, //负责RTP打包
    				rtpPayloadTypeIfDynamic,
    				source->samplingFrequency(),
    				"audio", "AAC-hbr", source->configStr(),
    				source->numChannels());
}