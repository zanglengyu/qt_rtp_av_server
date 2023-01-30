#include "videoaudiomux.h"

#include "audiocapture.h"
#include "cameracapture.h"
#include <QDateTime>
#include <QThread>
#include <QTimer>
#include <string>
#include <thread>
VideoAudioMux::VideoAudioMux()
    : av_format_context_(nullptr), av_code_context_(nullptr),
      video_stream_(nullptr), audio_stream_(nullptr) {

  //  拉取摄像头与麦克风画面编码成MP4格式流程
  //  通过ffmpeg或者Qt，拉取取到摄像头画面 用ffmpeg编码
  //  ffmpeg编码主要分为一下几步;

  //  AVFormatContext *av_format_context_;
  //  AVCodecContext *av_code_context_;
  //  AVStream *video_stream_;
  //  AVStream *audio_stream_;
  //  bool mux_state_;

  //  初始化ffmgpeg封包管理器，AVFormatContext 获取编码格式 AVoutputFormat
  //  打开文件; 添加视频流 创建编码器管理器，即AVCodecContext 设置相关参数;
  //  根据编码器类型获取编码器 AVCodec 打开编码器并设置相关参数;
  // setvbuf(stdout, NULL, _IONBF, 0);

  av_log_set_level(AV_LOG_DEBUG);
  av_log_set_callback(av_log_default_callback);

  avdevice_register_all();
  avformat_network_init();

  std::string out_file = "tests11.mp4";

  // string out_file = "rtsp://192.168.20.115:8554/ffmpeg";
  // string out_file = "rtmp://192.168.20.115/ffmpeg";
  if (avformat_alloc_output_context2(&av_format_context_, NULL, "flv",
                                     out_file.c_str()) < 0) {
    printf("Fail: avformat_alloc_output_context2\n");
    return;
  }

  int r = avio_open(&av_format_context_->pb, out_file.c_str(), AVIO_FLAG_WRITE);
  if (r < 0) {
    qDebug() << "Failed to open output file error = " << r << Qt::endl;
    return;
  }

  video_stream_ = avformat_new_stream(av_format_context_, 0);
  if (!video_stream_) {
    qDebug() << "Failed to open video_stream_" << Qt::endl;
    return;
  }
  video_stream_->time_base.den = 25;
  video_stream_->time_base.num = 1;
  video_stream_->index = 0;
  video_stream_->codecpar->codec_id = AV_CODEC_ID_H264;
  video_stream_->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
  video_stream_->codecpar->width = 1280;
  video_stream_->codecpar->height = 720;

  const AVCodec *tmp_codec = avcodec_find_encoder(AV_CODEC_ID_H264);
  av_code_context_ = avcodec_alloc_context3(tmp_codec);
  av_code_context_->codec_id = AV_CODEC_ID_H264;
  av_code_context_->codec_type = AVMEDIA_TYPE_VIDEO;
  av_code_context_->pix_fmt = AV_PIX_FMT_YUV420P;
  av_code_context_->width = 1280;
  av_code_context_->height = 720;
  av_code_context_->bit_rate = 400000;
  av_code_context_->gop_size = 250;
  av_code_context_->time_base.num = 1;
  av_code_context_->time_base.den = 25;
  av_code_context_->qmin = 10;
  av_code_context_->qmax = 51;
  av_code_context_->max_b_frames = 3;

  av_opt_set(av_code_context_->priv_data, "preset", "slow", 0);
  av_opt_set(av_code_context_->priv_data, "tune", "zerolatency", 0);

  if (avcodec_open2(av_code_context_, tmp_codec, NULL) < 0) {
    qDebug() << "Failed to open encoder! " << Qt::endl;
    return;
  }
  if (!av_codec_is_encoder(av_code_context_->codec)) {
    qDebug() << "av_codec_is_encoder  ! " << Qt::endl;
    return;
  }
  avcodec_parameters_from_context(video_stream_->codecpar, av_code_context_);

  // 创建aac编码器和编码器上下文章，并创建音频流
  if (av_format_context_->oformat->audio_codec != AV_CODEC_ID_NONE) {
    audio_stream_ = avformat_new_stream(av_format_context_, 0);
    audio_stream_->time_base.den = 48000;
    audio_stream_->time_base.num = 1;
    audio_stream_->id = 1;
  }

  // AV_CODEC_ID_AAC
  const AVCodec *tmp_audio_codec =
      avcodec_find_encoder(av_format_context_->oformat->audio_codec);

  av_audio_code_context_ = avcodec_alloc_context3(tmp_audio_codec);
  av_audio_code_context_->channel_layout = AV_CH_LAYOUT_STEREO;
  av_audio_code_context_->channels =
      av_get_channel_layout_nb_channels(av_audio_code_context_->channel_layout);
  av_audio_code_context_->time_base.den = 48000;
  av_audio_code_context_->time_base.num = 1;

  av_audio_code_context_->bit_rate = 96000;
  av_audio_code_context_->sample_rate = 48000;
  av_audio_code_context_->codec_id = av_format_context_->oformat->audio_codec;
  av_audio_code_context_->sample_fmt = AV_SAMPLE_FMT_FLTP;
  av_audio_code_context_->profile = FF_PROFILE_AAC_MAIN;

  av_audio_code_context_->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

  if (av_format_context_->oformat->flags & AVFMT_GLOBALHEADER) {
    av_audio_code_context_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    av_code_context_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
  }

  int ret = avcodec_open2(av_audio_code_context_, tmp_audio_codec, NULL);
  if (ret < 0) {
    //编码器上下位，打开编码器，并设置参数
    qDebug() << "Failed to open audio encoder, ret = " << ret << Qt::endl;
    return;
  }
  ret = avcodec_parameters_from_context(audio_stream_->codecpar,
                                        av_audio_code_context_);
  if (ret < 0) {
    //编码器上下位，打开编码器，并设置参数
    qDebug() << "Failed to open audio encoder, ret = " << ret << Qt::endl;
    return;
  }
  if (!av_codec_is_encoder(av_audio_code_context_->codec)) {
    qDebug() << "av_codec_is_encoder  ! " << Qt::endl;
    return;
  }

  audio_convert_ctx_ = swr_alloc();
  audio_convert_ctx_ = swr_alloc_set_opts(
      audio_convert_ctx_, av_audio_code_context_->channel_layout,
      av_audio_code_context_->sample_fmt, av_audio_code_context_->sample_rate,
      AV_CH_LAYOUT_MONO, AV_SAMPLE_FMT_S16, av_audio_code_context_->sample_rate,
      0, nullptr);
  int res = swr_init(audio_convert_ctx_);
  if (res != 0) {
    qDebug() << "swr init error " << res << Qt::endl;
  }

  av_dump_format(av_format_context_, 1, out_file.c_str(), 1);
  // fflush(stderr);

  int error = avformat_write_header(av_format_context_, NULL);
  if (error < 0) {
    qDebug() << "Could not write output file header " << Qt::endl;
    cout << "error : " << (error) << endl;
    return;
  }
  std::thread thread(&VideoAudioMux::start_mux, this);
  thread.detach();

  QTimer *timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this,
          QOverload<>::of(&VideoAudioMux::timeout));
  timer->start(15000);
  qDebug() << "start success !! " << res << Qt::endl;
}

