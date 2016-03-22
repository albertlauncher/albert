#!/bin/bash

function makeStyle(){
  cat "$1" | sed \
    -e "s/%background_color%/$2/g" \
    -e "s/%foreground_color%/$3/g" \
    -e "s/%inputline_color%/$4/g" \
    -e "s/%highlight_color%/$5/g" \
    -e "s/%border_color%/$6/g" \
    > "$7"
}

#makeStyle template bg fg inputline highlight border output
makeStyle "$1" "FFFFFF" "808080" "D0D0D0" "000000" "808080" "$2/Bright.qss"
makeStyle "$1" "FFFFFF" "808080" "D0D0D0" "E07000" "FF8000" "$2/BrightOrange.qss"
makeStyle "$1" "FFFFFF" "808080" "D0D0D0" "E00070" "FF0080" "$2/BrightMagenta.qss"
makeStyle "$1" "FFFFFF" "808080" "D0D0D0" "00c060" "00FF80" "$2/BrightMint.qss"
makeStyle "$1" "FFFFFF" "808080" "D0D0D0" "60c000" "80FF00" "$2/BrightGreen.qss"
makeStyle "$1" "FFFFFF" "808080" "D0D0D0" "0070E0" "0080FF" "$2/BrightBlue.qss"
makeStyle "$1" "FFFFFF" "808080" "D0D0D0" "7000E0" "8000FF" "$2/BrightViolet.qss"
makeStyle "$1" "404040" "808080" "202020" "E07000" "FF8000" "$2/Dark.qss"
makeStyle "$1" "404040" "808080" "202020" "FF9020" "FF8000" "$2/DarkOrange.qss"
makeStyle "$1" "404040" "808080" "202020" "FF2090" "FF0080" "$2/DarkMagenta.qss"
makeStyle "$1" "404040" "808080" "202020" "20FF90" "00FF80" "$2/DarkMint.qss"
makeStyle "$1" "404040" "808080" "202020" "90FF20" "80FF00" "$2/DarkGreen.qss"
makeStyle "$1" "404040" "808080" "202020" "2090FF" "0080FF" "$2/DarkBlue.qss"
makeStyle "$1" "404040" "808080" "202020" "A040FF" "8000FF" "$2/DarkViolet.qss"
makeStyle "$1" "002b36" "93a1a1" "073642" "b58900" "b58900" "$2/SolarizedDarkYellow.qss"
makeStyle "$1" "002b36" "93a1a1" "073642" "cb4b16" "cb4b16" "$2/SolarizedDarkOrange.qss"
makeStyle "$1" "002b36" "93a1a1" "073642" "dc322f" "dc322f" "$2/SolarizedDarkRed.qss"
makeStyle "$1" "002b36" "93a1a1" "073642" "d33682" "d33682" "$2/SolarizedDarkMagenta.qss"
makeStyle "$1" "002b36" "93a1a1" "073642" "6c71c4" "6c71c4" "$2/SolarizedDarkViolet.qss"
makeStyle "$1" "002b36" "93a1a1" "073642" "268bd2" "268bd2" "$2/SolarizedDarkBlue.qss"
makeStyle "$1" "002b36" "93a1a1" "073642" "2aa198" "2aa198" "$2/SolarizedDarkCyan.qss"
makeStyle "$1" "002b36" "93a1a1" "073642" "859900" "859900" "$2/SolarizedDarkGreen.qss"
makeStyle "$1" "fdf6e3" "586e75" "eee8d5" "b58900" "b58900" "$2/SolarizedBrightYellow.qss"
makeStyle "$1" "fdf6e3" "586e75" "eee8d5" "cb4b16" "cb4b16" "$2/SolarizedBrightOrange.qss"
makeStyle "$1" "fdf6e3" "586e75" "eee8d5" "dc322f" "dc322f" "$2/SolarizedBrightRed.qss"
makeStyle "$1" "fdf6e3" "586e75" "eee8d5" "d33682" "d33682" "$2/SolarizedBrightMagenta.qss"
makeStyle "$1" "fdf6e3" "586e75" "eee8d5" "6c71c4" "6c71c4" "$2/SolarizedBrightViolet.qss"
makeStyle "$1" "fdf6e3" "586e75" "eee8d5" "268bd2" "268bd2" "$2/SolarizedBrightBlue.qss"
makeStyle "$1" "fdf6e3" "586e75" "eee8d5" "2aa198" "2aa198" "$2/SolarizedBrightCyan.qss"
makeStyle "$1" "fdf6e3" "586e75" "eee8d5" "859900" "859900" "$2/SolarizedBrightGreen.qss"

