#include "audiocapture.h"

#include <windows.h>

#include <QDebug>
#include <chrono>

#include "Bits.h"
#include "WriteDataDevice.h"
AudioCapture* AudioCapture::ins_ = nullptr;

AudioCapture::AudioCapture() : audio_device_index_(0), input_device_(nullptr), device_data_(nullptr) {
    ins_ = this;
    init_devices();
}

void AudioCapture::init_devices() {
    QList<QAudioDeviceInfo> devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    qDebug() << "devices size = " << devices.size() << Qt::endl;
    for (auto& device_info : devices) {
        qDebug() << "inputDevice device name = " << device_info.deviceName() << Qt::endl;
        audio_input_devices_.append(device_info);
        audio_input_device_names_ << device_info.deviceName();
    }
}

QStringList AudioCapture::get_audio_device_names() { return audio_input_device_names_; }

void AudioCapture::set_select_device_index(int index) {
    qDebug() << "set_select_device_index = " << index << endl;
    audio_device_index_ = index;
}

AudioCapture* AudioCapture::ins() { return ins_; }

qint64 AudioCapture::read_buffer(char* data, int length) { return device_data_->readData(data, length); }

void AudioCapture::start_capture() {
    QAudioDeviceInfo input_device = audio_input_devices_[audio_device_index_];
    QAudioFormat formatAudio;
    formatAudio.setSampleRate(kSampleRate);
    formatAudio.setChannelCount(kChannels);
    formatAudio.setSampleSize(kSampleSize);
    formatAudio.setCodec("audio/pcm");
    formatAudio.setByteOrder(QAudioFormat::LittleEndian);
    formatAudio.setSampleType(QAudioFormat::SignedInt);

    if (device_data_) {
        device_data_->restart();
    } else {
        device_data_ = new WriteDataDevice;
        device_data_->open(QIODevice::ReadWrite);
    }

    if (input_device_) {
        input_device_->start(device_data_);
    } else {
        input_device_ = new QAudioInput(input_device, formatAudio);
        input_device_->start(device_data_);
    }

    qDebug() << "inputDevice device name = " << input_device.deviceName() << Qt::endl;
}
void AudioCapture::receive_address(QString address, int port, int ssrc) {
    address_ = address;
    port_ = port;
    send_thread_ = std::make_shared<std::thread>(&AudioCapture::transform_rtp_pkg, this);
    trans_state_ = true;
    quit_thread_ = false;
    udp_socket_ = nullptr;
    ssrc_ = ssrc;
    send_thread_->detach();

    start_capture();
}

void AudioCapture::stop_capture() {
    quit_thread_ = true;
    if (input_device_) {
        input_device_->stop();
        input_device_ = nullptr;
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
    if (device_data_) {
        delete device_data_;
        device_data_ = nullptr;
    }
}

int AudioCapture::gb28181_make_rtp_header(char* pData, int seqNum, int64_t timestamp, int ssrc, int isEnd) {
    bits_buffer_t bitsBuffer;
    bitsBuffer.i_size = RTP_HDR_LEN;
    bitsBuffer.i_data = 0;
    bitsBuffer.i_mask = 0x80;
    bitsBuffer.p_data = (unsigned char*)(pData);
    memset(bitsBuffer.p_data, 0, RTP_HDR_LEN);  // 12个字节，共96位
    bits_write(&bitsBuffer, 2, 2);              /*协议版本*/
    bits_write(&bitsBuffer, 1, 0);              /*P*/
    bits_write(&bitsBuffer, 1, 0);              /*X*/
    bits_write(&bitsBuffer, 4, 0);              /*CSRC个数*/
    bits_write(&bitsBuffer, 1, isEnd);          /*一帧是否结束*/
    bits_write(&bitsBuffer, 7, 97);             /*载荷的数据类型*/
    bits_write(&bitsBuffer, 16, seqNum);        /*序列号，第几个*/
    bits_write(&bitsBuffer, 32, timestamp);     /*时间戳，第一个 */
    bits_write(&bitsBuffer, 32, ssrc);          /*同步信源(SSRC)标识符*/
                                                // 1000 0000
                                                // 1 1
                                                // 7 97
                                                // 16

    // 1000 0000 0110 0000
    // 2 5 +26
    // 32 + 64
    // 80 60 e1 49
    // 01 60 10 10
    // 00 00 00 02

    // 80 60 e1 4A
    // 01 60 10 40

    // time pts 增加了48
    //序号增加了 1
    // 长度位588字节，头部12
    //数据共576字节
    // 48是一毫秒的数据，
    // 如果是1毫秒的数据，每秒48个采样，每个采样3个字节，四个通道就是 48*3*4 576 字节对不上

    return 0;
}
void AudioCapture::transform_rtp_pkg() {  // init header buffer
    udp_socket_ = std::make_shared<QUdpSocket>();

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    GetThreadPriority(GetCurrentThread());

    unsigned short seq = 0;
    int64_t timestamp = -100;
    int ssrc = ssrc_;

    while (!quit_thread_) {
        std::vector<ChannelData> channel_datas;
        std::shared_ptr<RtpPacket> four_channel_pcm = std::make_shared<RtpPacket>();
        if (device_data_) {
            int read_length = device_data_->readData(four_channel_pcm->pcm_data, RTP_AUDIO_LENGTH);
            if (read_length > 0) {
                gb28181_make_rtp_header(four_channel_pcm->rpt_header, seq++, timestamp, ssrc, 1);
                timestamp += 1 * 48;
                QByteArray data_rtp((char*)four_channel_pcm.get(), sizeof(RtpPacket));
                qDebug() << "rtp address_ = " << address_ << " , port_ = " << port_ << endl;
                udp_socket_->writeDatagram(data_rtp, QHostAddress(address_), port_);
            } else {
            }
        }
        LARGE_INTEGER perfCnt, start, now;

        QueryPerformanceFrequency(&perfCnt);
        QueryPerformanceCounter(&start);

        do {
            QueryPerformanceCounter((LARGE_INTEGER*)&now);
        } while ((now.QuadPart - start.QuadPart) / float(perfCnt.QuadPart) * 1000 * 1000 < 1000);
    }
    udp_socket_->close();
    udp_socket_.reset();
    udp_socket_ = nullptr;
    std::cout << "rtp audio  thread end" << std::endl;
}
