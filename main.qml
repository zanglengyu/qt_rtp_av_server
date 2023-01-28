import QtQuick 2.15
import QtQuick.Window 2.15

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("UdpServer")

    ChatDialog{
        id:chatDialog
        anchors.fill : parent
        width:parent.width
        height:parent.height
    }
}
