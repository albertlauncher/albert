import QtQuick 2.5
import QtGraphicalEffects 1.0

Item {
    property int animationDuration: 150

    id: listItem
    width: parent.width
    height: Math.max(listItemIcon.height, listItemTextArea.height)

    FontLoader {
        id: font
        source: "fonts/Roboto-Thin.ttf"
    }

    MouseArea {
        anchors.fill: parent
        onClicked: resultsList.currentIndex = index
        onDoubleClicked: root.activate()
    }

    Image {
        id: listItemIcon
        asynchronous: true
        source: {
            var path = itemDecorationRole
            return ( path[0] === ":" ) ? "qrc"+path : path
        }
        width: icon_size
        height: icon_size
        sourceSize.width: icon_size*2
        sourceSize.height: icon_size*2
        cache: true
        fillMode: Image.PreserveAspectFit
        visible: false
    }
    InnerShadow  {
        id: sunkenListItemIcon
        width: source.width
        height: source.height
        horizontalOffset: listItem.ListView.isCurrentItem ? 0 : 2
        verticalOffset: listItem.ListView.isCurrentItem ? 0 : 2
        radius: listItem.ListView.isCurrentItem ? 0 : 4
        samples: 8
        color: "#80000000"
        visible: false
        Behavior on verticalOffset { NumberAnimation{ duration: animationDuration } }
        Behavior on horizontalOffset { NumberAnimation{ duration: animationDuration } }
        Behavior on radius { NumberAnimation{ duration: animationDuration } }
        source: listItemIcon
    }
    Desaturate {
        id: desaturatedSunkenListItemIcon
        anchors.verticalCenter: parent.verticalCenter
        width: source.width
        height: source.height
        desaturation: listItem.ListView.isCurrentItem ? 0 : 0.25
        Behavior on desaturation { NumberAnimation{ duration: animationDuration } }
        source: sunkenListItemIcon
    }


    Column {
        id: listItemTextArea
        anchors {
            left: desaturatedSunkenListItemIcon.right
            leftMargin: space
            right: parent.right
            verticalCenter: parent.verticalCenter
        }
        Text {
            id: textId
            width: parent.width
            text: itemTextRole
            textFormat: Text.PlainText
            font.family: font_name
            elide: Text.ElideRight
            color: listItem.ListView.isCurrentItem ? highlight_color : foreground_color
            font.pixelSize: item_title_fontsize
            Behavior on color { ColorAnimation{ duration: animationDuration } }
         }
        Text {
            id: subTextId
            width: parent.width
            text: itemToolTipRole
            textFormat: Text.PlainText
            font.family: font_name
            elide: Text.ElideRight
            color: listItem.ListView.isCurrentItem ? highlight_color : foreground_color
            font.pixelSize: item_description_fontsize
            Behavior on color { ColorAnimation{ duration: animationDuration } }
            Behavior on text {
                SequentialAnimation {
                    PropertyAction  { }
                    NumberAnimation { target: subTextId; property: "opacity"; to: 1; duration: animationDuration }
                }
            }
        }
    }  // listItemTextArea (Column)


    // This function activates the "action" of item
    function activate(/*optional*/ action){
        if (typeof action === 'undefined')
            itemActionRole = 0
        else
            itemAltActionsRole = action
    }

    function actionsList() {
        return itemAltActionsRole
    }

}  // listItem (MouseArea)
