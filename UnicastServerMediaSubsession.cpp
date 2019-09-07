#include "UnicastServerMediaSubsession.h"

UnicastServerMediaSubsession::UnicastServerMediaSubsession(UsageEnvironment& env, Boolean reuseFirstSource,
    			portNumBits initialPortNum,
    			Boolean multiplexRTCPWithRTP) :
    OnDemandServerMediaSubsession(env, reuseFirstSource, initialPortNum, multiplexRTCPWithRTP)
{

}

 UnicastServerMediaSubsession::~UnicastServerMediaSubsession()
{

}
