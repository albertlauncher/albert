#!/usr/bin/python

import os
import re
import sys
import string
from shutil import copyfile

ID_KEYWORD = "PROJECTID"
NAMESPACE_KEYWORD = "PROJECTNAMESPACE"
PRETTYNAME_KEYWORD = "PROJECTPRETTYNAME"
ID_PATTERN = "^([a-z0-9]+)$"
NAMESPACE_PATTERN = "^([A-Za-z][A-Za-z0-9]+)$"
PRETTYNAME_PATTERN = "^([A-Za-z0-9 _\\-]+)$"
TEMPLATE_EXTENSION_BASE = "templateExtension/"

if len(sys.argv) != 4:
    u = "Usage: create_plugin.py <id [a-z0-9]> <namespace> <Pretty Name>\n"
    sys.stderr.write(u)
    sys.exit(1)

id_regex = re.compile(ID_PATTERN)
namespace_regex = re.compile(NAMESPACE_PATTERN)
prettyname_regex = re.compile(PRETTYNAME_PATTERN)

id_string = sys.argv[1]
namespace_string = sys.argv[2]
prettyname_string = sys.argv[3]

if not id_regex.match(id_string):
    e = "ID has to match " + ID_PATTERN + "\n"
    sys.stderr.write(e)
    sys.exit(1)

if not namespace_regex.match(namespace_string):
    e = "Namespace has to match " + NAMESPACE_PATTERN + "\n"
    sys.stderr.write(e)
    sys.exit(1)

if not prettyname_regex.match(prettyname_string):
    e = "Pretty Name has to match " + PRETTYNAME_PATTERN + "\n"
    sys.stderr.write(e)
    sys.exit(1)

raw_input("Are we in the src/plugins directory? If not do not proceed because it won't work! ")

print("Creating directory . . .")
os.mkdir(id_string)
os.chdir(id_string)

filesToPrepare = []

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
