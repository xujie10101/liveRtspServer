#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>

#include "V4l2H264DevUnicastServerMediaSubsession.h"
#include "V4l2H264DevMulticastServerMediaSubsession.h"
#include "AlsaAACDevUnicastServerMediaSubsession.h"
#include "AlsaAACDevMulticastServerMediaSubsession.h"

/*
 * ./liveRtspServer [-p port] [-m multicast?]           \
 *                  [-w width] [-h height] [-f fps]     \
 *                  [-A fmt] [-F freq] [-c channels]    \
 *                  <-v video dev> <-a audio dev>
 * 
 * -p port:         rtsp server port(default "8554")
 * -m multicast?:   is multicast? yes or no(default "no")
 * -W width:        video width(default "320")
 * -H height:       video height(default "240")
 * -f fps:          video fps(defalut "15")
 * -A fmt:          audio sampling format,"s16le"、"s24le"、"s32le"(defalut "s16le")
 * -F freq:         audio sampling frequency(default "44100")
 * -c channels:     audio number of sampling channels(default "2")
 * -v video dev:    video device("/dev/videon")
 * -a audio dev:    audio device("hw:x,y")
 * -h:              help
 */

static void help()
{
    printf("./liveRtspServer [-p port] [-m multicast?]              \\\n"
            "                   [-W width] [-H height] [-f fps]     \\\n"
            "                   [-A fmt] [-F freq] [-c channels]    \\\n"
            "                   <-v video dev> <-a audio dev>\n"
            "-p port:         rtsp server port(default \"8554\")\n"
            "-m multicast?:   is multicast? yes or no(default \"no\")\n"
            "-W width:        video width(default \"320\")\n"
            "-H height:       video height(default \"240\")\n"
            "-f fps:          video fps(defalut \"15\")\n"
            "-A fmt:          audio sampling format;\"s16le\",\"u16le\",\"s32le\",\"u32le\"(defalut \"s16le\")\n"
            "-F freq:         audio sampling frequency(default \"44100\")\n"
            "-c channels:     audio number of sampling channels(default \"2\")\n"
            "-v video dev:    video device(\"/dev/videon\")\n"
            "-a audio dev:    audio device(\"hw:x,y\")\n"
            "-h:              help\n");
}

int main(int argc, char* argv[])
{ 
    int port = 8554;
    bool multicast = false;
    int videoWidth = 320;
    int videoHeight = 240;
    int videoFps = 15;
    std::string audioSamplingFmt("s16le");
    int audioSamplingFreq = 44100;
    int audioNumOfChannel = 2;
    std::string videoDev;
    std::string audioDev;
    char c;

    if(argc == 1)
    {
        help();
        return 0;
    }

    while ((c = getopt(argc, argv, "p:m:W:H:f:A:F:c:v:a:h")) != -1)
	{
		switch (c)
		{
        case 'p':
            port = atoi(optarg);
            printf("port: %d\n", port);
            break;

        case 'm':
            if(!strcmp(optarg, "yes"))
                multicast = true;
            else
                multicast = false;
            
            multicast == true ? printf("multicast\n") : printf("unicast\n");
            
            break;

        case 'W':
            videoWidth = atoi(optarg);
            printf("video width: %d\n", videoWidth);
            break;

        case 'H':
            videoHeight = atoi(optarg);
            printf("video height: %d\n", videoHeight);
            break;

        case 'f':
            videoFps = atoi(optarg);
            printf("video fps: %d\n", videoFps);
            break;

        case 'A':
            audioSamplingFmt = optarg;
            printf("audio sampling format: %s\n", audioSamplingFmt.c_str());
            break;

        case 'F':
            audioSamplingFreq = atoi(optarg);
            printf("audio sampling frequency: %d\n", audioSamplingFreq);
            break;

        case 'c':
            audioNumOfChannel = atoi(optarg);
            printf("audio number of sampling channels: %d\n", audioNumOfChannel);
            break;

        case 'v':
            videoDev = optarg;
            printf("video device: %s\n", videoDev.c_str());
            break;

        case 'a':
            audioDev = optarg;
            printf("audio device: %s\n", audioDev.c_str());
            break;

        case 'h':
        default:
            help();
            return 0;
        }
	}

    if(videoDev.empty() && audioDev.empty())
    {
        help();
        return 0;
    }

    TaskScheduler* scheduler = BasicTaskScheduler::createNew();
    UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);

    RTSPServer* rtspServer = RTSPServer::createNew(*env, port);
    if (rtspServer == NULL)
    {
        *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
        return -1;
    }

    ServerMediaSession* sms;

    if(multicast == true)
    {
        sms = ServerMediaSession::createNew(*env, "live", "live rtsp server",
                                                "live rtsp server", True /* 表示是否为多播*/);
        
        struct in_addr videoDestinationAddress;
        videoDestinationAddress.s_addr = chooseRandomIPv4SSMAddress(*env);
        const unsigned short videoRtpPortNum = 18888;
        const unsigned short videoRtcpPortNum = videoRtpPortNum+1;
        const Port videoRtpPort(videoRtpPortNum);
        const Port videoRtcpPort(videoRtcpPortNum);

        struct in_addr audioDestinationAddress;
        audioDestinationAddress.s_addr = chooseRandomIPv4SSMAddress(*env);
        const unsigned short audioRtpPortNum = 18890;
        const unsigned short audioRtcpPortNum = audioRtpPortNum+1;
        const Port audioRtpPort(audioRtpPortNum);
        const Port audioRtcpPort(audioRtcpPortNum);

        int ttl = 255;

        if(!videoDev.empty())
            sms->addSubsession(V4l2H264DevMulticastServerMediaSubsession::createNew(*env, videoDev.c_str(),
                                videoDestinationAddress, videoRtpPort, videoRtcpPort, ttl,
                                videoWidth, videoHeight, videoFps));
        
        if(!audioDev.empty())
            sms->addSubsession(AlsaAACDevMulticastServerMediaSubsession::createNew(*env, audioDev.c_str(),
                                audioDestinationAddress, audioRtpPort, audioRtcpPort, ttl,
                                audioSamplingFreq, audioNumOfChannel, audioSamplingFmt.c_str()));
    }
    else
    {
        sms = ServerMediaSession::createNew(*env, "live", "live rtsp server",
                                                "live rtsp server");

        if(!videoDev.empty())
            sms->addSubsession(V4l2H264DevUnicastServerMediaSubsession::createNew(*env, videoDev.c_str(),
                                    True, videoWidth, videoHeight, videoFps));
        
        if(!audioDev.empty())
            sms->addSubsession(AlsaAACDevUnicastServerMediaSubsession::createNew(*env, audioDev.c_str(),
                                    True, audioSamplingFreq, audioNumOfChannel, audioSamplingFmt.c_str()));
    }

    rtspServer->addServerMediaSession(sms);

    char* url = rtspServer->rtspURL(sms);
    *env << "Play this stream using the URL \"" << url << "\"\n";
    delete[] url;

    env->taskScheduler().doEventLoop();

    return 0;
}