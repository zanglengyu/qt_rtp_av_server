#include "cameracapture.h"

#include <QCameraImageCapture>

CameraCapture* CameraCapture::ins_ = nullptr;
CameraCapture::CameraCapture() : camera_(nullptr) {
    ins_ = this;
    init_camearas();
}

void CameraCapture::init_camearas() {
    QList<QCameraInfo> all_devices = QCameraInfo::availableCameras();
    for (auto info : all_devices) {
        camera_device_names_.append(info.description());
    }
}

CameraCapture* CameraCapture::get_ins() { return ins_; }

QStringList CameraCapture::get_cameras() { return camera_device_names_; }

void CameraCapture::set_camera(QObject* qml_camera) {
    if (camera_) {
        delete camera_;
    }
    camera_ = qvariant_cast<QCamera*>(qml_camera->property("mediaObject"));
    camera_image_capture_ = new QCameraImageCapture(camera_);
    camera_->setCaptureMode(QCamera::CaptureStillImage);  //将其采集为图片
    connect(camera_image_capture_, SIGNAL(imageCaptured(int, QImage)), this, SLOT(displayImage(int, QImage)));
    camera_image_capture_->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);
}

void CameraCapture::displayImage(int index, const QImage& image) {
    unique_lock<std::mutex> locker(buffer_mutex_);
    QImage* image1 = new QImage(move(image));
    video_frames_.append(image1);

    if (video_frames_.size() > 100) {
        QImage* pop_img = video_frames_.front();
        video_frames_.pop_front();
        delete pop_img;
    }
}

QImage* CameraCapture::read_video_frame() {
    unique_lock<std::mutex> locker(buffer_mutex_);
    if (video_frames_.size() > 0) {
        QImage* image = video_frames_.front();
        video_frames_.pop_front();

        return image;
    }
    return nullptr;
}
