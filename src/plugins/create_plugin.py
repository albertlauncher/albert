#!/usr/bin/python

import os
import re
import sys
import urllib

ID_KEYWORD = "##ID##"
NAMESPACE_KEYWORD = "##NAMESPACE##"
PRETTYNAME_KEYWORD = "##PRETTYNAME##"
#LOC_BASE = "https://raw.githubusercontent.com/ManuelSchneid3r/albert/master/src/plugins/templateExtension/"
LOC_BASE = "https://raw.githubusercontent.com/idkCpp/albert/create_plugin_py/src/plugins/templateExtension/"
LOC_INDEX_FILE = "index"

id_string = sys.argv[1]
namespace_string = sys.argv[2]
prettyname_string = sys.argv[3]

if len(sys.argv) != 4:
    u = "Usage: create_plugin.py <id [a-z0-9]> <namespace> <Pretty Name>"
    sys.stderr.write(u)
    sys.exit(1)


input("Are we in the src/plugins directory? If not do not proceed because it won't work! ")

print("Creating directory . . .")
os.mkdir(id_string)
os.chdir(id_string)

print("Downloading index file . . .")
urllib.urlretrieve(LOC_BASE + LOC_INDEX_FILE, LOC_INDEX_FILE)

downloadedFiles = []

with open(LOC_INDEX_FILE) as indexFile:
    for line in indexFile:
        print("Downloading file " + line)
        urllib.urlretrieve(LOC_BASE + line, line)
        downloadedFiles.append(line)
