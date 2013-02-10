#!/bin/sh

doMerge=0
doInfo=0

if test "x$1" = "xmerge" ; then
  doMerge=1
elif test "x$1" = "xinfo" ; then
  doInfo=1
else
  echo "error: most specify 'merge' or 'info'"
  exit 255
fi

if test \! -e version.h ; then
  echo "error: version.h not found"
  exit 255
fi

grep -q -i PTLib_VERSION_H version.h
if test $? -eq 0 ; then
  SVN_DIR=ptlib
else
  SVN_DIR=opal
fi

MAJOR_VERSION=`cat version.h | grep MAJOR_VERSION | cut -f3 -d' '`
MINOR_VERSION=`cat version.h | grep MINOR_VERSION | cut -f3 -d' '`
STABLE_VERSION=`echo $MINOR_VERSION | awk '{ v = ($0 + 0); v -= v % 2; print v; }'`

if test "x$MINOR_VERSION" = "xSTABLE_VERSION" ; then
  echo "error: current version is ${MAJOR_VERSION}.${MINOR_VERSION}, which is the stable version"
  exit 255
fi

STABLE_BRANCH=v${MAJOR_VERSION}_${STABLE_VERSION}
CURRENT_BRANCH=v${MAJOR_VERSION}_${MINOR_VERSION}

if test $doMerge = 1 ; then
  echo "Merging from $STABLE_BRANCH to this branch (${CURRENT_BRANCH})"
  svn merge ^/${SVN_DIR}/branches/${STABLE_BRANCH}
  exit
fi

if test $doInfo = 1 ; then
  RANGEINFO=`svn diff --depth empty . | grep "^   Merged" | grep $STABLE_BRANCH`
  if test "x$RANGEINFO" = "x" ; then
    echo "error: cannot find range of changes"
    exit 255
  fi
  RANGE=`echo "$RANGEINFO" | sed -e 's/^   Merged[^:]*://' -e 's/-/:/'`
  FROM=`echo $RANGE | cut -f1 -d:`
  TO=`echo $RANGE | cut -f2 -d:`
  echo "Backported changes from ${SVN_DIR}/branches/${STABLE_BRANCH}"
  svn log -${RANGE} ^/${SVN_DIR}/branches/${STABLE_BRANCH}
fi

exit
