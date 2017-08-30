import QtQuick 2.0
import QtQuick.Controls 2.1
import QtGraphicalEffects 1.0


Item {

    property color color
    property color hoverColor
    property color pressedColor
    property int size
    signal rightClicked()
    signal leftClicked()

    width: size
    height: size

    Rectangle {
        id: gearcolor
        anchors.fill: parent
        color: color
        Behavior on color {
           ColorAnimation {
               duration: 1500
               easing.type: Easing.OutCubic
           }
       }
        visible: false
    }
    Image {
        id: gearmask
        source: "gear.svg"
        anchors.fill: parent
        sourceSize.width: width*2
        sourceSize.height: height*2
        smooth: true
        visible: false
    }
    OpacityMask {
        id: gear
        anchors.fill: parent
        source: gearcolor
        maskSource: gearmask
        RotationAnimation on rotation {
            duration : 10000
            easing.type: Easing.Linear
            loops: Animation.Infinite
            from: 0
            to: 360
        }
    }
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: {
            if ( mouse.button === Qt.LeftButton )
                leftClicked()
            else if  ( mouse.button === Qt.RightButton )
                rightClicked()
        }
        onEntered: gearcolor.color=hoverColor
        onExited: gearcolor.color=color
        onPressed: gearcolor.color=pressedColor
        onReleased: gearcolor.color=hoverColor
    }
}
