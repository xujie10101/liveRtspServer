#ifndef _ALSA_AAC_DEV_FRAMED_SOURCE_H_
#define _ALSA_AAC_DEV_FRAMED_SOURCE_H_

#include "AlsaDevFramedSource.h"
#include <faac.h>

class AlsaAACDevFramedSource : public AlsaDevFramedSource
{
public:
    static AlsaAACDevFramedSource* createNew(UsageEnvironment& env, const char* dev,
                                            int samplingFre=44100, int numChannel=2, const char* fmt="s16le");
    
    char const* configStr() const { return mConfigStr; }

protected:
    AlsaAACDevFramedSource(UsageEnvironment& env, const char* dev,
                            int samplingFre, int numChannel, const char* fmt);
    virtual ~AlsaAACDevFramedSource();

    virtual bool encode(uint8_t* pcmFrame, int samplingNum, Frame* frame);

private:
    bool faacInit();
    void faacExit();

private:
    char mConfigStr[5];
    unsigned int mAACObjectType;

    faacEncHandle mFaacEncHandle;
    uint32_t mMaxOutputBytes;
};

#endif //_ALSA_AAC_DEV_FRAMED_SOURCE_H_