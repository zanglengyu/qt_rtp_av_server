import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 1.4

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("MicphoneRtpTran")

    Rectangle {
        id: select_audio_input
        color: 'white'
        width: parent.width / 2 - 20
        height: 50
        anchors.top: parent.top
        anchors.left: parent.left

        anchors.leftMargin: 10
        anchors.topMargin: 10
        RowLayout {
            anchors.fill: parent
            spacing: 10
            Text {
                id: text_mic
                text: "麦克风："
                Layout.margins: 10
            }

            ComboBox {
                id: devices_box
                Layout.minimumWidth: 100
                Layout.preferredWidth: parent.width - text_mic.width - 20
                Layout.margins: 2
                currentIndex: 0
                model: audio_capture.get_audio_device_names()
                onCurrentIndexChanged: {
                    console.log("selete index = " + currentIndex)
                    audio_capture.set_select_device_index(currentIndex)
                }
            }
            Button {
                id: start_btn
                text: "准备喊话"
                onClicked: {
                    udp_sip.send_ready_msg()
                    start_btn.enabled = false
                }
            }
            Button {
                id: stop_btn
                text: "停止喊话"
                onClicked: {
                    start_btn.enabled = true
                    audio_capture.stop_capture()
                }
            }
        }
    }
}
