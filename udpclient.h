#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <QObject>
#include <QUdpSocket>
#include <QString>
#include <QNetworkDatagram>
#include <QByteArray>

struct RTPPacket{
  unsigned char * data;
  int size;
};

struct MsgHeader{
  int number;
  unsigned char data_type;
  unsigned char  sender_id[32];
  unsigned char  send_dts[32];
  int data_length;
};

class UdpClient : public QObject
{
  Q_OBJECT
public:
  UdpClient();
  ~UdpClient();
  void init_udp_socket(QString ip,int port);
  Q_INVOKABLE void send_msg(const QString& msg);
  void parse_buffer(QNetworkDatagram data);

  bool read_msg_header(MsgHeader& header);
  int read_msg_data(unsigned char* data);
public slots:
  void onReceiveMsg();
private:
  QUdpSocket *udp_socket_;
  QString bind_ip_address_;
  QString bind_port_;

  QByteArray receive_buffer_;

};

#endif // UDPCLIENT_H
