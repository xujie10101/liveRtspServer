#include "AlsaAACDevMulticastServerMediaSubsession.h"
#include "AlsaAACDevFramedSource.h"

AlsaAACDevMulticastServerMediaSubsession* AlsaAACDevMulticastServerMediaSubsession::createNew(UsageEnvironment& env,
                                            char const* dev, struct in_addr destinationAddress,
                                            Port rtpPort, Port rtcpPort, int ttl,
                                            int samplingFreq, int numChannel, const char* fmt)
{
    return new AlsaAACDevMulticastServerMediaSubsession(env, dev, destinationAddress, rtpPort, rtcpPort, ttl,
                                                        samplingFreq, numChannel, fmt);
}

AlsaAACDevMulticastServerMediaSubsession::AlsaAACDevMulticastServerMediaSubsession(UsageEnvironment& env,
                                            char const* dev, struct in_addr destinationAddress,
                                            Port rtpPort, Port rtcpPort, int ttl,
                                            int samplingFreq, int numChannel, const char* fmt) :
    DevMulticastServerMediaSubsession(env, dev, destinationAddress, rtpPort, rtcpPort, ttl),
    mSamplingFreq(samplingFreq), mNumChannel(numChannel), mFmt(fmt)
{
    
}

AlsaAACDevMulticastServerMediaSubsession::~AlsaAACDevMulticastServerMediaSubsession()
{

}

FramedSource* AlsaAACDevMulticastServerMediaSubsession::createNewStreamSource(unsigned& estBitrate)
{
    return AlsaAACDevFramedSource::createNew(envir(), mDev.c_str(), mSamplingFreq, mNumChannel, mFmt.c_str());
}

RTPSink* AlsaAACDevMulticastServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock,
                                        unsigned char rtpPayloadTypeIfDynamic,
                                        FramedSource* inputSource)
{
    AlsaAACDevFramedSource* source = (AlsaAACDevFramedSource*)inputSource;
    return MPEG4GenericRTPSink::createNew(envir(), rtpGroupsock, //负责RTP打包
    				rtpPayloadTypeIfDynamic,
    				source->samplingFrequency(),
    				"audio", "AAC-hbr", source->configStr(),
    				source->numChannels());
}