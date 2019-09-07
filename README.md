# liveRtspServer

## 一、项目简介

基于live555的RTSP服务器

## 二、功能介绍

- RTP/UDP 单播
- RTP/UDP 多播
- RTP/TCP
- 采集V4L2摄像头数据，编码成H.264
- 采集ALSA声卡数据，编码成AAC

## 三、依赖

- [live555](http://www.live555.com/ )
- [X264](https://www.videolan.org/developers/x264.html )
- [alsa-lib](https://www.alsa-project.org/main/index.php/Main_Page )、[faac](https://www.audiocoding.com/downloads.html )

## 四、编译

安装上述的依赖库后，执行

```
# make
```

生成`liveRtspServer`

## 四、用法

```
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
```

**示例**

- 采集摄像头

  ```
  ./liveRtspServer -v /dev/video0
  ```

- 采集声卡

  ```
  ./liveRtspServer -a hw:0,0
  ```

- 同时采集

  ```
  ./liveRtspServer -v /dev/video0 -a hw:0,0
  ```

- 多播

  ```
  ./liveRtspServer -m yes -v /dev/video0 -a hw:0,0
  ```

## 五、效果

![在这里插入图片描述](https://img-blog.csdnimg.cn/20190907212746924.PNG?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl80MjQ2MjIwMg==,size_16,color_FFFFFF,t_70)

## 六、联系方式

- 邮箱：1345648755@qq.com

