#!/bin/sh

# Download if necessary a file
if [ ! -z "$DOWNLOAD" ]
then
    ZIM=`basename $DOWNLOAD`
    wget $DOWNLOAD -O "$ZIM"

    # Set arguments
    if [ "$#" -eq "0" ]
    then
        set -- "$@" $ZIM
    fi
fi

if [ $# -eq 1 ] && [ "${1}" = "generate-library" ]; then
    # docker run [...] generate-library
    CMD="/usr/local/bin/generate-library"
else
    CMD="/usr/local/bin/kiwix-serve --port=80 $@"
fi

echo $CMD
$CMD

# If error, print the content of /data
if [ $? -ne 0 ]
then
    echo "Here is the content of /data:"
    find /data -type f
fi
