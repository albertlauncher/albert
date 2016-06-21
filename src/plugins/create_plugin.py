#!/usr/bin/python

import os
import re
import sys
import urllib
import string

ID_KEYWORD = "##ID##"
NAMESPACE_KEYWORD = "##NAMESPACE##"
PRETTYNAME_KEYWORD = "##PRETTYNAME##"
#LOC_BASE = "https://raw.githubusercontent.com/ManuelSchneid3r/albert/master/src/plugins/templateExtension/"
LOC_BASE = "https://raw.githubusercontent.com/idkCpp/albert/create_plugin_py/src/plugins/templateExtension/"
LOC_INDEX_FILE = "index"

if len(sys.argv) != 4:
    u = "Usage: create_plugin.py <id [a-z0-9]> <namespace> <Pretty Name>"
    sys.stderr.write(u)
    sys.exit(1)

id_string = sys.argv[1]
namespace_string = sys.argv[2]
prettyname_string = sys.argv[3]


raw_input("Are we in the src/plugins directory? If not do not proceed because it won't work! ")

print("Creating directory . . .")
os.mkdir(id_string)
os.chdir(id_string)

print("Downloading index file . . .")
urllib.urlretrieve(LOC_BASE + LOC_INDEX_FILE, LOC_INDEX_FILE)

downloadedFiles = []

with open(LOC_INDEX_FILE) as indexFile:
    for line in indexFile:
        line = string.replace(line, "\n", "")
        print("Downloading file " + line)
        urllib.urlretrieve(LOC_BASE + line, line)
        downloadedFiles.append(line)

for localFile in downloadedFiles:
    print("Preparing file " + localFile)
    with open(localFile) as fd:
        tmpfile = localFile + ".tmp"
        tmp = open(tmpfile, "w")
        for line in fd:
            tmp.write(re.sub(ID_KEYWORD, id_string, 
                    re.sub(NAMESPACE_KEYWORD, namespace_string, 
                            re.sub(PRETTYNAME_KEYWORD, prettyname_string, line))))
        tmp.close()
        os.rename(tmpfile, localFile)
