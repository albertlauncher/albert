#!/usr/bin/python

import os
import re
import sys
import urllib
import string
from shutil import copyfile

ID_KEYWORD = "PROJECTID"
NAMESPACE_KEYWORD = "PROJECTNAMESPACE"
PRETTYNAME_KEYWORD = "PROJECTPRETTYNAME"
ID_PATTERN = "^([a-z0-9]+)$"
NAMESPACE_PATTERN = "^([A-Za-z][A-Za-z0-9]+)$"
PRETTYNAME_PATTERN = "^([A-Za-z0-9 _\\-]+)$"
#LOC_BASE = "https://raw.githubusercontent.com/ManuelSchneid3r/albert/master/src/plugins/templateExtension/"
LOC_BASE = "https://raw.githubusercontent.com/idkCpp/albert/create_plugin_py/src/plugins/templateExtension/"
LOC_INDEX_FILE = "index"
TEMPLATE_EXTENSION_BASE = "templateExtension/"

if len(sys.argv) != 4:
    u = "Usage: create_plugin.py <id [a-z0-9]> <namespace> <Pretty Name>"
    sys.stderr.write(u)
    sys.exit(1)

id_regex = re.compile(ID_PATTERN)
namespace_regex = re.compile(NAMESPACE_PATTERN)
prettyname_regex = re.compile(PRETTYNAME_PATTERN)

id_string = sys.argv[1]
namespace_string = sys.argv[2]
prettyname_string = sys.argv[3]

if not id_regex.match(id_string):
    e = "ID has to match " + ID_PATTERN
    sys.stderr.write(e)
    sys.exit(1)

if not namespace_regex.match(namespace_string):
    e = "Namespace has to match " + NAMESPACE_PATTERN
    sys.stderr.write(e)
    sys.exit(1)

if not prettyname_regex.match(prettyname_string):
    e = "Pretty Name has to match " + PRETTYNAME_PATTERN
    sys.stderr.write(e)
    sys.exit(1)

raw_input("Are we in the src/plugins directory? If not do not proceed because it won't work! ")

print("Creating directory . . .")
os.mkdir(id_string)
os.chdir(id_string)

decision = "INVALID"
while decision.lower() != "dl" and decision.lower() != "cp":
    if decision is not "INVALID":
        print("Give 'dl' or 'cp' as answer!")
    decision = raw_input("Do you want to download the newest template files or copy existing ones? (dl/cp) ")

filesToPrepare = []

if decision.lower() == "dl":
    print("Downloading index file . . .")
    urllib.urlretrieve(LOC_BASE + LOC_INDEX_FILE, LOC_INDEX_FILE)


    with open(LOC_INDEX_FILE) as indexFile:
        for line in indexFile:
            line = string.replace(line, "\n", "")
            print("Downloading file " + line)
            urllib.urlretrieve(LOC_BASE + line, line)
            filesToPrepare.append(line)
else:
    files = os.listdir("../" + TEMPLATE_EXTENSION_BASE)
    for nextFile in files:
        print("Copying file " + nextFile)
        copyfile("../" + TEMPLATE_EXTENSION_BASE + nextFile, nextFile)
        filesToPrepare.append(nextFile) 

for localFile in filesToPrepare:
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
