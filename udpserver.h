#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QObject>
#include "udpclient.h"

class UdpServer : public QObject
{
public:
  UdpServer();
  ~UdpServer();
  void init_udp_server(QString ip,int port);

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

#endif // UDPSERVER_H
