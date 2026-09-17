import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import CoastWarn

ApplicationWindow {
    id: window

    width: 400
    height: 700
    minimumWidth: 200
    minimumHeight: 250
    visible: true
    title: qsTr("Coast Warn")
    background: Rectangle {
        //color: "#001827"
        color: calculateBackColor(AppController.distance)
    }

    /*Text {
        id: lblTitle

        anchors {
            top: parent.top
            topMargin: 10
            left: parent.left
            leftMargin: 10
            right: parent.right
            rightMargin: 10
        }

        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 30
        color: "#f7fafa"
        text: "Coast Warn"
    }*/

    Text {
        id: lblDistanceSubTitle
        /*anchors {
            bottom: lblDistance.top
            bottomMargin: (lblDistance.height*lblScale.yScale)/2 + 20
            left: parent.left
            right: parent.right
        }*/
        anchors {
            top: parent.top
            topMargin: 10
            left: parent.left
            leftMargin: 10
            right: parent.right
            rightMargin: 10
        }

        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 30
        color: "#dde8e8"
        text: qsTr("Distance to coast")
        visible: lblDistance.visible
    }

    Text {
        id: lblInformation

        anchors {
            top: lblTitle.bottom
            topMargin: 10
            left: parent.left
            leftMargin: 10
            right: parent.right
            rightMargin: 10
        }

        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 20
        font.italic: true
    }

    Text {
        id: lblDistance
        anchors {
            left: parent.left
            leftMargin: 20
            right: parent.right
            rightMargin: 20
            verticalCenter: parent.verticalCenter
        }

        property int targetWidth: parent.width*0.8

        fontSizeMode: Text.Fit
        text: AppController.distance
        color: "white"
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter

        transform: Scale {
            id: lblScale
            origin.x: lblDistance.width / 2
            origin.y: lblDistance.height / 2
            xScale: lblDistance.contentWidth > 0
                    ? lblDistance.targetWidth / lblDistance.contentWidth
                    : 1
            yScale: xScale
        }

        visible: text !== "0"
    }

    Text {
        id: lblUnit

        anchors {
            top: lblDistance.bottom
            topMargin: (lblDistance.height*lblScale.yScale)/4 + 20
            left: parent.left
            right: parent.right
        }

        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 30
        color: "#dde8e8"
        text: qsTr("meters")
        visible: lblDistance.visible
    }

    Text {
        id: lblMaxSpeed

        anchors {
            bottom: parent.bottom
            bottomMargin: 20
            left: parent.left
            leftMargin: 10
            right: parent.right
            rightMargin: 10
        }

        horizontalAlignment: Text.AlignHCenter
        text: qsTr("Maximum speed: %1 %2").arg(AppController.maxSpeed === -1 ? "no limit" : AppController.maxSpeed).arg(AppController.maxSpeed === -1 ? "" : "kn" )
        color: "#fcfcfc"
        font.pixelSize: 20
        visible: ShomCoastDownloader.ready
    }

    Connections {
        target: AppController

        function onError(errorString) {
            lblInformation.text = errorString
            lblInformation.visible = true
        }
    }

    Connections {
        target: ShomCoastDownloader

        function onError(errorString) {
            lblInformation.text = errorString
            lblInformation.visible = true
            lblInformation.color = "red"
        }

        function onDownloading() {
            lblInformation.text = qsTr("Downloading coast data")
            lblInformation.visible = true
            lblInformation.color = "#fcfcfc"
        }

        function onDownloadFinished() {
            lblInformation.visible = false
            lblInformation.color = "#fcfcfc"
        }
    }

    function calculateBackColor(distance) {
        if(distance === 0)
            return "transparent"

        if(distance < 300) {
            //return "#E57373"
            return "#d96c6c"
        } else if(distance < 500) {
            //return "#E6B85C"
            return "#d6aa55"
        } else {
            //return "#5BAFA3"
            return "#4f9d95"
        }
    }

}
