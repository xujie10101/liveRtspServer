#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "AlsaAACDevFramedSource.h"

static unsigned const samplingFrequencyTable[16] = {
    96000, 88200, 64000, 48000,
    44100, 32000, 24000, 22050,
    16000, 12000, 11025, 8000,
    7350, 0, 0, 0
};

AlsaAACDevFramedSource* AlsaAACDevFramedSource::createNew(UsageEnvironment& env, const char* dev,
                                                        int samplingFre, int numChannel, const char* fmt)
{
    return new AlsaAACDevFramedSource(env, dev, samplingFre, numChannel, fmt);
}

AlsaAACDevFramedSource::AlsaAACDevFramedSource(UsageEnvironment& env, const char* dev,
                                                int samplingFre, int numChannel, const char* fmt) :
    AlsaDevFramedSource(env, dev, 1024, samplingFre, numChannel, fmt),
    mAACObjectType(LOW)
{
    int index = 0;

    while(samplingFrequencyTable[index])
    {
        if(samplingFrequencyTable[index] == samplingFrequency())
            break;
        index++;
    }

    uint8_t audioSpecificConfig[2];
    uint8_t const audioObjectType = mAACObjectType;
    audioSpecificConfig[0] = (audioObjectType<<3) | (index>>1);
    audioSpecificConfig[1] = (index<<7) | (numChannels()<<3);
    sprintf(mConfigStr, "%02X%02x", audioSpecificConfig[0], audioSpecificConfig[1]);

    assert(faacInit() == true);
}

AlsaAACDevFramedSource::~AlsaAACDevFramedSource()
{
    faacExit();
}

bool AlsaAACDevFramedSource::encode(uint8_t* pcmFrame, int samplingNum, Frame* frame)
{
    int ret;

    ret = faacEncEncode(mFaacEncHandle, (int*)pcmFrame, samplingNum,
                                    frame->mFrame, mMaxOutputBytes);
    if(ret <= 0)
        return false;

    timeval time;
    gettimeofday(&time, NULL);
    frame->mFrameSize = ret;
    frame->mTime = time;
    frame->mDurationInMicroseconds = 1000000 / samplingFrequency() * samplingNumPeriod();

    return true;
}

bool AlsaAACDevFramedSource::faacInit()
{
	long unsigned int nInputSamples   = 0;
	long unsigned int nMaxOutputBytes = 0;

    mFaacEncHandle = faacEncOpen(samplingFrequency(), numChannels(), &nInputSamples, &nMaxOutputBytes);

    mMaxOutputBytes = nMaxOutputBytes;

    faacEncConfigurationPtr configuration;
    configuration = faacEncGetCurrentConfiguration(mFaacEncHandle);

    switch (getSampleFmt())
    {
    case SAMPLE_FMT_16BIT:
        configuration->inputFormat = FAAC_INPUT_16BIT;
        break;
    
    case SAMPLE_FMT_24BIT:
        configuration->inputFormat = FAAC_INPUT_24BIT;
        break;

    case SAMPLE_FMT_32BIT:
        configuration->inputFormat = FAAC_INPUT_32BIT;
        break;
    
    case SAMPLE_FMT_FLOAT:
        configuration->inputFormat = FAAC_INPUT_FLOAT;
        break;

    default:
        configuration->inputFormat = FAAC_INPUT_NULL;
        break;
    }
    configuration->aacObjectType = mAACObjectType;

    configuration->allowMidside = 1;
	configuration->useLfe = 0;
	configuration->useTns = 0;

	//configuration->outputFormat = 1; //加上adts
	configuration->outputFormat = 0;
    
    faacEncSetConfiguration(mFaacEncHandle, configuration);

    return true;
}

void AlsaAACDevFramedSource::faacExit()
{
    faacEncClose(mFaacEncHandle);
}