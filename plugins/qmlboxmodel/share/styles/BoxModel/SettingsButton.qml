import QtQuick 2.5
import QtGraphicalEffects 1.0

Item {
    id: root
    property color color
    property color hoverColor
    property int size
    signal rightClicked()
    signal leftClicked()

    width: size
    height: size

    Rectangle {
        id: gearcolor
        anchors.fill: gearmask
        color: root.color
        Behavior on color { ColorAnimation { duration: 3000; easing.type: Easing.OutExpo } }
        visible: false
    }
    Image {
        id: gearmask
        source: "gear.svg"
        anchors.fill: gear
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
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: {
            if ( mouse.button === Qt.LeftButton )
                leftClicked()
            else if  ( mouse.button === Qt.RightButton )
                rightClicked()
        }
    }

    states: State {
        name: "hovered"
        when: mouseArea.containsMouse
        PropertyChanges { target: gearcolor; color: root.hoverColor }
    }

}
