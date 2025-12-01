#!/bin/sh

# Download if necessary a file
if [ ! -z "$DOWNLOAD" ]
then
    # Check if /data is writable
    if [ ! -w /data ]
    then
        echo "'/data' directory is not writable by '$(id -n -u):$(id -n -g)' ($(id -u):$(id -g)). ZIM file(s) can not be written."
        exit 1
    fi

    # Dwonload ZIM file
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
CMD="/usr/local/bin/kiwix-serve --port=$PORT $@"
echo $CMD
$CMD

# If error, print the content of /data
if [ $? -ne 0 ]
then
    echo "Here is the content of /data:"
    find /data -type f
fi
