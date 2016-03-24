#!/bin/bash

TEMPLATE="SlickTemplate.qss"

function makeStyle(){
  cat "$1" | sed \
    -e "s/%background_color%/$2/g" \
    -e "s/%foreground_color%/$3/g" \
    -e "s/%button_color%/$4/g" \
    -e "s/%scroll_color%/$5/g" \
    -e "s/%selection_background_color%/$6/g" > "${7}"
}

#makeStyle            bg       fg       button   scroll   selection output
makeStyle "$TEMPLATE" "fcfcfc" "000000" "bdbdbd" "e2e2e2" "eeeeee" "$1/Yosemite.qss"
makeStyle "$TEMPLATE" "f4000000" "ffffff" "484848" "484848" "262626" "$1/Yosemite Dark.qss"

