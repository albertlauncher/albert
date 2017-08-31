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
        onDoubleClicked:  (mouse.modifiers===Qt.NoModifier) ? root.activate() : root.activate(-mouse.modifiers)
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
            text: (listItem.ListView.isCurrentItem && root.state==="fallback") ? itemFallbackRole : itemToolTipRole
            textFormat: Text.PlainText
            elide: Text.ElideRight
            color: listItem.ListView.isCurrentItem ? listItem.highlightColor : listItem.textColor
            font.family: listItem.fontName
            font.pixelSize: listItem.descriptionSize
            Behavior on color { ColorAnimation{ duration: animationDuration } }
            Behavior on text {
                SequentialAnimation {
                    NumberAnimation { target: subTextId; property: "opacity"; from:1; to: 0; duration: animationDuration/2 }
                    PropertyAction  { }
                    NumberAnimation { target: subTextId; property: "opacity"; from:0; to: 1; duration: animationDuration/2 }
                }
            }
        }
    }  // listItemTextArea (Column)


    /*
     * The function to activate an item
     * Currently work as follows:
     * action is undefined -> default action
     * action 0<= are the alternative actions
     * action 0> activation while modifier is pressed (-action is the number of the modifier)
     *   currently only Meta is supported
     */
    function activate(/*optional*/ action){
        if (typeof action === 'undefined')
            itemActionRole = 0
        else
            if (action < 0 && -action==Qt.MetaModifier)
                itemFallbackRole = 0
            else
                itemAltActionsRole = action
    }

    function actionsList() {
        return itemAltActionsRole
    }

}  // listItem (MouseArea)
