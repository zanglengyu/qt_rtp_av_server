#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "udpclient.h"
#include "videoaudiomux.h"

#include <QImage>
#include <QLocale>
#include <QPixmap>
#include <QQmlContext>
#include <QScreen>
#include <QTranslator>
#include <audiocapture.h>
#include <cameracapture.h>
int main(int argc, char *argv[]) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
  QGuiApplication app(argc, argv);

  QTranslator translator;
  const QStringList uiLanguages = QLocale::system().uiLanguages();
  for (const QString &locale : uiLanguages) {
    const QString baseName = "VAtransfrom_" + QLocale(locale).name();
    if (translator.load(":/i18n/" + baseName)) {
      app.installTranslator(&translator);
      break;
    }
  }

  QQmlApplicationEngine engine;
  UdpClient udp;
  udp.init_udp_socket("192.168.20.115", 3000);

  engine.rootContext()->setContextProperty("udp_connect", &udp);

  AudioCapture audio_capture;

  engine.rootContext()->setContextProperty("audio_capture", &audio_capture);

  CameraCapture camera_capture;

  engine.rootContext()->setContextProperty("camera_capture", &camera_capture);

  const QUrl url(QStringLiteral("qrc:/main.qml"));
  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreated, &app,
      [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
          QCoreApplication::exit(-1);
      },
      Qt::QueuedConnection);
  engine.load(url);
  VideoAudioMux video_audio_mux;

  QScreen *screen = QGuiApplication::primaryScreen();
  QPixmap map = screen->grabWindow(0);
  QImage *image = new QImage(std::move(map.toImage()));
  qDebug() << "image formagt = " << image->format();
  QImage *imageRGB32 =
      new QImage(image->width(), image->height(), QImage::Format_RGB888);

  for (int i = 0; i != image->height(); ++i) {
    for (int j = 0; j < image->width(); j++) {
      QColor rgb = image->pixel(j, i);
      QColor color = QColor(rgb.blue(), rgb.green(), rgb.red());
      imageRGB32->setPixelColor(j, i, color);
    }
  }
  qDebug() << "image color count:" << image->colorCount() << Qt::endl;
  qDebug() << "rgb32 color count :" << imageRGB32->colorCount() << Qt::endl;
  image->save("rgb.jpg");
  imageRGB32->save("bgr.jpg");
  return app.exec();
}
