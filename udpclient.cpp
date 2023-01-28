#include "udpclient.h"

#include <QDebug>
#include <sstream>
#include <QDateTime>

using namespace  std;

UdpClient::UdpClient():udp_socket_(nullptr){

}

UdpClient::~UdpClient(){

}


void UdpClient::init_udp_socket(QString ip,int port){
  udp_socket_ = new QUdpSocket();
  QHostAddress hostAddress(ip);
  bool res = udp_socket_->bind(hostAddress,port,QUdpSocket::ShareAddress);
  if(!res){
    qDebug()<<"bind ip ,port error"<<Qt::endl;
  }else{
       qDebug()<<"bind ip success ip = "<<hostAddress<<"port:"<<port<<Qt::endl;
  }
  connect(udp_socket_,&QUdpSocket::readyRead,this,&UdpClient::onReceiveMsg);
}

Q_INVOKABLE void UdpClient::send_msg(const QString& msg){


  ostringstream osstr;
  MsgHeader msg_header;
  msg_header.data_length = msg.toStdString().size();
  msg_header.data_type=0x00;
  msg_header.number = 0;
  string id = "rtp100001";
  for(int i=0;i!=32;++i){
      if(i<id.size()){
          msg_header.sender_id[i] = id[i];
      }
  }
  QDateTime now = QDateTime::currentDateTime();
  string time_str = now.toString(Qt::RFC2822Date).toStdString();
  for(int i=0;i!=32;++i){
      if(i<time_str.size()){
          msg_header.send_dts[i] = time_str[i];
      }
  }
  osstr.write((char*)&msg_header,sizeof(MsgHeader));
  osstr.write((char*)msg.toStdString().data(),msg.toStdString().size());

  QByteArray byte_data(osstr.str().data(),osstr.str().length());
  //QByteArray byte_data = "hello world";
  int send_size = udp_socket_->writeDatagram(byte_data,QHostAddress("224.0.0.1"),2000);
  qDebug()<<"send ip address = "<<bind_ip_address_<<Qt::endl;
  qDebug()<<"send port = "<<5555<<Qt::endl;
  qDebug()<<"send size = "<<send_size<<Qt::endl;
}


void UdpClient::onReceiveMsg(){
  while(udp_socket_->hasPendingDatagrams()){
      QNetworkDatagram data = udp_socket_->receiveDatagram();
      parse_buffer(data);
    };

}

void UdpClient::parse_buffer(QNetworkDatagram data){
  qDebug()<<"receive data = "<<QString(data.data().toStdString().c_str())<<Qt::endl;
  receive_buffer_.append(data.data());
}

bool UdpClient::read_msg_header(MsgHeader& header){

  return true;
}

int UdpClient::read_msg_data(unsigned char* data){

  return 0;
}
