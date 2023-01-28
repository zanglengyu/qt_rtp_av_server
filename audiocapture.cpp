#include "audiocapture.h"
#include "WriteDataDevice.h"
#include <QDebug>

AudioCapture *AudioCapture::ins_ = nullptr;

AudioCapture::AudioCapture()
    : audio_device_index_(0), input_device_(nullptr), device_data_(nullptr) {
  ins_ = this;
  init_devices();
}

void AudioCapture::init_devices() {

  QList<QAudioDeviceInfo> devices =
      QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
  qDebug() << "devices size = " << devices.size() << Qt::endl;
  for (auto &device_info : devices) {
    qDebug() << "inputDevice device name = " << device_info.deviceName()
             << Qt::endl;
    audio_input_devices_.append(device_info);
    audio_input_device_names_ << device_info.deviceName();
  }
}

QStringList AudioCapture::get_audio_device_names() {

  return audio_input_device_names_;
}

void AudioCapture::set_select_device_index(int index) {
  audio_device_index_ = index;
  QAudioDeviceInfo input_device = audio_input_devices_[index];
  QAudioFormat formatAudio;
  formatAudio.setSampleRate(48000);
  formatAudio.setChannelCount(1);
  formatAudio.setSampleSize(16);
  formatAudio.setCodec("audio/pcm");
  formatAudio.setByteOrder(QAudioFormat::LittleEndian);
  formatAudio.setSampleType(QAudioFormat::SignedInt);

  device_data_ = new WriteDataDevice;
  device_data_->open(QIODevice::ReadWrite);

  qDebug() << "inputDevice device name = " << input_device.deviceName()
           << Qt::endl;
  input_device_ = new QAudioInput(input_device, formatAudio);
  input_device_->start(device_data_);
}

AudioCapture *AudioCapture::ins() { return ins_; }

qint64 AudioCapture::read_buffer(char *data, int length) {

  return device_data_->readData(data, length);
}
