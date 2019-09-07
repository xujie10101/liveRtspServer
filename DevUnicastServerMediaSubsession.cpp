#include "DevUnicastServerMediaSubsession.h"

DevUnicastServerMediaSubsession::DevUnicastServerMediaSubsession(UsageEnvironment& env, char const* dev,
            Boolean reuseFirstSource,
			portNumBits initialPortNum,
			Boolean multiplexRTCPWithRTP) :
    UnicastServerMediaSubsession(env, reuseFirstSource, initialPortNum, multiplexRTCPWithRTP),
    mDev(dev)
{

}

DevUnicastServerMediaSubsession::~DevUnicastServerMediaSubsession()
{

}
