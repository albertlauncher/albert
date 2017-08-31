import QtQuick 2.5
import QtQuick.Controls 1.0
import QtGraphicalEffects 1.0
import "themes.js" as Themes

FocusScope {

    id: root
    width: frame.width+2*preferences.shadow_size
    height: frame.height+2*preferences.shadow_size
    focus: true

    Preferences {
        id: preferences
        objectName: "preferences"
    }

    FontLoader {
        source: "fonts/Roboto-Thin.ttf"
        onStatusChanged: if (loader.status === FontLoader.Ready) preferences.font_name = fontname
    }


    Rectangle {

        id: shadowArea
        color:"#80ff0000"

        Rectangle {

            id: frame
            objectName: "frame" // for C++
            x: preferences.shadow_size;
            y: preferences.shadow_size
            width: preferences.window_width
            height: content.height+2*content.anchors.margins
            radius: preferences.radius
            color: preferences.background_color
            Behavior on color { ColorAnimation { duration: 1500; easing.type: Easing.OutCubic } }
            Behavior on border.color { ColorAnimation { duration: 1500; easing.type: Easing.OutCubic } }
            border.color: preferences.border_color
            border.width: preferences.border_size

            layer.enabled: true
            layer.effect: DropShadow {
                transparentBorder: true
                verticalOffset: preferences.shadow_size/3
                radius: preferences.shadow_size
                samples: preferences.shadow_size*2
                color: preferences.shadow_color
            }

            Column {

                id: content
                anchors { top: parent.top; left: parent.left; right: parent.right; margins: preferences.spacing*2 }
                spacing: preferences.spacing

                HistoryTextInput {
                    id: historyTextInput
                    anchors {
                        left: parent.left;
                        right: parent.right;
                        rightMargin: 4;
                        leftMargin: 4;
                    }
                    clip: true
                    color: preferences.input_color
                    focus: true
                    font.pixelSize: preferences.input_fontsize
                    font.family: preferences.font_name
                    selectByMouse: true
                    selectedTextColor: preferences.background_color
                    selectionColor: preferences.selection_color
                    Keys.forwardTo: [root, resultsList]
                    cursorDelegate : Item {
                        id: cursor
                        Rectangle { width: 1
                            height: parent.height
                            color: preferences.foreground_color
                        }
                        SequentialAnimation on opacity {
                            loops: Animation.Infinite;
                            NumberAnimation { to: 0; duration: 500; easing.type: Easing.InOutExpo }
                            NumberAnimation { to: 1; duration: 500; easing.type: Easing.InOutExpo }
                        }
                    }
                } // historyTextInput

                DesktopListView {
                    id: resultsList
                    width: parent.width
                    model: resultsModel
                    itemCount: preferences.max_items
                    spacing: preferences.spacing
                    delegate: Component {
                        ItemViewDelegate{
                            iconSize: preferences.icon_size
                            spacing: preferences.spacing
                            textSize: preferences.item_title_fontsize
                            descriptionSize: preferences.item_description_fontsize
                            textColor: preferences.foreground_color
                            highlightColor: preferences.highlight_color
                            fontName: preferences.font_name
                        }
                    }
                    Keys.onEnterPressed: activate()
                    Keys.onReturnPressed: activate()
                }  // resultsList (ListView)

                DesktopListView {
                    id: actionsListView
                    width: parent.width
                    model: ListModel { id: actionsModel }
                    itemCount: actionsModel.count
                    spacing: preferences.spacing
                    delegate: Text {
                        horizontalAlignment: Text.AlignHCenter
                        width: parent.width
                        text: name
                        textFormat: Text.PlainText
                        font.family: preferences.font_name
                        elide: Text.ElideRight
                        font.pixelSize: (preferences.item_description_fontsize+preferences.item_title_fontsize)/2
                        color: ListView.isCurrentItem ? preferences.highlight_color : preferences.foreground_color
                        Behavior on color { ColorAnimation{ duration: 100 } }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: actionsListView.currentIndex = index
                            onDoubleClicked: activate(index)
                        }
                    }
                    visible: false
                    Keys.onEnterPressed: activate(currentIndex)
                    Keys.onReturnPressed: activate(currentIndex)
                }  // actionsListView (ListView)
            }  // content (Column)


            SettingsButton {
                id: settingsButton
                size: preferences.settingsbutton_size
                color: preferences.settingsbutton_color
                hoverColor: preferences.settingsbutton_hover_color
                onLeftClicked: settingsWidgetRequested()
                onRightClicked: menu.popup()
                anchors {
                    top: parent.top
                    right: parent.right
                    topMargin: 2*preferences.spacing
                    rightMargin: 2*preferences.spacing
                }

                Menu {
                    id: menu
                    MenuItem {
                        text: "Preferences"
                        shortcut: "Alt+,"
                        onTriggered: settingsWidgetRequested()
                    }
                    MenuItem {
                        text: "Quit"
                        shortcut: "Alt+F4"
                        onTriggered: Qt.quit()

                    }
                }
            }
        }  // frame (Rectangle)
    }  // shadowArea (Item)


    // Key handling
    Keys.onPressed: {
        event.accepted = true
        if ( event.key === Qt.Key_Up && state === "" && resultsList.currentIndex === -1 ) {
            historyTextInput.nextIteration()
        }
        else if ( event.key === Qt.Key_Up && event.modifiers === Qt.ControlModifier ) {
            state == ""
            historyTextInput.nextIteration()
        }
        else if ( event.key === Qt.Key_Down && event.modifiers === Qt.ControlModifier ) {
            state == ""
            historyTextInput.prevIteration()
        }
        else if ( event.key === Qt.Key_Comma && event.modifiers === Qt.AltModifier ) {
            settingsWidgetRequested()
        }
        else if ( event.key === Qt.Key_Alt && resultsList.count > 0 ) {
            if (resultsList.currentIndex === -1)
                resultsList.currentIndex = 0
            state = (state === "detailsView") ? "" : "detailsView"
        }
        else if ( event.key === Qt.Key_Tab && resultsList.count > 0 ) {
            if ( resultsList.currentIndex === -1 )
                resultsList.currentIndex = 0
            historyTextInput.text = resultsList.model.data(resultsList.model.index(resultsList.currentIndex, 0),3) // OMG magic numbers hacked in
        }
        else
            event.accepted = false
    }

    states : State {
        name: "detailsView"
        PropertyChanges { target: resultsList; enabled: false }
        PropertyChanges { target: actionsListView; visible: true  }
        PropertyChanges { target: historyTextInput; Keys.forwardTo: [root, actionsListView] }
        StateChangeScript {
            name: "actionLoaderScript"
            script: {
                actionsModel.clear()
                var actionTexts = resultsList.currentItem.actionsList();
                for ( var i = 0; i < actionTexts.length; i++ )
                    actionsModel.append({"name": actionTexts[i]});
                actionsListView.currentIndex = 0
            }
        }
    }

    Connections {
        target: historyTextInput
        onTextChanged: {
            inputChanged(historyTextInput.text)
            state=""
        }
    }

    Connections {
        target: mainWindow
        onVisibilityChanged: {
            state=""
            historyTextInput.selectAll()
            historyTextInput.clearIterator()
        }
    }

    Component.onCompleted: setTheme("Bright")


    // ▼ ▼ ▼ ▼ ▼ DO NOT CHANGE THIS UNLESS YOU KNOW WHAT YOU ARE DOING ▼ ▼ ▼ ▼ ▼

    /*
     * Currently the interface with the program logic comprises the following:
     *
     * Context property 'resultsModel'
     * Context property 'history'
     * Listeners on signal: 'inputChanged'
     * External invokation of 'onMainWindowHidden' (Focus out)
     * External invokation of availableThemes
     * External invokation of setTheme
     * External invokation of settableProperties
     * External mutations of the properties returned by availableProperties
     *
     * Canges to this interface will increment the minor version of the
     * interface version, if the new interface is a superset of the last one,
     * i.e. it is backwards compatible, otherwise the major version will be
     * incremented.
     *
     * Note: As long albert is in alpha stage the interface may break anytime.
     */
    property string interfaceVersion: "1.0-alpha" // Will not change until beta

    signal inputChanged(string text)
    signal settingsWidgetRequested()

    function activate(/*optional*/ action) {
        if ( resultsList.count > 0 ) {
            if ( resultsList.currentIndex === -1 )
                resultsList.currentIndex = 0
            resultsList.currentItem.activate(action)
            historyTextInput.pushTextToHistory()
            mainWindow.hide()
        }
    }

    function availableThemes() { return Object.keys(Themes.themes()) }
    function setTheme(themeName) {
        var themeObject = Themes.themes()[themeName]
        for (var property in themeObject)
            if (themeObject.hasOwnProperty(property))
                preferences[property] = themeObject[property]
    }
}
