import QtQuick 2.1
import QtQuick.Controls 1.0
import QtGraphicalEffects 1.0
import "presets.js" as Presets

FocusScope {
    // ▼ Setfle properties of this style ▼
    property color border_color
    property color background_color
    property color foreground_color
    property color highlight_color
    property int settingsbutton_size
    property int input_fontsize
    property int icon_size
    property int item_title_fontsize
    property int item_description_fontsize
    property int max_items
    property int space
    property int window_width
    // ▲ Settable properties of this style ▲
    property int currentModifiers
    property string font_name: fontLoader.name
    FontLoader {
        id: fontLoader
        source: "fonts/Roboto-Thin.ttf"
    }

    id: root
    width: frame.width
    height: frame.height
    focus: true

    Rectangle {
        id: frame
        width: window_width
        height: content.height+2*content.anchors.margins
        radius: space*2+4
        color: background_color
        border.color: border_color
        border.width: space

        Column {

            id: content
            anchors { top: parent.top; left: parent.left; right: parent.right; margins: space*2 }
            spacing: space

            HistoryTextInput {
                id: historyTextInput
                anchors {
                    left: parent.left;
                    right: parent.right;
                    rightMargin: 4;
                    leftMargin: 4;
                }
                clip: true
                color: foreground_color
                focus: true
                font.pixelSize: input_fontsize
                font.family: font_name
                selectByMouse: true
                selectedTextColor: background_color
                selectionColor: highlight_color
                Keys.forwardTo: [root, resultsList]
                cursorDelegate : Item {
                    id: cursor
                    Rectangle { width: 1
                        height: parent.height
                        color: foreground_color
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
                itemCount: max_items
                spacing: space
                delegate: Component { ItemViewDelegate{ } }
                Keys.onEnterPressed: activate()
                Keys.onReturnPressed: activate()
            }  // resultsList (ListView)

            DesktopListView {
                id: actionsListView
                width: parent.width
                model: ListModel { id: actionsModel }
                itemCount: actionsModel.count
                delegate: Text {
                    horizontalAlignment: Text.AlignHCenter
                    width: parent.width
                    text: name
                    font.family: font_name
                    elide: Text.ElideRight
                    font.pixelSize: (item_description_fontsize+item_title_fontsize)/2
                    color: ListView.isCurrentItem ? highlight_color : foreground_color
                    Behavior on color { ColorAnimation{ duration: 100 } }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: actionsListView.currentIndex = index
                        onDoubleClicked: resultsList.currentItem.activate(currentIndex)
                    }
                }
                visible: false
                Keys.onEnterPressed: activate(currentIndex)
                Keys.onReturnPressed: activate(currentIndex)
            }  // actionsListView (ListView)
        }  // content (Column)


        SettingsButton {
            id: settingsButton
            size: settingsbutton_size
            color: background_color
            hoverColor: border_color
            pressedColor: highlight_color
            onLeftClicked: settingsWidgetRequested()
            onRightClicked: menu.popup()
            anchors {
                top: parent.top
                right: parent.right
                topMargin: 2*space
                rightMargin: 2*space
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

    Component.onCompleted: setPreset("Bright")


    // ▼ ▼ ▼ ▼ ▼ DO NOT CHANGE THIS UNLESS YOU KNOW WHAT YOU ARE DOING ▼ ▼ ▼ ▼ ▼

    /*
     * Currently the interface with the program logic comprises the following:
     *
     * Context property 'resultsModel'
     * Context property 'history'
     * Listeners on signal: 'inputChanged'
     * External invokation of 'onMainWindowHidden' (Focus out)
     * External invokation of availablePresets
     * External invokation of setPreset
     * External invokation of availableProperties
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

    function availableProperties() { return Presets.settableProperties }
    function availablePresets() { return Object.keys(Presets.presets) }
    function setPreset(p) {
        var preset = Presets.presets[p]
        input_fontsize = preset.input_fontsize
        item_title_fontsize = preset.item_title_fontsize
        item_description_fontsize = preset.item_description_fontsize
        icon_size = preset.icon_size
        max_items = preset.max_items
        space = preset.space
        settingsbutton_size = preset.settingsbutton_size
        window_width = preset.window_width
        background_color = preset.background_color
        foreground_color = preset.foreground_color
        highlight_color = preset.highlight_color
        border_color = preset.border_color
    }
}
