#ifndef AUDIOCAPTURE_H
#define AUDIOCAPTURE_H

#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <QStringList>
#include <QUdpSocket>
#include <iostream>
#include <thread>

#include "WriteDataDevice.h"
struct ChannelData {
    int channel;
    std::vector<char> pcm_data;
};

const int kAudioBuffer = 10;  //缓存10毫秒数据再开始发送rtp
const int kSampleRate = 48000;
const int kSampleSize = 16;
const int kChannels = 1;

#define RTP_HDR_LEN 12
const int SINGLE_CHANNEL_PCM = 48 * kSampleSize / 8;
const int RTP_AUDIO_LENGTH = SINGLE_CHANNEL_PCM * kChannels;
struct RtpPacket {
    // 福川每秒48000次采样，
    // 一次长度为，4个通道数据
    //每个通道1毫秒的pcm数据和头部数据
    // 负载长度为 48000 *1/1000 = 48次采样
    // 一次采样为24位，所以每次每个通道一共是 48*3个字节
    // 一个组播最多4个通道，最多就是576字节数据

    //目前我们一次只发送两个通道，所以每次只有 288个字节
    // pts  每次增加48
    //序号每次加1

    char rpt_header[12] = {0};
    char pcm_data[RTP_AUDIO_LENGTH] = {0};
};
class AudioCapture : public QObject {
    Q_OBJECT

  public:
    AudioCapture();
    void init_devices();
    Q_INVOKABLE QStringList get_audio_device_names();
    Q_INVOKABLE void set_select_device_index(int index);

    static AudioCapture* ins();
    qint64 read_buffer(char* data, int length);
    void start_capture();
    Q_INVOKABLE void stop_capture();
    void transform_rtp_pkg();
    void receive_address(QString address, int port, int ssrc);
    int gb28181_make_rtp_header(char* pData, int seqNum, int64_t timestamp, int ssrc, int isEnd);

  private:
    QStringList audio_input_device_names_;
    QList<QAudioDeviceInfo> audio_input_devices_;
    int audio_device_index_;
    QAudioInput* input_device_;
    WriteDataDevice* device_data_;
    static AudioCapture* ins_;
    std::shared_ptr<std::thread> send_thread_;
    bool trans_state_;
    bool quit_thread_;
    std::shared_ptr<QUdpSocket> udp_socket_;
    QString address_;
    int port_;
    int ssrc_;
};

#endif  // AUDIOCAPTURE_H
