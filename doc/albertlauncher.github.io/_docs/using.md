---
layout: docs
title: Using Albert
permalink: /docs/using/
---

From the user perspective the application consists of the launcher, which is the main window, the settings window and the tray icon, if you enabled it. However under the hood Albert has a plugin based architecture, which allows the user to modify or extend the functionality of the application.

The main use case, as you would expect from a launcher, is to type a query into the input box in the main window. The query is delegated to the plugins, which may or may not return result items. These matches are shown to the user who can finally interact with these icons.

If a query yielded no results several fallback items are provided. Fallback items are items that are capable of handling every query, e.g. an item that opens an external search engine like a google in your browser or your local dictionary application.

## The launcher

The main window of Albert behaves like launcher ought to be. By pressing the hotkey the launcher is shown and the focused input line awaits your query. Pressing the hotkey again, by focusing another window or hitting escape hides the window.

While typing a query Albert instantly shows the results of the queries. The results can be browsed like a regular list with the arrow up and down, page up and down, home and end keys. If you found the item you were looking for you can run its associated action by pressing enter.

Items can have multiple actions. By pressing <kbd>Tab</kbd> a list containing the alternative actions pops up. As long as this list is visible the arrow up and down keys move the cursor in this list. Press <kbd>Tab</kbd> again to return to the result items; the action list disappears.

Modifiers can be used as a shortcut for the alternative actions. <kbd>Meta</kbd>, <kbd>Ctrl</kbd> or <kbd>Shift</kbd> can be hold to run the 1<sup>st</sup>, 2<sup>nd</sup> or 3<sup>rd</sup> alternative action when activating the item. Slightly different: Holding <kbd>Alt</kbd> while activating an item runs the 1<sup>st</sup> fallback item with the entered query.

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
<kbd>Alt</kbd>+<kbd>F4</kbd> | Quit albert

## The extensions

> *TODO*
