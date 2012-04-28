#!/bin/sh
SCRIPT_DIR=$(dirname $0)
RESOURCE_FILE=$SCRIPT_DIR/../src/common/resourceTools.h
MAP="static std::map<std::string, const char*> createResourceMap() { \n"
MAP=$MAP"\tstd::map<std::string, const char*> m; \n"

# Delete old version of the file
rm -f "$RESOURCE_FILE"

# Create header of resourceTools.h
cat << EOF > "$RESOURCE_FILE"
#ifndef KIWIX_RESOURCETOOLS_H
#define KIWIX_RESOURCETOOLS_H

#include <string>
#include <map>

std::string getResourceAsString(const std::string &name);

EOF

# Goes through all files in /static/
for FILE in `find . -type f | sed 's/\.\///' | grep -v .svn | grep -v Makefile | grep -v .sh | grep -v "~" | grep -v "#"`
do
    FILE_ID=`echo "$FILE" | sed "s/\//_/g" | sed "s/\./_/g" | sed "s/\-/_/g"`
    echo "Inserting $FILE... [$FILE_ID]"
    reswrap -s -t -oa $RESOURCE_FILE -r $FILE_ID $FILE
    MAP=$MAP"\tm[\""$FILE"\"] = "$FILE_ID"; \n"; 
done;
MAP=$MAP"\treturn m; \n";
MAP=$MAP"} \n\n"
MAP=$MAP"static std::map<std::string, const char*> resourceMap = createResourceMap(); \n\n"

# Create the map table 
# map<int, int> m = map_list_of (1,2) (3,4) (5,6) (7,8);
echo $MAP >> "$RESOURCE_FILE"

# Create the footer
cat << EOF >> "$RESOURCE_FILE"
#endif

EOF
