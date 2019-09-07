#ifndef _ALSA_DEV_FRAMED_SOURCE_H_
#define _ALSA_DEV_FRAMED_SOURCE_H_

#include "DevFramedSource.h"
#include <alsa/asoundlib.h>
#include <faac.h>

class AlsaDevFramedSource : public DevFramedSource
{
public:
    enum SampleFmt
    {
        SAMPLE_FMT_NULL,
        SAMPLE_FMT_16BIT,
        SAMPLE_FMT_24BIT,
        SAMPLE_FMT_32BIT,
        SAMPLE_FMT_FLOAT,
    };

    uint32_t samplingFrequency() const { return mSamplingFrequency; }
    int numChannels() const { return mNumChannels; }
    int samplingNumPeriod() const { return mSamplingNum; }
    SampleFmt getSampleFmt();

protected:
    AlsaDevFramedSource(UsageEnvironment& env, const char* dev, int mSamplingNum=1024,
                        int samplingFre=44100, int numChannel=2, const char* fmt="s16le");
    virtual ~AlsaDevFramedSource();

    virtual bool createFrame(Frame* frame);

    virtual bool encode(uint8_t* pcmFrame, int samplingNum, Frame* frame) = 0;

private:
    bool alsaInit();
    bool alsaExit();

private:
    uint32_t mSamplingFrequency;
    int mNumChannels;
    snd_pcm_t *mPcmHandle;
	snd_pcm_hw_params_t *mPcmParams;
    int mSamplingNum; //一帧音频采样数
    int mPcmBufferSize;
    uint8_t* mPcmBuffer;
    snd_pcm_format_t mFmt;
};

#endif //_ALSA_DEV_FRAMED_SOURCE_H_