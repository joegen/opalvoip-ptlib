#!/bin/sh

COMMIT=`git show 2> /dev/null | sed -n "s/^commit //p"`
if [ -z "$COMMIT" ]; then
   echo "git commit cannot be found"
   exit 1
fi

sed "s/GIT_COMMIT.*/GIT_COMMIT    \"$COMMIT\"/" $1.in > $1.tmp
if diff -q $1 $1.tmp >/dev/null 2>&1; then
   rm $1.tmp
   echo "Revision file unchanged"
else
   mv -f $1.tmp $1
   echo "Revision file updated to $COMMIT"
fi
