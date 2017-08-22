import QtQuick 2.0

TextInput {

    function clearLine() {
        text = ""
    }

    function pushTextToHistory() {
        history.add(text)
        clearLine()
        clearIterator()
    }

    function clearIterator() {
        return history.resetIterator()
    }

    function nextIteration() {
        var entry = history.next()
        if ( entry.length !== 0 ){
            textChanged.disconnect(clearIterator)
            text = entry
            textChanged.connect(clearIterator)
        }
    }

    function prevIteration() {
        var entry = history.prev()
        if (entry.length!==0) {
            textChanged.disconnect(clearIterator)
            text = entry
            textChanged.connect(clearIterator)
        }
    }

    Component.onCompleted: {
        textChanged.connect(clearIterator)
    }

}