void VideoAudioMux::init_av_mux() {}

void VideoAudioMux::start_mux() {
  qDebug() << "start_mux" << Qt::endl;
  mux_state_ = true;
  AVPacket *packet = av_packet_alloc();

  long video_frame_index = 1;
  long audio_sample_index = 1;
  // ARGB2YUV422p数据重采样
  AVPixelFormat pixel_format = av_code_context_->pix_fmt;

  int width = av_code_context_->width;
  int height = av_code_context_->height;

  while (true) {
    if (!mux_state_) {
      qDebug() << "quit write !" << Qt::endl;
      break;
    }
    QThread::msleep(1);

    int time_res =
        av_compare_ts(video_frame_index, av_code_context_->time_base,
                      audio_sample_index, av_audio_code_context_->time_base);
    AVFrame *frame = av_frame_alloc();
    if (!frame) {
      qDebug() << "Could not allocate video frame" << Qt::endl;
      break;
    }
    if (time_res < 0) {
      //音频下一帧时间比视频大
      QImage *img = CameraCapture::get_ins()->read_video_frame();
      if (!img) {
        continue;
      }

      frame->format = pixel_format;
      frame->width = width;
      frame->height = height;
      av_frame_get_buffer(frame, 0);
      fill_by_image(frame, *img);
      delete img;
      int ret = avcodec_send_frame(av_code_context_, frame);
      if (ret == 0) {
        while (true) {
          ret = avcodec_receive_packet(av_code_context_, packet);
          if (ret >= 0) {
            packet->stream_index = video_stream_->index;
            if (packet->pts == AV_NOPTS_VALUE) {
              packet->pts = video_frame_index;
              packet->duration = 1;
              packet->dts = packet->pts;
              video_frame_index++;
            }
            av_write_frame(av_format_context_, packet);
            av_packet_unref(packet);
            qDebug() << "video data write success" << Qt::endl;
          } else {
            break;
          }
        }
      }
    } else {
      //写入音频帧
      int length = av_audio_code_context_->frame_size;
      char *audio_frame = (char *)av_malloc(length * sizeof(int16_t));
      int size = AudioCapture::ins()->read_buffer(audio_frame,
                                                  length * sizeof(int16_t));
      if (size <= 0) {
        av_free(audio_frame);

      } else {

        frame->nb_samples = av_audio_code_context_->frame_size;
        frame->format = av_audio_code_context_->sample_fmt;
        frame->channel_layout = av_audio_code_context_->channel_layout;
        frame->channels = av_audio_code_context_->channels;
        frame->sample_rate = av_audio_code_context_->sample_rate;
        frame->pts = audio_sample_index;
        frame->time_base = av_audio_code_context_->time_base;
        audio_sample_index += frame->nb_samples;
        //        unsigned char *out_audio_buf[2] = {0};
        unsigned char *in_audio_buf[2] = {0};
        av_frame_get_buffer(frame, 0);
        in_audio_buf[0] = (unsigned char *)audio_frame;
        swr_convert(audio_convert_ctx_, frame->data, frame->nb_samples,
                    (const uint8_t **)in_audio_buf, frame->nb_samples);
        int ret = avcodec_send_frame(av_audio_code_context_, frame);
        // 发送数据到解码队列中
        if (ret == 0) {
          while (true) {
            ret = avcodec_receive_packet(av_audio_code_context_, packet);
            if (ret >= 0) {
              packet->stream_index = audio_stream_->index;
              av_write_frame(av_format_context_, packet);

              av_packet_unref(packet);
              qDebug() << "audio data write   success  " << Qt::endl;
            } else {

              break;
            }
          }
        } else {
        }
      }
    }
    // 以时间为基本单位的表示时间戳（应该向用户显示帧的时间）。
    // 编码一帧视频。即将AVFrame（存储YUV像素数据）编码为AVPacket（存储H.264等格式的码流数据）。
    av_frame_free(&frame);
  }

  // 发送数据到解码队列中
  qDebug() << "start flush last data " << Qt::endl;
  QList<AVPacket *> last_video_packets;
  int ret = avcodec_send_frame(av_code_context_, NULL);
  if (ret == 0) {
    while (true) {
      ret = avcodec_receive_packet(av_code_context_, packet);
      if (ret >= 0) {
        packet->stream_index = video_stream_->index;
        packet->pts = video_frame_index;
        packet->duration = 1;
        packet->dts = packet->pts;
        video_frame_index++;
        last_video_packets.push_back(packet);
      } else {
        break;
      }
    }
  }
  qDebug() << "start last_video_packets size =  " << last_video_packets.size()
           << Qt::endl;

  QList<AVPacket *> last_audio_packets;
  ret = avcodec_send_frame(av_audio_code_context_, NULL);
  if (ret == 0) {
    while (true) {
      ret = avcodec_receive_packet(av_audio_code_context_, packet);
      if (ret >= 0) {
        packet->stream_index = audio_stream_->index;
        packet->pts = audio_sample_index;
        packet->dts = audio_sample_index;
        audio_sample_index += av_audio_code_context_->frame_size;
        last_audio_packets.push_back(packet);
      } else {
        break;
      }
    }
  }
  qDebug() << "start last_audio_packets size =  " << last_audio_packets.size()
           << Qt::endl;
  while (true) {
    if (last_video_packets.size() <= 0) {
      for (auto packet : last_audio_packets) {
        av_write_frame(av_format_context_, packet);
        av_packet_unref(packet);
      }
      break;
    }

    if (last_audio_packets.size() <= 0) {
      for (auto packet : last_video_packets) {
        av_write_frame(av_format_context_, packet);
        av_packet_unref(packet);
      }
      break;
    }
    AVPacket *video_packet = last_video_packets.front();
    AVPacket *audio_packet = last_audio_packets.front();
    int time_res =
        av_compare_ts(video_packet->pts, av_code_context_->time_base,
                      audio_packet->pts, av_audio_code_context_->time_base);
    if (time_res <= 0) {
      av_write_frame(av_format_context_, video_packet);
      av_packet_unref(video_packet);
      last_video_packets.pop_front();

    } else {
      av_write_frame(av_format_context_, audio_packet);
      av_packet_unref(audio_packet);
      last_audio_packets.pop_front();
    }
  }

  av_packet_free(&packet);

  //写入尾部信息
  av_write_trailer(av_format_context_);
  //关闭文件指针
  avio_close(av_format_context_->pb);
  //释放资源
  avformat_free_context(av_format_context_);
  qDebug() << "end_mux  " << Qt::endl;
}

