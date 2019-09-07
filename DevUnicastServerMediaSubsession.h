#ifndef _DEV_UNICAST_SERVER_MEDIA_SUBSESSION_H_
#define _DEV_UNICAST_SERVER_MEDIA_SUBSESSION_H_
#include <string>

#include "UnicastServerMediaSubsession.h"

class DevUnicastServerMediaSubsession : public UnicastServerMediaSubsession
{
protected:
    DevUnicastServerMediaSubsession(UsageEnvironment& env, char const* dev,
                Boolean reuseFirstSource,
    			portNumBits initialPortNum = 6970,
    			Boolean multiplexRTCPWithRTP = False);

    virtual ~DevUnicastServerMediaSubsession();

protected:
    std::string mDev;    
};

#endif //_DEV_UNICAST_SERVER_MEDIA_SUBSESSION_H_
