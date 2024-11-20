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

if [ -z "$PORT" ]
then
    PORT=8080
fi
if [ -z "$ROOTPATH" ]
then
    CMD="/usr/local/bin/kiwix-serve --port=$PORT $*"
else
    CMD="/usr/local/bin/kiwix-serve --urlRootLocation $ROOTPATH --port=$PORT $*"
fi
echo $CMD
$CMD

# If error, print the content of /data
if [ $? -ne 0 ]
then
    echo "Here is the content of /data:"
    find /data -type f
fi
