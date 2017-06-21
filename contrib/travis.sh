#! /bin/bash

IMAGE="libral/$TARGET"

USERDIR=/var/tmp/libral
mkdir -p $USERDIR

CIDFILE="$USERDIR/$TARGET.cid"

if [ "$TARGET" == "f25-build" ]
then
   docker pull $IMAGE
   docker run --cidfile "$CIDFILE" -e 'MAKEFLAGS=-j2' $IMAGE
elif [ "$TARGET" == "el6-build-static" ]
   docker pull $IMAGE
   docker run --cidfile "$CIDFILE" -e 'MAKEFLAGS=-j2' $IMAGE
else
    echo "Unknown TARGET '$TARGET'"
    exit 1
fi
