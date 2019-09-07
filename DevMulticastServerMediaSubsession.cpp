#include "DevMulticastServerMediaSubsession.h"

DevMulticastServerMediaSubsession::DevMulticastServerMediaSubsession(UsageEnvironment& env, char const* dev,
                                                    struct in_addr destinationAddress,
                                                    Port rtpPort, Port rtcpPort, int ttl) :
    MulticastServerMediaSubsession(env, destinationAddress, rtpPort, rtcpPort, ttl),
    mDev(dev)
{

}
    
DevMulticastServerMediaSubsession::~DevMulticastServerMediaSubsession()
{

}
