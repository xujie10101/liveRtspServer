#ifndef _MULTICAST_SERVER_MEDIA_SUBSESSION_H_
#define _MULTICAST_SERVER_MEDIA_SUBSESSION_H_
#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>

class MulticastServerMediaSubsession : public ServerMediaSubsession
{
public:
    virtual ~MulticastServerMediaSubsession();

protected:

    MulticastServerMediaSubsession(UsageEnvironment& env, struct in_addr destinationAddress,
                                                Port rtpPort, Port rtcpPort, int ttl);

    virtual Boolean rtcpIsMuxed();
    virtual char const* sdpLines();
    virtual void getStreamParameters(unsigned clientSessionId,
    			   netAddressBits clientAddress,
                                   Port const& clientRTPPort,
                                   Port const& clientRTCPPort,
    			                   int tcpSocketNum,
                                   unsigned char rtpChannelId,
                                   unsigned char rtcpChannelId,
                                   netAddressBits& destinationAddress,
    			                   u_int8_t& destinationTTL,
                                   Boolean& isMulticast,
                                   Port& serverRTPPort,
                                   Port& serverRTCPPort,
                                   void*& streamToken);
    virtual void startStream(unsigned clientSessionId, void* streamToken,
    		                TaskFunc* rtcpRRHandler,
    		                void* rtcpRRHandlerClientData,
                            unsigned short& rtpSeqNum,
                            unsigned& rtpTimestamp,
    		                ServerRequestAlternativeByteHandler* serverRequestAlternativeByteHandler,
                            void* serverRequestAlternativeByteHandlerClientData);
    virtual float getCurrentNPT(void* streamToken);
    virtual void getRTPSinkandRTCP(void* streamToken,
    			 RTPSink const*& rtpSink, RTCPInstance const*& rtcp);
    virtual void deleteStream(unsigned clientSessionId, void*& streamToken);

    virtual FramedSource* createNewStreamSource(        unsigned& estBitrate) = 0;
    virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
                                                    unsigned char rtpPayloadTypeIfDynamic,
                                                        FramedSource* inputSource) = 0;
    void setSDPLinesFromRTPSink();
    virtual char const* getAuxSDPLine(RTPSink*, FramedSource*);

    static void afterPlaying(void* clientData);
    void afterPlaying1();
    
private:
    FramedSource* mFramedSource;
    RTPSink* mRTPSink;
    Groupsock mRtpGroupsock;
    Groupsock mRtcpGroupsock;
    RTCPInstance* mRTCPInstance;
    HashTable* mClientRTCPSourceRecords;
    char* mSDPLines;
};

#endif //_MULTICAST_SERVER_MEDIA_SUBSESSION_H_
