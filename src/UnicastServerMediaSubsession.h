#ifndef _UNICAST_SERVER_MEDIA_SUBSESSION_H_
#define _UNICAST_SERVER_MEDIA_SUBSESSION_H_
#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>

class UnicastServerMediaSubsession : public OnDemandServerMediaSubsession
{
protected:
    UnicastServerMediaSubsession(UsageEnvironment& env, Boolean reuseFirstSource,
    			portNumBits initialPortNum = 6970,
    			Boolean multiplexRTCPWithRTP = False);

    virtual ~UnicastServerMediaSubsession();
};

#endif //_UNICAST_SERVER_MEDIA_SUBSESSION_H_
