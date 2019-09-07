#ifndef _DEV_MULTICAST_SERVER_MEDIA_SUBSESSION_H_
#define _DEV_MULTICAST_SERVER_MEDIA_SUBSESSION_H_
#include <string>

#include "MulticastServerMediaSubsession.h"

class DevMulticastServerMediaSubsession : public MulticastServerMediaSubsession
{
protected:
    DevMulticastServerMediaSubsession(UsageEnvironment& env, char const* dev,
                                                    struct in_addr destinationAddress,
                                                    Port rtpPort, Port rtcpPort, int ttl);
    
    virtual ~DevMulticastServerMediaSubsession();

protected:
    std::string mDev;
};

#endif //_DEV_MULTICAST_SERVER_MEDIA_SUBSESSION_H_