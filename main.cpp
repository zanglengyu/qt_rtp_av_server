#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <QLocale>
#include <QTranslator>
#include "udpclient.h"
#include <QQmlContext>
#include <audiocapture.h>
#include <cameracapture.h>
#include "videoaudiomux.h"

int main(int argc, char *argv[])
{
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
  udp.init_udp_socket("192.168.20.115",3000);

  engine.rootContext()->setContextProperty("udp_connect",&udp);

  AudioCapture audio_capture;

  engine.rootContext()->setContextProperty("audio_capture",&audio_capture);

  CameraCapture camera_capture;

  engine.rootContext()->setContextProperty("camera_capture",&camera_capture);



  const QUrl url(QStringLiteral("qrc:/main.qml"));
  QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                   &app, [url](QObject *obj, const QUrl &objUrl) {
    if (!obj && url == objUrl)
      QCoreApplication::exit(-1);
  }, Qt::QueuedConnection);
  engine.load(url);
  VideoAudioMux video_audio_mux;

  return app.exec();
}
