import QtQuick 2.5
import QtGraphicalEffects 1.0

Item {

    id: listItem

    property int iconSize
    property int spacing
    property int textSize
    property int descriptionSize
    property color textColor
    property color highlightColor
    property string fontName
    property int animationDuration: 150

    width: parent.width
    height: Math.max(listItemIcon.height, listItemTextArea.height)

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
        width: listItem.iconSize
        height: listItem.iconSize
        sourceSize.width: listItem.iconSize*2
        sourceSize.height: listItem.iconSize*2
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
            leftMargin: listItem.spacing
            right: parent.right
            verticalCenter: parent.verticalCenter
        }
        Text {
            id: textId
            width: parent.width
            text: itemTextRole
            textFormat: Text.PlainText
            elide: Text.ElideRight
            color: listItem.ListView.isCurrentItem ? listItem.highlightColor : listItem.textColor
            font.family: listItem.fontName
            font.pixelSize: listItem.textSize
            Behavior on color { ColorAnimation{ duration: animationDuration } }
         }
        Text {
            id: subTextId
            width: parent.width
            text: itemToolTipRole
            textFormat: Text.PlainText
            elide: Text.ElideRight
            color: listItem.ListView.isCurrentItem ? listItem.highlightColor : listItem.textColor
            font.family: listItem.fontName
            font.pixelSize: listItem.descriptionSize
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
