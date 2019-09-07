#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "AlsaDevFramedSource.h"

AlsaDevFramedSource::AlsaDevFramedSource(UsageEnvironment& env, const char* dev, int samplingNum,
                                            int samplingFre, int numChannel, const char* fmt) :
    DevFramedSource(env, dev, 4096),
    mSamplingFrequency(samplingFre),
    mNumChannels(numChannel),
    mSamplingNum(samplingNum)
{
    if(!strcmp(fmt, "s16le"))
        mFmt = SND_PCM_FORMAT_S16_LE;
    else if(!strcmp(fmt, "s16be"))
        mFmt = SND_PCM_FORMAT_S16_BE;
    else if(!strcmp(fmt, "u16le"))
        mFmt = SND_PCM_FORMAT_U16_BE;
    else if(!strcmp(fmt, "u16be"))
        mFmt = SND_PCM_FORMAT_U16_BE;
    else if(!strcmp(fmt, "s32le"))
        mFmt = SND_PCM_FORMAT_S32_LE;
    else if(!strcmp(fmt, "s32be"))
        mFmt = SND_PCM_FORMAT_S32_BE;
    else if(!strcmp(fmt, "u32le"))
        mFmt = SND_PCM_FORMAT_U32_LE;
    else if(!strcmp(fmt, "u32be"))
        mFmt = SND_PCM_FORMAT_U32_BE;
    else
    {
        envir()<<"can't support this audio fmt\n";
        mFmt = SND_PCM_FORMAT_S16_LE;
    }

    assert(alsaInit() == true);
}

AlsaDevFramedSource::~AlsaDevFramedSource()
{
    alsaExit();
}

bool AlsaDevFramedSource::createFrame(Frame* frame)
{
    int ret;

    /* mSamplingNum 一帧采样点数 */
    ret = snd_pcm_readi(mPcmHandle, mPcmBuffer, mSamplingNum);
    if(ret == -EPIPE)
    {
        /* EPIPE means overrun */
        envir()<<"overrun occurred\n";
        snd_pcm_prepare(mPcmHandle);
    }
    else if(ret < 0)
    {
        envir()<<"error from read: %s\n"<<snd_strerror(ret);
    }
    else if(ret != (int)mSamplingNum)
    {
        envir()<<"short read\n";
    }

    return encode(mPcmBuffer, mSamplingNum*mNumChannels, frame);
}

bool AlsaDevFramedSource::alsaInit()
{
    int ret;
    uint32_t val;
    int dir;
    snd_pcm_uframes_t frames;

	/* 打开设备的捕获功能 */
	ret = snd_pcm_open(&mPcmHandle, mDev.c_str(), SND_PCM_STREAM_CAPTURE, 0);
    if(ret < 0)
    {
        envir()<<"failed to open snd dev\n";
        return false;
    }

    /* 分配参数设置 */
    snd_pcm_hw_params_alloca(&mPcmParams);
    
    /* 获取默认的参数设置 */
    snd_pcm_hw_params_any(mPcmHandle, mPcmParams);

    /* 设置采样为交互模式 */
    snd_pcm_hw_params_set_access(mPcmHandle, mPcmParams, SND_PCM_ACCESS_RW_INTERLEAVED);

    /* 设置采样为：有符号，16为，小端 */
	//snd_pcm_hw_params_set_format(mPcmHandle, mPcmParams, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_format(mPcmHandle, mPcmParams, mFmt);

    /* 设置通道数 */
    snd_pcm_hw_params_set_channels(mPcmHandle, mPcmParams, mNumChannels);

    /* 设置采样频率 */
    val = mSamplingFrequency;
    snd_pcm_hw_params_set_rate_near(mPcmHandle, mPcmParams, &val, &dir);

    /* 设置多少次采样为一帧 */
	frames = mSamplingNum;
	snd_pcm_hw_params_set_period_size_near(mPcmHandle, mPcmParams, &frames, &dir);

	ret = snd_pcm_hw_params(mPcmHandle, mPcmParams);
	if (ret < 0)
    {
		envir()<<"unable to set hw parameters: %s\n"<<snd_strerror(ret);
		return false;
	}

	snd_pcm_hw_params_get_period_size(mPcmParams, &frames, &dir);
    
    //mFrames = frames;
    mSamplingNum = mSamplingNum;
    mPcmBufferSize = mSamplingNum*mNumChannels*2;

    mPcmBuffer = new uint8_t[mPcmBufferSize];

	snd_pcm_hw_params_get_period_time(mPcmParams, &val, &dir);

    return true;
}

bool AlsaDevFramedSource::alsaExit()
{
	snd_pcm_drain(mPcmHandle);
	snd_pcm_close(mPcmHandle);
    delete[] mPcmBuffer;
}

AlsaDevFramedSource::SampleFmt AlsaDevFramedSource::getSampleFmt()
{
    switch (mFmt)
    {
    case SND_PCM_FORMAT_S16_LE:
    case SND_PCM_FORMAT_S16_BE:
    case SND_PCM_FORMAT_U16_LE:
    case SND_PCM_FORMAT_U16_BE:
        return SAMPLE_FMT_16BIT;
    case SND_PCM_FORMAT_S24_LE:
    case SND_PCM_FORMAT_S24_BE:
    case SND_PCM_FORMAT_U24_LE:
    case SND_PCM_FORMAT_U24_BE:
        return SAMPLE_FMT_24BIT;
    case SND_PCM_FORMAT_S32_LE:
    case SND_PCM_FORMAT_S32_BE:
    case SND_PCM_FORMAT_U32_LE:
    case SND_PCM_FORMAT_U32_BE:
        return SAMPLE_FMT_32BIT;
    case SND_PCM_FORMAT_FLOAT_LE:
    case SND_PCM_FORMAT_FLOAT_BE:
        return SAMPLE_FMT_FLOAT;
    default:
        return SAMPLE_FMT_NULL;
    }
}