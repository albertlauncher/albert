---
layout: docs
title: Using Albert
permalink: /docs/using/
---

As you would expect from a launcher, the main use case is to type a query into an input box and finally to interact with some sort of results. Actually there is not much more Albert allows you to do - launching things. No more, no less.

By pressing the hotkey the launcher is shown and the focused input line awaits a query. If you press <kbd>Esc</kbd> or the hotkey again or if the window looses focus, the window hides. The query entered in the input box is delegated to a set of plugins, which in turn returns items that are somehow related to the query.

While typing a query Albert instantly shows the results in a vertical list below the input box. These results can be browsed like a regular list view. With the keys <kbd>⬆</kbd> and <kbd>⬇</kbd> the selection can be moved a single item, <kbd>PgUp</kbd> and <kbd>PgDn</kbd> moves the selection by the amount of visible items, <kbd>Home</kbd> and <kbd>End</kbd> moves the selection to the first and last item respectively. If you found the item you were looking for, you can run its associated action by activating the item with <kbd>⏎</kbd>, <kbd>Enter</kbd> or a mouse click.


If a query yielded no results, several fallback items are provided. Fallback items are items that can handle every query, e.g. an item that opens an external search engine like google in your browser or your local dictionary application.

Items can have multiple alternative actions. By pressing <kbd>Tab</kbd> a list containing the alternative actions pops up. As long as this list is visible the arrow up and down keys move the cursor in this list. Press <kbd>Tab</kbd> again to return to the result items; the action list disappears.

Modifiers can be used as a shortcut for the alternative actions. <kbd>Meta</kbd>, <kbd>Ctrl</kbd> or <kbd>Shift</kbd> can be hold to run the 1<sup>st</sup>, 2<sup>nd</sup> or 3<sup>rd</sup> alternative action when activating the item. Slightly different: Holding <kbd>Alt</kbd> while activating an item runs the 1<sup>st</sup> fallback item with the entered query.

Albert stores the input query when you activate an item. If the item selection is on the first item, the <kbd>⬆</kbd> is overloaded to iterate over this history. The arrow down key is reserved for moving the selection, however in combination with the <kbd>Ctrl</kbd> key the arrow keys can be used to navigate the history back and forth. The history is sorted chronologically and duplicates are removed.

In the input box a little gear is spinning. If you left click on this gear, the settings dialog will appear. Alternatively you can simply hit <kbd>Ctrl</kbd>+<kbd>,</kbd> or <kbd>Alt</kbd>+<kbd>,</kbd> to open the settings dialog. A right click opens a context menu offering some actions including the
"Quit" action. Clicking this item will quit the application. Alternatively you
can quit the application by hitting <kbd>Alt</kbd>+<kbd>F4</kbd>.

For reference the following table lists all keys you can use to control Albert:

Key  | Action
------------- | -------------
<kbd>Esc</kbd> | Hide Albert
<kbd>Tab</kbd>  | Show/hide alternative actions for the item
<kbd>Enter</kbd> | Activate the primary action of selected item
<kbd>Alt</kbd>+<kbd>Enter</kbd> | Activate the 1<sup>st</sup> fallback item with the query
<kbd>Meta/Win</kbd>+<kbd>Enter</kbd> | Activate the 1<sup>st</sup> alternative action of the selected item
<kbd>Ctrl</kbd>+<kbd>Enter</kbd> | Activate the 2<sup>nd</sup> alternative action of the selected item
<kbd>Shift</kbd>+<kbd>Enter</kbd> | Activate the 3<sup>rd</sup> alternative action of the selected item
<kbd>Ctrl</kbd>+<kbd>,</kbd><br><kbd>Alt</kbd>+<kbd>,</kbd> | Show the settings dialog
<kbd>⬆</kbd>,<kbd>⬇</kbd>,<br><kbd>PgUp</kbd>,<kbd>PgDn</kbd>,<br><kbd>Home</kbd>,<kbd>End</kbd> | Navigation. If the first item is selected, <kbd>⬆</kbd> iterates though the query history.
<kbd>Ctrl</kbd>+<kbd>⬆</kbd><br><kbd>Ctrl</kbd>+<kbd>⬇</kbd> | Navigate in query history
<kbd>Alt</kbd>+<kbd>F4</kbd> | Quit Albert



## The extensions

From the user perspective the application consists of the launcher, which is the main window, the settings window and the tray icon if you enabled it. However under the hood Albert has a plugin based architecture, which allows the user to modify or extend the functionality of the application.

> *TODO*
