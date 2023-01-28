import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 1.4
import QtMultimedia 5.15

Rectangle {
    color:'lightblue'

    Rectangle{
        id:select_audio_input
        color:'white'
        width: parent.width/2-20
        height:50
        anchors.top: parent.top
        anchors.left: parent.left

        anchors.leftMargin: 10
        anchors.topMargin:  10
        RowLayout{
            anchors.fill: parent
            spacing:-20
            Text{
                id:text_mic
                text:"麦克风："
                Layout.margins: 10
            }

            ComboBox {
                id:devices_box;
                Layout.minimumWidth: 100
                Layout.preferredWidth:  parent.width -text_mic.width-20
                Layout.margins: 2
                currentIndex: 0
                model: audio_capture.get_audio_device_names()
                onCurrentIndexChanged:{
                    console.log("selete index = "+currentIndex);
                    audio_capture.set_select_device_index(currentIndex);
                }
            }
        }

    }

    Rectangle{
        id:select_camera
        color:'white'
        width: parent.width/2-20
        height:50
        anchors.top: parent.top
        anchors.right: parent.right

        anchors.rightMargin:  10
        anchors.topMargin:  10
        RowLayout{
            anchors.fill: parent
            spacing:-20
            Text{
                id:text_camera
                text:"摄像头："
                Layout.margins: 10
            }

            ComboBox {
                id:camera_devices_box;
                Layout.minimumWidth: 100
                Layout.preferredWidth:  parent.width -text_camera.width-20
                Layout.margins: 2
                currentIndex: 0
                model: camera_capture.get_cameras()
                onCurrentIndexChanged:{
                    console.log("selete index = "+currentIndex);


                }
            }
        }

    }


    Camera {
        id: camera
        imageProcessing.whiteBalanceMode: CameraImageProcessing.WhiteBalanceFlash
        exposure {
            exposureCompensation: -1.0
            exposureMode: Camera.ExposurePortrait
        }
        flash.mode: Camera.FlashRedEyeReduction
    }

    Rectangle{
        width:parent.width/2-20
        height:parent.height - select_audio_input.height - text_input_item.height - 40
        anchors.left: parent.left
        anchors.top:select_audio_input.bottom
        anchors.topMargin: 10
        anchors.bottomMargin: 10
        anchors.leftMargin: 10
        color:'white'

        VideoOutput {
            source: camera
            anchors.fill: parent
            fillMode: VideoOutput.Stretch
        }
    }

    Rectangle{
        width:parent.width/2-20
        height:parent.height - select_audio_input.height - text_input_item.height - 40
        anchors.right: parent.right
        anchors.top:select_audio_input.bottom
        anchors.topMargin: 10
        anchors.bottomMargin: 10
        anchors.rightMargin: 10
        color:'white'

        Image {
            id: orther_image
            anchors.fill: parent
        }
    }


    Timer{
        interval: 40
        repeat:true
        running:true
        onTriggered: {
            if(camera.imageCapture.ready){
                camera.imageCapture.capture();
            }
        }
    }
    Rectangle{
        id:text_input_item
        color:'white'
        width: parent.width
        height:80
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 10
        anchors.rightMargin:  130
        anchors.bottomMargin:  10
        TextInput{
            id:input_item
            anchors.fill : parent
            color:'black'
            wrapMode:TextInput.WordWrap
        }
    }

    Rectangle{
        id:send_btn_item
        color:'white'
        width: parent.width - text_input_item.width-30
        height:80
        anchors.bottom: parent.bottom
        anchors.left: text_input_item.right
        anchors.right: parent.right
        anchors.leftMargin: 10
        anchors.rightMargin:  10
        anchors.bottomMargin:  10
        ToolButton{
            id:send_btn
            anchors.fill : parent
            text: "发送"
            onClicked: {
                udp_connect.send_msg(input_item.text);
            }
        }
    }

    Component.onCompleted: {
        console.log("init ChatDialog view")
        camera_capture.set_camera(camera);
    }


}
