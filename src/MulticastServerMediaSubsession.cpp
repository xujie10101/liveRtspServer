#include <assert.h>

#include "MulticastServerMediaSubsession.h"

MulticastServerMediaSubsession::MulticastServerMediaSubsession(UsageEnvironment& env, struct in_addr destinationAddress,
                                                Port rtpPort, Port rtcpPort, int ttl) :
    ServerMediaSubsession(env),
    mFramedSource(NULL),
    mRTPSink(NULL),
    mRtpGroupsock(env, destinationAddress, rtpPort, ttl),
    mRtcpGroupsock(env, destinationAddress, rtcpPort, ttl),
    mRTCPInstance(NULL),
    mClientRTCPSourceRecords(NULL),
    mSDPLines(NULL)
{
    mClientRTCPSourceRecords = HashTable::create(ONE_WORD_HASH_KEYS);
}

class RTCPSourceRecord
{
public:
    RTCPSourceRecord(netAddressBits addr, Port const& port)
        : addr(addr), port(port)
    {

    }

    netAddressBits addr;
    Port port;
};

MulticastServerMediaSubsession::~MulticastServerMediaSubsession()
{
    delete[] mSDPLines;
    
    while(1)
    {
        RTCPSourceRecord* source = (RTCPSourceRecord*)(mClientRTCPSourceRecords->RemoveNext());
        if (source == NULL) break;
        delete source;
    }

    delete mClientRTCPSourceRecords;

    Medium::close(mFramedSource);
    Medium::close(mRTPSink);
}

Boolean MulticastServerMediaSubsession::rtcpIsMuxed()
{
    if (mRTCPInstance == NULL) return False;

    return &(mRTPSink->groupsockBeingUsed()) == mRTCPInstance->RTCPgs();
}

char const* MulticastServerMediaSubsession::sdpLines()
{
    if (mSDPLines == NULL)
    {
        unsigned estimatedSessionBandwidth;
        mFramedSource = createNewStreamSource(estimatedSessionBandwidth);
        assert(mFramedSource != NULL);

        unsigned char rtpPayloadType = 96 + trackNumber()-1; // if dynamic
        mRTPSink = createNewRTPSink(&mRtpGroupsock, rtpPayloadType, mFramedSource);
        assert(mRTPSink != NULL);

        const unsigned maxCNAMElen = 100;
        unsigned char CNAME[maxCNAMElen+1];
        gethostname((char*)CNAME, maxCNAMElen);
        CNAME[maxCNAMElen] = '\0';
        
        mRTCPInstance = RTCPInstance::createNew(envir(), &mRtcpGroupsock,
                      estimatedSessionBandwidth, CNAME,
                      mRTPSink, NULL,
                      True /* 表示多播 */);

        mRTPSink->startPlaying(*mFramedSource, afterPlaying, this);

        setSDPLinesFromRTPSink();
    }

    return mSDPLines;
}

void MulticastServerMediaSubsession::setSDPLinesFromRTPSink()
{
    Groupsock const& gs = mRTPSink->groupsockBeingUsed();
    AddressString groupAddressStr(gs.groupAddress());
    unsigned short portNum = ntohs(gs.port().num());
    unsigned char ttl = gs.ttl();
    unsigned char rtpPayloadType = mRTPSink->rtpPayloadType();
    char const* mediaType = mRTPSink->sdpMediaType();
    unsigned estBitrate = mRTCPInstance == NULL ? 50 : mRTCPInstance->totSessionBW();
    char* rtpmapLine = mRTPSink->rtpmapLine();
    char const* rtcpmuxLine = rtcpIsMuxed() ? "a=rtcp-mux\r\n" : "";
    char const* rangeLine = rangeSDPLine();
    char const* auxSDPLine = getAuxSDPLine(mRTPSink, mFramedSource);
    if (auxSDPLine == NULL) auxSDPLine = "";
    
    char const* const sdpFmt =  "m=%s %d RTP/AVP %d\r\n"
                                "c=IN IP4 %s/%d\r\n"
                                "b=AS:%u\r\n"
                                "%s"
                                "%s"
                                "%s"
                                "%s"
                                "a=control:%s\r\n";
    
    unsigned sdpFmtSize = strlen(sdpFmt)
                        + strlen(mediaType) + 5 /* max short len */ + 3 /* max char len */
                        + strlen(groupAddressStr.val()) + 3 /* max char len */
                        + 20 /* max int len */
                        + strlen(rtpmapLine)
                        + strlen(rtcpmuxLine)
                        + strlen(rangeLine)
                        + strlen(auxSDPLine)
                        + strlen(trackId());
    char* sdpLines = new char[sdpFmtSize];
    sprintf(sdpLines, sdpFmt,
                    mediaType, // m= <media>
                    portNum, // m= <port>
                    rtpPayloadType, // m= <fmt list>
                    groupAddressStr.val(), // c= <connection address>
                    ttl, // c= TTL
                    estBitrate, // b=AS:<bandwidth>
                    rtpmapLine, // a=rtpmap:... (if present)
                    rtcpmuxLine, // a=rtcp-mux:... (if present)
                    rangeLine, // a=range:... (if present)
                    auxSDPLine, // optional extra SDP line
                    trackId()); // a=control:<track-id>
    delete[] (char*)rangeLine;
    delete[] rtpmapLine;
    
    mSDPLines = strDup(sdpLines);
    delete[] sdpLines;
}


