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

/usr/local/bin/kiwix-serve --port=80 $@
