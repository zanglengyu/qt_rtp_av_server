#ifndef VIDEOAUDIOMUTEX_H
#define VIDEOAUDIOMUTEX_H
#include <QObject>
extern "C" {
#include <stdio.h>

#include "libavcodec/avcodec.h"
#include "libavdevice/avdevice.h"
#include "libavformat/avformat.h"
#include "libavutil/audio_fifo.h"
#include "libavutil/hwcontext.h"
#include "libavutil/imgutils.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avdevice.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "swresample.lib")
#pragma comment(lib, "swscale.lib")
//
}

class VideoAudioMux : public QObject {
    Q_OBJECT
  public:
    VideoAudioMux();
    void init_av_mux();
    void start_mux();
    void stop_mux();
    void fill_by_image(AVFrame* frame, QImage image);
    void rgb32_2_yuv420(int width, int height, uint8_t* rgb_buffer, uint8_t* yuv_buffer);

  public slots:
    void timeout();

  private:
    AVFormatContext* av_format_context_;
    AVCodecContext* av_code_context_;
    AVCodecContext* av_audio_code_context_;
    AVStream* video_stream_;
    AVStream* audio_stream_;
    SwrContext* audio_convert_ctx_;
    bool mux_state_;
};

#endif  // VIDEOAUDIOMUTEX_H
