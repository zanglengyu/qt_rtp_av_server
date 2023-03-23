#include "udpclient.h"

#include <QDateTime>
#include <QDebug>
#include <QNetworkInterface>
#include <sstream>

#include "audiocapture.h"
using namespace std;

UdpClient::UdpClient() : udp_socket_(nullptr) {
    // test 22
}

UdpClient::~UdpClient() {}

void UdpClient::init_udp_socket(QString ip, int port) {
    bind_ip_address_ = ip;
    bind_port_ = port;
    udp_socket_ = new QUdpSocket();
    QHostAddress hostAddress(ip);
    // bool res = udp_socket_->bind(hostAddress, port, QUdpSocket::ShareAddress);

    udp_socket_->setSocketOption(QAbstractSocket::MulticastTtlOption, 32);

    //加入组播之前，必须先绑定端口，端口为多播组统一的一个端口。
    bool res =
        udp_socket_->bind(QHostAddress::AnyIPv4, bind_port_, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

    QNetworkInterface join_interface;
    for (auto& interface : QNetworkInterface::allInterfaces()) {
        bool ok = false;
        for (const QNetworkAddressEntry& entry : interface.addressEntries())  //遍历每一个IP地址条目
        {
            qDebug() << "IP地址：" << entry.ip().toString();
            qDebug() << "子网掩码：" << entry.netmask().toString();
            qDebug() << "广播地址：" << entry.broadcast().toString();
            if (entry.ip().toString() == "192.168.105.211") {
                udp_socket_->setMulticastInterface(interface);
                join_interface = interface;
                ok = true;
                break;
            }
        }
        if (ok) {
            break;
        }
    }

    udp_socket_->joinMulticastGroup(QHostAddress(bind_ip_address_), join_interface);
    if (res) {
        qDebug() << "bind bind_port_ " << bind_port_ << "success" << endl;
    }
    connect(udp_socket_, &QUdpSocket::readyRead, this, &UdpClient::onReceiveMsg);
}

void UdpClient::send_ready_msg() {
    QString msg = QString("ready_capture,sampleRate=%1,channels=%2,sampleSzie=%3,port=%4")
                      .arg(kSampleRate)
                      .arg(kChannels)
                      .arg(kSampleSize)
                      .arg(bind_port_);
    send_msg(msg);
}

void UdpClient::send_msg(const QString& msg) {
    QByteArray byte_data(msg.toStdString().c_str(), msg.length());

    int send_size = udp_socket_->writeDatagram(byte_data, QHostAddress(bind_ip_address_), 5556);
    qDebug() << "send ip address = " << bind_ip_address_ << Qt::endl;
    qDebug() << "send port = " << bind_port_ << Qt::endl;
    qDebug() << "send size = " << send_size << Qt::endl;
}

void UdpClient::onReceiveMsg() {
    while (udp_socket_->hasPendingDatagrams()) {
        QNetworkDatagram data = udp_socket_->receiveDatagram();
        parse_buffer(data);
    };
}

void UdpClient::parse_buffer(QNetworkDatagram data) {
    QStringList msgs = QString(data.data()).split(',');

    if (msgs[0] == "ready_receive") {
        //收到远端服务器的通知
        QString address = msgs[1].split('=')[1];
        int port = msgs[2].split('=')[1].toInt();
        int ssrc = msgs[3].split('=')[1].toInt();
        AudioCapture::ins()->receive_address(address, port, ssrc);
    }
    qDebug() << "receive data = " << msgs << Qt::endl;
}

bool UdpClient::read_msg_header(MsgHeader& header) { return true; }

int UdpClient::read_msg_data(unsigned char* data) { return 0; }