void VideoAudioMux::stop_mux() { mux_state_ = false; }

void VideoAudioMux::timeout() { stop_mux(); }

void VideoAudioMux::fill_by_image(AVFrame *frame, QImage image) {

  //由于是YUV420，采样时每四个像素点采集一组UV，每个像素点都采集一次Y
  // YUV  Y   YUV  Y    YUV   Y
  // Y    Y   Y    Y    Y     Y
  // YUV  Y   YUV  Y    YUV   Y
  // Y    Y   Y    Y    Y     Y
  for (int h = 0; h < image.height(); h++) {
    for (int w = 0; w < image.width(); w++) {
      QRgb rgb = image.pixel(w, h);

      int r = qRed(rgb);
      int g = qGreen(rgb);
      int b = qBlue(rgb);

      int dy = ((66 * r + 129 * g + 25 * b) >> 8) + 16;
      int du = ((-38 * r + -74 * g + 112 * b) >> 8) + 128;
      int dv = ((112 * r + -94 * g + -18 * b) >> 8) + 128;

      uchar yy = (uchar)dy;
      uchar uu = (uchar)du;
      uchar vv = (uchar)dv;

      frame->data[0][h * frame->linesize[0] + w] = yy;
      //平行放置Y

      if (h % 2 == 0 && w % 2 == 0) {
        frame->data[1][h / 2 * (frame->linesize[1]) + w / 2] = uu;
        //平行放置U
        frame->data[2][h / 2 * (frame->linesize[2]) + w / 2] = vv;
        //平行放置V
      }
    }
  }
}

