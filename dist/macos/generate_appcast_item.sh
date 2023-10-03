#!/usr/bin/env bash

set -exu

DMG_PATH="$1"
VERSION="$2"
SPARKLE_ED_PRIVATE_KEY="$3"
OUTPUT="$4"
BREW_PREFIX="$(brew --prefix)"

echo "
    <item>
      <title>Version $VERSION</title>
      <pubDate>$(date +'%a, %d %b %Y %H:%M:%S %z')</pubDate>
      <link>https://albertlauncher.github.io/</link>
      <sparkle:version>2.0</sparkle:version>
      <sparkle:minimumSystemVersion>11.0.0</sparkle:minimumSystemVersion>
      <sparkle:releaseNotesLink>https://albertlauncher.github.io/news/</sparkle:releaseNotesLink>
      <enclosure
        url=\"https://github.com/albertlauncher/albert/releases/download/v$VERSION/Albert-v$VERSION.dmg\"
        sparkle:version=\"$VERSION\"
        sparkle:shortVersionString=\"$VERSION\"
        $($BREW_PREFIX/Caskroom/sparkle/2.5.0/bin/sign_update -s "$SPARKLE_ED_PRIVATE_KEY" "$DMG_PATH")
        type=\"application/octet-stream\"/>
    </item>
" > "$OUTPUT"

