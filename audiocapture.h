#ifndef AUDIOCAPTURE_H
#define AUDIOCAPTURE_H

#include <QAudioDeviceInfo>
#include <QAudioInput>

#include "WriteDataDevice.h"
#include <QStringList>

class AudioCapture : public QObject {
  Q_OBJECT

public:
  AudioCapture();
  void init_devices();
  Q_INVOKABLE QStringList get_audio_device_names();
  Q_INVOKABLE void set_select_device_index(int index);
  static AudioCapture *ins();
  qint64 read_buffer(char *data, int length);

private:
  QStringList audio_input_device_names_;
  QList<QAudioDeviceInfo> audio_input_devices_;
  int audio_device_index_;
  QAudioInput *input_device_;
  WriteDataDevice *device_data_;
  static AudioCapture *ins_;
};

#endif // AUDIOCAPTURE_H
