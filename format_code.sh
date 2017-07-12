#!/usr/bin/bash

files=(
"src/installer/kiwix-install.cpp"
"src/searcher/kiwix-search.cpp"
"src/reader/kiwix-read.cpp"
"src/manager/kiwix-manage.cpp"
"src/server/kiwix-serve.cpp"
)

for i in "${files[@]}"
do
  echo $i
  clang-format -i -style=file $i
done
