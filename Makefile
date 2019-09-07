CXX = g++

SRC = Main.cpp MulticastServerMediaSubsession.cpp UnicastServerMediaSubsession.cpp DevUnicastServerMediaSubsession.cpp \
        V4l2H264DevUnicastServerMediaSubsession.cpp  DevFramedSource.cpp V4l2DevFramedSource.cpp  \
        V4l2H264DevFramedSource.cpp V4l2.cpp DevMulticastServerMediaSubsession.cpp V4l2H264DevMulticastServerMediaSubsession.cpp \
        AlsaDevFramedSource.cpp AlsaAACDevFramedSource.cpp AlsaAACDevUnicastServerMediaSubsession.cpp \
        AlsaAACDevMulticastServerMediaSubsession.cpp

TAR = liveRtspServer

CXX_FLAGS = -g -I/usr/local/include/liveMedia/ -I/usr/local/include/BasicUsageEnvironment \
                -I/usr/local/include/groupsock -I/usr/local/include/UsageEnvironment

LD_FLAGS = -lliveMedia -lBasicUsageEnvironment -lgroupsock -lUsageEnvironment -lpthread -lx264 -lasound -lfaac

all:
        $(CXX) -o $(TAR) $(SRC) $(CXX_FLAGS) $(LD_FLAGS)
