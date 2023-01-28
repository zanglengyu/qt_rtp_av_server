#ifndef CAMERACAPTURE_H
#define CAMERACAPTURE_H

#include <QCamera>
#include <QCameraImageCapture>
#include <QCameraInfo>
#include <QStringList>
#include <iostream>
#include <memory>
#include <mutex>

using namespace std;
class CameraCapture : public QObject {
  Q_OBJECT
public:
  CameraCapture();

  void init_camearas();
  static CameraCapture *get_ins();

  Q_INVOKABLE QStringList get_cameras();
  Q_INVOKABLE void set_camera(QObject *qml_camera);

  QImage *read_video_frame();

public slots:
  void displayImage(int index, const QImage &image);

private:
  QList<QImage *> video_frames_;
  QCamera *camera_;
  QCameraImageCapture *camera_image_capture_;
  QList<QCameraInfo> all_devices = QCameraInfo::availableCameras();

  QStringList camera_device_names_;
  static CameraCapture *ins_;
  mutex buffer_mutex_;
};

#endif // CAMERACAPTURE_H
