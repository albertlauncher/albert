#!/bin/bash

usage(){
  echo "Usage:"
  echo "$0 <id_[A-Za-z0-9]> <namespace> <pretty name>"
}

if [ ! $# -eq 3 ]; then
 usage
 exit 1
fi

ID="$1"
NAMESPACE="$2"
PRETTY="$3"

if [[ ! "$ID" =~ ^[a-z0-9]+$ ]]; then
  echo -e "\e[31mid has to be alphanumeric and all lowercase\e[0m"
  usage
  exit 1
fi

if [[ ! "$NAMESPACE" =~ ^[A-Za-z][A-Za-z0-9]+$ ]]; then
  echo -e "\e[31mNamespace id has to be alphanumeric and must not begin with a number\e[0m"
  usage
  exit 1
fi


echo "Clone template"
cp -r "templateExtension" "$ID"

echo "Adjust metadata.json"
sed -e "s/template/${ID}/" -e "s/Template/${PRETTY}/" "templateExtension/metadata.json" > "${ID}/metadata.json"

echo "Adjust CMakeListss.txt"
sed -e "s/template/${ID}/" "templateExtension/CMakeLists.txt" > "${ID}/CMakeLists.txt"

echo "Adjust namespaces"


for FILE in $( ls templateExtension | grep -e ".cpp$" -e ".h$" -e ".ui$" ); do
  sed \
    -e "s/namespace Template/namespace ${NAMESPACE}/" \
    -e "s/Template::/${NAMESPACE}::/" \
    "templateExtension/${FILE}" > "${ID}/${FILE}"
done
