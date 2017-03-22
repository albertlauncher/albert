#!/usr/bin/python

import os
import re
import sys
import string
from shutil import copyfile

ID_KEYWORD = "projectid"
NAMESPACE_KEYWORD = "ProjectNamespace"
PRETTYNAME_KEYWORD = "Template"
ID_PATTERN = "^([a-z0-9]+)$"
NAMESPACE_PATTERN = "^([A-Za-z][A-Za-z0-9]+)$"
PRETTYNAME_PATTERN = "^([A-Za-z0-9 _\\-]+)$"
TEMPLATE_EXTENSION_BASE = "templateExtension/"
CMAKE_PATTERN = "^add_subdirectory\(([^\)]+)\)$"

if len(sys.argv) != 4:
    u = "Usage: create_plugin.py <id [a-z0-9]> <namespace> <Pretty Name>\n"
    sys.stderr.write(u)
    sys.exit(1)

id_regex = re.compile(ID_PATTERN)
namespace_regex = re.compile(NAMESPACE_PATTERN)
prettyname_regex = re.compile(PRETTYNAME_PATTERN)
cmake_regex = re.compile(CMAKE_PATTERN)

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

baseDir = os.path.join("..", TEMPLATE_EXTENSION_BASE)
filesToPrepare = []

def scanDir(toscan):
    global filesToPrepare
    global baseDir
    if toscan:
        template = os.path.join(baseDir, toscan)
    else:
        template = baseDir
    ext = toscan
    files = os.listdir(template)
    for nextFile in files:
        path = os.path.join(template, nextFile)
        pathNew = os.path.join(ext, nextFile)
        if os.path.isdir(path):
            print("Creating directory " + pathNew)
            os.mkdir(pathNew)
            scanDir(os.path.join(toscan, nextFile))
        else:
            print("Copying file " + pathNew)
            copyfile(path, pathNew)
            filesToPrepare.append(pathNew)

scanDir("")

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

print("Leaving directory . . .")
os.chdir("..")
print("Updating CMakeLists.txt . . . ")
existingExtensions = []
preLines = []
postLines = []
preSubdir = True
postSubdir = False
with open("CMakeLists.txt", "r") as makelist:
    for line in makelist:
        match = cmake_regex.match(line)
        if match:
            preSubdir = False
            existingExtensions.append(match.group(1))
        else:
            postSubdir = not preSubdir
        
        if preSubdir:
            preLines.append(line)
        elif postSubdir:
            postLines.append(line)

existingExtensions.append(id_string)
existingExtensions.sort()

cmakefile = open("CMakeLists.txt", "w")
for line in preLines:
    cmakefile.write(line) # The newline is already in the line, because it didn't get stripped

for ext in existingExtensions:
    cmakefile.write("add_subdirectory(")
    cmakefile.write(ext)
    cmakefile.write(")\n")

for line in postLines:
    cmakefile.write(line) # The newline is already in the line, because it didn't get stripped

cmakefile.close()
