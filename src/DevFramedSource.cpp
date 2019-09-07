#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "DevFramedSource.h"

DevFramedSource::DevFramedSource(UsageEnvironment& env, const char* dev, int frameSize) :
    FramedSource(env),
    mDev(dev),
    mFps(20)
{
    for(int i = 0; i < sizeof(mFrameArray)/sizeof(mFrameArray[0]); ++i)
    {
        mFrameArray[i] = new Frame(frameSize);
        mInputQueue.push(mFrameArray[i]);
    }

    pthread_mutex_init(&mMutex, NULL);
    pthread_cond_init(&mCond, NULL);

    mTaskToken = envir().taskScheduler().scheduleDelayedTask(0, startCreateFrame, this);
}

DevFramedSource::~DevFramedSource()
{
    envir().taskScheduler().unscheduleDelayedTask(mTaskToken);
    envir().taskScheduler().unscheduleDelayedTask(mNextTaskToken);

	pthread_mutex_destroy(&mMutex);
	pthread_cond_destroy(&mCond);

    for(int i = 0; i < sizeof(mFrameArray)/sizeof(mFrameArray[0]); ++i)
        delete mFrameArray[i];
}

void DevFramedSource::startCreateFrame(void* data)
{
    DevFramedSource* source = (DevFramedSource*)data;
    source->startCreateFrame();
}

void DevFramedSource::startCreateFrame()
{
    mThreadRun = true;
    pthread_create(&mThreadId, NULL, threadFunc, this);
}

void DevFramedSource::afterGetNextFrame(void* data)
{
    DevFramedSource* source = (DevFramedSource*)data;
    source->doGetNextFrame();
}

void DevFramedSource::doGetNextFrame()
{
    pthread_mutex_lock(&mMutex);

    if(mOutputQueue.empty())
    {
        pthread_mutex_unlock(&mMutex);

        mNextTaskToken = envir().taskScheduler().scheduleDelayedTask(10*1000, afterGetNextFrame, this);
        return;
    }

    mNextTaskToken = 0;
    Frame* frame = mOutputQueue.front();
    mOutputQueue.pop();

    pthread_mutex_unlock(&mMutex);

    if(frame->mFrameSize > fMaxSize)
    {
        fFrameSize = fMaxSize;
        fNumTruncatedBytes = frame->mFrameSize - fMaxSize;
    }
    else
    {
        fFrameSize = frame->mFrameSize;
        fNumTruncatedBytes = 0;
    }

    fPresentationTime = frame->mTime;
    fDurationInMicroseconds = frame->mDurationInMicroseconds;

    memcpy(fTo, frame->mFrame, fFrameSize);

    pthread_mutex_lock(&mMutex);
    mInputQueue.push(frame);
    pthread_cond_signal(&mCond);
    pthread_mutex_unlock(&mMutex);
    
    FramedSource::afterGetting(this);
}

void DevFramedSource::doStopGettingFrames()
{
    stopCreateFrame();
}

void* DevFramedSource::threadFunc(void* data)
{
    DevFramedSource* source = (DevFramedSource*)data;
    source->createFrame();

    return NULL;
}

void DevFramedSource::stopCreateFrame()
{
    if(mThreadRun == false)
        return;

    mThreadRun = false;
    pthread_cond_broadcast(&mCond);
    pthread_join(mThreadId, NULL);
}

void DevFramedSource::createFrame()
{
    while(mThreadRun == true)
    {
        pthread_mutex_lock(&mMutex);

        if(mInputQueue.empty())
        {
            pthread_cond_wait(&mCond, &mMutex);
            if(mThreadRun == false)
            {
                pthread_mutex_unlock(&mMutex);
                break;
            }
        }

        Frame* frame = mInputQueue.front();
        mInputQueue.pop();
        pthread_mutex_unlock(&mMutex);

        while(createFrame(frame) == false);

        pthread_mutex_lock(&mMutex);
        mOutputQueue.push(frame);
        pthread_mutex_unlock(&mMutex);
    }
}