void VideoAudioMux::rgb32_2_yuv420(int width, int height, uint8_t *rgb_buffer,
                                   uint8_t *yuv_buffer) {
  //这种采集方法如下
  //由于是YUV420，采样时每四个像素点采集一组UV，每个像素点都采集一次Y
  // YU   Y   YU   Y    YU   Y
  // YV   Y   YV   Y    YV   Y
  // YU   Y   YU   Y    YU   Y
  // YV   Y   YV   Y    YV   Y
  int i, j;
  uint8_t *bufY = yuv_buffer;
  uint8_t *bufU = yuv_buffer + width * height;
  uint8_t *bufV = bufU + (width * height * 1 / 4);
  uint8_t *bufRGB;
  unsigned char y, u, v, r, g, b;
  if (NULL == rgb_buffer) {
    return;
  }
  for (j = 0; j < height; j++) {
    bufRGB = rgb_buffer + width * (height - 1 - j) * 3;
    for (i = 0; i < width; i++) {
      r = *(bufRGB++);
      g = *(bufRGB++);
      b = *(bufRGB++);
      y = (unsigned char)((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;   // 16
      v = (unsigned char)((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128; // 128
      u = (unsigned char)((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
      *(bufY++) = __max(0, __min(y, 255));

      if (j % 2 == 0 && i % 2 == 0) {
        if (u > 255) {
          u = 255;
        }
        if (u < 0) {
          u = 0;
        }
        *(bufU++) = u;
        //存u分量
      } else {
        //存v分量
        if (i % 2 == 0) {
          if (v > 255) {
            v = 255;
          }
          if (v < 0) {
            v = 0;
          }
          *(bufV++) = v;
        }
      }
    }
  }
}
