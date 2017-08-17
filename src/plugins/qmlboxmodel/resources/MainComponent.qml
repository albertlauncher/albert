import QtQuick 2.0
import QtGraphicalEffects 1.0

FocusScope {
    // ▼▲ Settable properties of this style ▲▼
    property color border_color
    property color background_color
    property color foreground_color
    property color inputline_color
    property color highlight_color
    property int settingsbutton_size
    property int input_fontsize
    property int item_height
    property int icon_size
    property int item_title_fontsize
    property int item_description_fontsize
    property int max_items
    property int space
    property int window_width
    // ▲▼ Settable properties of this style ▼▲
    property int currentModifiers

    width: frame.width
    height: frame.height
    focus: true

    Rectangle {
        id: frame
        width: window_width
        height: content.height+2*content.anchors.margins
        radius: space*2
        color: background_color
        border.color: border_color
        border.width: space

        Column {
            id: content
            anchors { top: parent.top; left: parent.left; right: parent.right; margins: space*2}
            spacing: space

            Rectangle {
                id: historyTextInputFrame
                width: parent.width
                height: historyTextInput.height
                color: inputline_color
                radius: space/2

                HistoryTextInput {
                    id: historyTextInput
                    clip: true
                    anchors { left: parent.left; right: parent.right; rightMargin: 4; leftMargin: 4; topMargin:2; bottomMargin:2 }
                    font.pixelSize: input_fontsize
                    color: foreground_color
                    selectedTextColor: background_color
                    selectionColor: highlight_color
                    selectByMouse: true
                    focus: true
                    Keys.forwardTo: resultsList
                    onTextChanged: { inputChanged(text) }
                    Connections {
                        target: resultsList
                        onItemActivated: {
                            historyTextInput.pushTextToHistory()
                            historyTextInput.clearLine()
                        }
                    }
                    cursorDelegate : Component {
                        Item {
                            id: cursor
                            Rectangle {
                                y: 2; width: 1
                                height: parent.height-4
                                color: highlight_color
                            }
                            SequentialAnimation on opacity {
                                loops: Animation.Infinite;
                                NumberAnimation { to: 0.2; duration: 1000; easing.type: Easing.InOutCubic}
                                NumberAnimation { to: 1; duration: 1000; easing.type: Easing.InOutCubic}
                            }
                        }
                    }
                    Keys.onPressed: {
                        event.accepted = true
                        if (event.key===Qt.Key_Tab){  // For selected or first item, if list is not empty, show details
                            if (resultsList.count > 0) {
                                if (resultsList.currentIndex < 0)
                                    resultsList.currentIndex=0
                                resultsList.state = (resultsList.state==="detailsView") ? "" : "detailsView";
                            } else event.accepted = false
                        } else if (event.key===Qt.Key_Comma && event.modifiers===Qt.AltModifier) {
                            settingsWidgetRequested()
                        } else
                            event.accepted = false
                    }
                } // historyTextInput

                Item {
                    width: settingsbutton_size
                    height: settingsbutton_size
                    anchors { top: parent.top;topMargin: 3;right: parent.right;rightMargin: 3 }
                    Rectangle {
                        id: gearcolor
                        width: settingsbutton_size
                        height: settingsbutton_size
                        color: background_color
                        visible: false
                    }
                    Image {
                        id: gearmask
                        source: "gear.svg"
                        width: settingsbutton_size
                        height: settingsbutton_size
                        sourceSize{width: settingsbutton_size; height: settingsbutton_size}
                        smooth: true
                        visible: false
                    }
                    OpacityMask {
                        id: gear
                        anchors.fill: parent
                        source: gearcolor
                        maskSource: gearmask
                        RotationAnimation on rotation {
                            duration : 15000
                            easing.type: Easing.Linear
                            loops: Animation.Infinite
                            from: 0
                            to: 360
                        }
                    }
                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: settingsWidgetRequested()
                        onEntered: gearcolor.color=foreground_color
                        onExited: gearcolor.color=background_color
                        onPressed: gearcolor.color=highlight_color
                        onReleased: gearcolor.color=background_color
                    }  // gear
                }  // settingsbutton
            }  // historyTextInputFrame

            ListView {
                id: resultsList
                width: parent.width
                height: Math.min(max_items, count)*(item_height+spacing)-spacing
                model: resultsModel
                snapMode: ListView.SnapToItem
                clip: true
                highlightMoveDuration : 250
                highlightMoveVelocity : 1000
                spacing: space
                signal itemActivated()

                // Reset the state if the list content changes
                Connections {target: resultsModel; onModelReset: {resultsList.state="";resultsList.currentIndex=-1} }

                // Key handling
                Keys.onPressed: {
                    event.accepted = true
                    if (event.key===Qt.Key_Up)  // Move up in list, or next from history if none selected or ctrl is hold
                        (currentIndex===-1 || event.modifiers===Qt.ControlModifier) ? historyTextInput.nextIteration() : decrementCurrentIndex()
                    else if (event.key===Qt.Key_Down)  // Move down in list, or prev from history if ctrl is hold
                        (event.modifiers===Qt.ControlModifier) ? historyTextInput.prevIteration() : incrementCurrentIndex()
                    else if (event.key===Qt.Key_PageUp)  // Move page up
                        currentIndex = (currentIndex - max_items < 0) ? 0 : currentIndex - max_items
                    else if (event.key===Qt.Key_PageDown)  // Move page down
                        currentIndex = (currentIndex + max_items > count) ? count-1 : currentIndex + max_items
                    else if ((event.key===Qt.Key_Enter || event.key===Qt.Key_Return) && count>0){  // Activate on accepted
                        if (currentIndex===-1)
                            currentIndex=0;
                        currentItem.activate()
                    } else if (event.key===Qt.Key_Shift || event.key===Qt.Key_Control || event.key===Qt.Key_Meta || event.key===Qt.Key_Alt) {  // Change text on mod
                        if (event.key===Qt.Key_Alt && event.modifiers===Qt.AltModifier)  // Show fallbacks on alt
                            inputChanged(historyTextInput.text)
                        currentModifiers=event.modifiers
                    } else
                        event.accepted = false
                }
                Keys.onReleased: {
                    if (event.key===Qt.Key_Shift || event.key===Qt.Key_Control || event.key===Qt.Key_Meta || event.key===Qt.Key_Alt) {  // Change text on mod
                        if (event.key===Qt.Key_Alt && event.modifiers!==Qt.AltModifier) // Show fallbacks on alt
                            inputChanged(historyTextInput.text)
                        currentModifiers=event.modifiers
                    }
                }

                // Definition of the actions view
                states : State {
                    name: "detailsView"
                    PropertyChanges { target: resultsList; height: resultsList.currentItem.height }
                    PropertyChanges { target: resultsList; explicit: true; contentY: currentItem.y }
                    PropertyChanges { target: resultsList; interactive: false }
                }

                // Make state change visually smooth by fading list
                Behavior on state {
                    SequentialAnimation {
                        PropertyAction { target: resultsList; property: "opacity"; value: 0 }
                        PropertyAction { }
                        NumberAnimation { target: resultsList; property: "opacity"; to: 1 }
                    }
                }

//                transitions: Transition {
//                    SequentialAnimation {
//                        PropertyAction { target: resultsList; properties: "interactive" }
//                        NumberAnimation { target: resultsList; property: "opacity"; to: 0 }
//                        PropertyAction { target: resultsList.currentItem; properties: "state" }
//                        PropertyAction { target: resultsList; property: "contentY" }
//                        PropertyAction { target: resultsList; property: "height" }
//                        NumberAnimation { target: resultsList; property: "opacity"; to: 1 }
//                    }
//                }

                delegate: Component {
                    id: listItemDelegate
                    Item {
                        id: listItem
                        width: parent.width
                        height: listItemColumn.height

                        // Definition of the actions view
                        states : State {
                            name: "detailsView"
                            when: resultsList.state==="detailsView" && listItem.ListView.isCurrentItem
                            PropertyChanges { target: actionsListView; visible: true  }
                            PropertyChanges { target: listItemTopRow; height: icon_size*2 }
                            PropertyChanges { target: listItemIcon; width: icon_size*2; height: icon_size*2 }
                            PropertyChanges { target: textId; font.pixelSize:item_title_fontsize*1.2; horizontalAlignment: Text.AlignHCenter}
                            PropertyChanges { target: subTextId; font.pixelSize: item_description_fontsize*1.2; horizontalAlignment: Text.AlignHCenter; color: foreground_color; wrapMode:Text.Wrap}
                            PropertyChanges { target: mouseArea; enabled: false }
                            PropertyChanges { target: historyTextInput; Keys.forwardTo: actionsListView }
                            StateChangeScript{
                                id: actionLoaderScript
                                script: {
                                    if (actionsModel.count===0){
                                        var actionTexts = actionsRole;
                                        for (var i = 0; i < actionTexts.length; i++)
                                            actionsModel.append({"name": actionTexts[i]});
                                    }
                                }
                            }
                        } // states

                        function subtextForModifier(){ // TODO
                            return resultsModel.data(resultsModel.index(index, 0), 1000)
                        }

                        // This function activates the "action" of item
                        function activate(/*optional*/ action){

                            // Default if none given
                            action = (typeof action === 'undefined') ? -1 : action;

                            // Emit a signal to inform other components about the activation
                            listItem.ListView.view.itemActivated()

                            /*
                             *  For this use the setData funtion is abused, to be able to use the
                             *  interface of QAbstractItemModel.
                             *  Currently a positive value "action" activates the action 'a' in the set of
                             *  actions 'A' returned by actionsRole. If "action" is not in the range of
                             *  'A' the default role is activated. Negative values are used for
                             *  actions that should depend on the modifiers pressed. This way the backend
                             *  extensions can decide which action should be executed for the modifiers.
                             *  Currently this mapping is used:
                             *  "NoModifier"="-1"
                             *  "Alt"="-2"
                             *  "Meta"="-3"
                             *  "Ctrl"="-4"
                             */
                            resultsModel.setData(resultsModel.index(index, 0), action, activateRole)
                        }

                        MouseArea {
                            id: mouseArea
                            anchors.fill: parent
                            onClicked: resultsList.currentIndex = index
                            onDoubleClicked: { listItem.activate() }
                        } // mouseArea (MouseArea)

                        Column{
                            id: listItemColumn
                            width: parent.width
                            Item {
                                id: listItemTopRow
                                width: parent.width
                                height: Math.max(listItemIcon.height, listItemTextArea.height)

                                Image {
                                    id: listItemIcon
                                    asynchronous: true
                                    source: decoration
                                    width: icon_size
                                    height: icon_size
                                    sourceSize.width: icon_size*2
                                    sourceSize.height: icon_size*2
                                    cache: true
                                    fillMode: Image.PreserveAspectFit
                                    visible: false
                                } // listItemIcon
                                InnerShadow {
                                    id: innerShadow
                                    width: source.width
                                    height: source.height
                                    horizontalOffset: listItem.ListView.isCurrentItem?0:4
                                    verticalOffset: listItem.ListView.isCurrentItem?0:4
                                    radius: listItem.ListView.isCurrentItem?0:4
                                    samples: 8
                                    color: "#80000000"
                                    visible: false
                                    Behavior on verticalOffset { NumberAnimation{ } }
                                    Behavior on horizontalOffset { NumberAnimation{ } }
                                    Behavior on radius { NumberAnimation{ } }
                                    source: listItemIcon
                                }
                                Desaturate {
                                    id: desaturate
                                    width: source.width
                                    height: source.height
                                    desaturation: listItem.ListView.isCurrentItem?0:0.9
                                    Behavior on desaturation { NumberAnimation{ } }
                                    source: innerShadow
                                }

                                Column{
                                    id: listItemTextArea
                                    anchors {
                                        left: listItemIcon.right
                                        leftMargin: space
                                        right: parent.right
                                        verticalCenter: listItemIcon.verticalCenter
                                    }

                                    Text {
                                        id: textId
                                        width: parent.width
                                        text: display
                                        elide: Text.ElideRight
                                        color: listItem.ListView.isCurrentItem?highlight_color:foreground_color
                                        font.pixelSize: item_title_fontsize
                                        Behavior on color { ColorAnimation{ } }
                                    }

                                    Text {
                                        id: subTextId
                                        width: parent.width
//                                        text: (listItem.ListView.isCurrentItem)?subtextForModifier():subTextRole
                                        text: toolTip
                                        elide: Text.ElideRight
                                        color: listItem.ListView.isCurrentItem?highlight_color:foreground_color
                                        font.pixelSize: item_description_fontsize
                                        // Make selection color change smooth
                                        Behavior on color { ColorAnimation{ } }
                                        // Make text changes smooth (actions, etc...)
                                        Behavior on text {
                                            SequentialAnimation {
//                                                NumberAnimation { target: subTextId; property: "opacity"; to: 0; duration: 100 }
                                                PropertyAction  { }
                                                NumberAnimation { target: subTextId; property: "opacity"; to: 1; duration: 250 }
                                            }
                                        }
                                    }
                                }  // Column
                            }  // Item

                            ListView {
                                id: actionsListView
                                width: parent.width
                                height: count?contentHeight:0 // Avoid warning
                                keyNavigationWraps: true
                                boundsBehavior: Flickable.StopAtBounds
                                model: ListModel { id:actionsModel }
                                clip: true
                                spacing: space
                                delegate: Text {
                                    horizontalAlignment: Text.AlignHCenter
                                    width: parent.width
                                    text: name
                                    elide: Text.ElideRight
                                    font.pixelSize: (item_description_fontsize+item_title_fontsize)/2
                                    color: ListView.isCurrentItem?highlight_color:foreground_color
                                    Behavior on color { ColorAnimation{ } }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: actionsListView.currentIndex = index
                                        onDoubleClicked: { listItem.activate(index)}
                                    } // mouseArea (MouseArea)
                                }
                                visible: false
                                Keys.onEnterPressed:  listItem.activate(currentIndex)
                                Keys.onReturnPressed: listItem.activate(currentIndex)
                            }  // actionsListView
                        }  // Column
                    }  // listItem (Item)
                }  // listItemDelegate (Component)
            }  // resultsList (ListView)
        }  // content (Column)
    }  // frame (Rectangle)

    Component.onCompleted: setPreset("Dark")

    function availableProperties() {
        return ["background_color","foreground_color","highlight_color","inputline_color",
                "border_color","input_fontsize","item_height","item_title_fontsize",
                "item_description_fontsize","icon_size","max_items","space","settingsbutton_size",
                "window_width"];
    }

    function availablePresets() {
        return ["Dark", "DarkOrange", "DarkMagenta", "DarkMint",
                "DarkGreen", "DarkBlue", "DarkViolet",
                "Bright", "BrightOrange", "BrightMagenta", "BrightMint",
                "BrightGreen", "BrightBlue", "BrightViolet"];
    }

    function setPreset(p) {
        input_fontsize = 36
        item_title_fontsize = 26
        item_description_fontsize = 12
        item_height = 48
        icon_size = 48
        max_items = 5
        space = 6
        settingsbutton_size = 14
        window_width = 640
        switch (p) {
        case "BrightOrange":
            setPreset("Bright")
            highlight_color= "#E07000"
            border_color= "#80FF8000"
            break;
        case "BrightMagenta":
            setPreset("Bright")
            highlight_color= "#E00070"
            border_color= "#80FF0080"
            break;
        case "BrightMint":
            setPreset("Bright")
            highlight_color= "#00c060"
            border_color= "#8000FF80"
            break;
        case "BrightGreen":
            setPreset("Bright")
            highlight_color= "#60c000"
            border_color= "#8080FF00"
            break;
        case "BrightBlue":
            setPreset("Bright")
            highlight_color= "#0070E0"
            border_color= "#800080FF"
            break;
        case "BrightViolet":
            setPreset("Bright")
            highlight_color= "#7000E0"
            border_color= "#808000FF"
            break;
        case "DarkOrange":
            setPreset("Dark")
            highlight_color= "#FF9020"
            border_color= "#80FF8000"
            break;
        case "DarkMagenta":
            setPreset("Dark")
            highlight_color= "#FF2090"
            border_color= "#80FF0080"
            break;
        case "DarkMint":
            setPreset("Dark")
            highlight_color= "#20FF90"
            border_color= "#8000FF80"
            break;
        case "DarkGreen":
            setPreset("Dark")
            highlight_color= "#90FF20"
            border_color= "#8080FF00"
            break;
        case "DarkBlue":
            setPreset("Dark")
            highlight_color= "#2090FF"
            border_color= "#800080FF"
            break;
        case "DarkViolet":
            setPreset("Dark")
            highlight_color= "#A040FF"
            border_color= "#808000FF"
            break;
        case "Bright":
            background_color = "#FFFFFF"
            foreground_color = "#808080"
            inputline_color = "#D0D0D0"
            highlight_color = "#000000"
            border_color = "#80808080"
            break;
        case "Dark":
        default:
            background_color = "#404040"
            foreground_color = "#808080"
            inputline_color = "#202020"
            highlight_color = "#E0E0E0"
            border_color = "#80808080"
            break;
        }
    }


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
    property string interfaceVersion: "0.1" // Will not change until beta

    signal inputChanged(string text)
    signal settingsWidgetRequested()

    function onMainWindowHidden() {
        resultsList.state=""
        historyTextInput.clearLine();
    }
}
