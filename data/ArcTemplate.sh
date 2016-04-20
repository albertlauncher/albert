#!/bin/bash

TEMPLATE="ArcTemplate.qss"

function makeStyle(){
  cat "$1" | sed \
    -e "s/%background_color%/$2/g" \
    -e "s/%foreground_color%/$3/g" \
    -e "s/%inputline_color%/$4/g" \
    -e "s/%border_color%/$5/g" \
    -e "s/%button_color%/$6/g" \
    -e "s/%scroll_color%/$7/g" \
    -e "s/%selection_background_color%/$8/g" \
    -e "s/%selection_border_color%/$9/g" > "${10}"
}

#makeStyle            bg       fg       input    border   button   scrollbar sel_bg sel_border  output
makeStyle "$TEMPLATE" "e7e8eb" "727A8F" "fdfdfd" "CFD6E6" "ffffff" "b8babf" "95c4fb" "cfd6e6" "themes/Arc Blue.qss"
makeStyle "$TEMPLATE" "e7e8eb" "727A8F" "fdfdfd" "CFD6E6" "ffffff" "b8babf" "F5F6F7" "4084D6" "themes/Arc Grey.qss"
makeStyle "$TEMPLATE" "383C4A" "AFB8C5" "404552" "21252B" "ffffff" "b8babf" "4084D6" "4084D6" "themes/Arc Dark Blue.qss"
makeStyle "$TEMPLATE" "383C4A" "AFB8C5" "404552" "21252B" "ffffff" "b8babf" "404552" "2B2E39" "themes/Arc Dark Grey.qss"