void MulticastServerMediaSubsession::getStreamParameters(unsigned clientSessionId,
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
                               void*& streamToken)
{
    isMulticast = True;
    Groupsock& gs = mRTPSink->groupsockBeingUsed();
    if (destinationTTL == 255)
        destinationTTL = gs.ttl();
    
    if (destinationAddress == 0) // normal case
    {
        destinationAddress = gs.groupAddress().s_addr;
    }
    else  // use the client-specified destination address instead:
    {
        struct in_addr destinationAddr; destinationAddr.s_addr = destinationAddress;
        gs.changeDestinationParameters(destinationAddr, 0, destinationTTL);
        if (mRTCPInstance != NULL)
        {
            Groupsock* rtcpGS = mRTCPInstance->RTCPgs();
            rtcpGS->changeDestinationParameters(destinationAddr, 0, destinationTTL);
        }
    }
    
    serverRTPPort = gs.port();
    if (mRTCPInstance != NULL)
    {
        Groupsock* rtcpGS = mRTCPInstance->RTCPgs();
        serverRTCPPort = rtcpGS->port();
    }
    streamToken = NULL; // not used

    RTCPSourceRecord* source = new RTCPSourceRecord(clientAddress, clientRTCPPort);
    mClientRTCPSourceRecords->Add((char const*)clientSessionId, source);
}

void MulticastServerMediaSubsession::startStream(unsigned clientSessionId, void* streamToken,
		                TaskFunc* rtcpRRHandler,
		                void* rtcpRRHandlerClientData,
                        unsigned short& rtpSeqNum,
                        unsigned& rtpTimestamp,
		                ServerRequestAlternativeByteHandler* serverRequestAlternativeByteHandler,
                        void* serverRequestAlternativeByteHandlerClientData)
{
    rtpSeqNum = mRTPSink->currentSeqNo();
    rtpTimestamp = mRTPSink->presetNextTimestamp();

    unsigned streamBitrate = mRTCPInstance == NULL ? 50 : mRTCPInstance->totSessionBW(); // in kbps
    unsigned rtpBufSize = streamBitrate * 25 / 2; // 1 kbps * 0.1 s = 12.5 bytes
    if (rtpBufSize < 50 * 1024)
        rtpBufSize = 50 * 1024;
    increaseSendBufferTo(envir(), mRTPSink->groupsockBeingUsed().socketNum(), rtpBufSize);

    if (mRTCPInstance != NULL)
    {
        // Hack: Send a RTCP "SR" packet now, so that receivers will (likely) be able to
        // get RTCP-synchronized presentation times immediately:
        mRTCPInstance->sendReport();

        // Set up the handler for incoming RTCP "RR" packets from this client:
        RTCPSourceRecord* source = (RTCPSourceRecord*)(mClientRTCPSourceRecords->Lookup((char const*)clientSessionId));
        if (source != NULL)
        {
            mRTCPInstance->setSpecificRRHandler(source->addr, source->port,
                                                    rtcpRRHandler, rtcpRRHandlerClientData);
        }
    }
}

char const* MulticastServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink, FramedSource* /*inputSource*/)
{
    return rtpSink == NULL ? NULL : rtpSink->auxSDPLine();
}

float MulticastServerMediaSubsession::getCurrentNPT(void* streamToken)
{
    // Return the elapsed time between our "RTPSink"s creation time, and the current time:
    struct timeval const& creationTime  = mRTPSink->creationTime(); // alias

    struct timeval timeNow;
    gettimeofday(&timeNow, NULL);

    return (float)(timeNow.tv_sec - creationTime.tv_sec + (timeNow.tv_usec - creationTime.tv_usec)/1000000.0);
}

void MulticastServerMediaSubsession::getRTPSinkandRTCP(void* streamToken,
			 RTPSink const*& rtpSink, RTCPInstance const*& rtcp)
{
    rtpSink = mRTPSink;
    rtcp = mRTCPInstance;
}
             
void MulticastServerMediaSubsession::deleteStream(unsigned clientSessionId, void*& streamToken)
{
    // Lookup and remove the 'RTCPSourceRecord' for this client.  Also turn off RTCP "RR" handling:
    RTCPSourceRecord* source = (RTCPSourceRecord*)(mClientRTCPSourceRecords->Lookup((char const*)clientSessionId));
    if (source != NULL)
    {
        if (mRTCPInstance != NULL)
        {
            mRTCPInstance->unsetSpecificRRHandler(source->addr, source->port);
        }

        mClientRTCPSourceRecords->Remove((char const*)clientSessionId);
        delete source;
    }
}

void MulticastServerMediaSubsession::afterPlaying(void* clientData)
{
    MulticastServerMediaSubsession* subsession = (MulticastServerMediaSubsession*)clientData;
    subsession->afterPlaying1();
}

void MulticastServerMediaSubsession::afterPlaying1()
{
    Medium::close(mFramedSource);

    unsigned int estBitrate;
    mFramedSource = createNewStreamSource(estBitrate);

    mRTPSink->startPlaying(*mFramedSource, afterPlaying, this);
}
