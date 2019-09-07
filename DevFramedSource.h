#ifndef _DEV_FRAMED_SOURCE_H_
#define _DEV_FRAMED_SOURCE_H_
#include <queue>
#include <string>
#include <pthread.h>

#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>

class DevFramedSource : public FramedSource
{
public:
    virtual void doGetNextFrame();	
    virtual void doStopGettingFrames();

    static void startCreateFrame(void* data);
    void startCreateFrame();

protected:
    class Frame
    {
    public:
        Frame(int size) :
            mSize(size),
            mFrame(new uint8_t[size]),
            mFrameSize(0),
            mDurationInMicroseconds(0)
        { }

        ~Frame()
        {
            delete[] mFrame;
        }

        uint8_t* const mFrame;
        int mFrameSize;
        timeval mTime;
        uint32_t mDurationInMicroseconds;

    private: 
        int mSize;
    };

    DevFramedSource(UsageEnvironment& env, const char* dev, int frameSize=1000000);
    virtual ~DevFramedSource();

    void setFps(int fps) { mFps = fps; }
    int fps() { return mFps; }

    virtual bool createFrame(Frame* frame) = 0;
    void stopCreateFrame();

private:
    bool videoInit();
    bool videoExit();

    static void* threadFunc(void* data);
    void createFrame();

    static void afterGetNextFrame(void* data);

protected:
    std::string mDev;

private:
    TaskToken mTaskToken;
    TaskToken mNextTaskToken;

    Frame* mFrameArray[4];

    std::queue<Frame*> mInputQueue;
    std::queue<Frame*> mOutputQueue;
    
    struct v4l2_buf* mV4l2Buf;
    struct v4l2_buf_unit* mV4l2BufUnit;

    pthread_t mThreadId;
    pthread_mutex_t mMutex;
    pthread_cond_t mCond;
    bool mThreadRun;

    int mFps;
};

#endif //_V4L2_FRAMED_SOURCE_H_
