#!/bin/bash

# Script for making releases to Source Forge and web site
#   automatically edits version files
#   automatically manages tags/branches
#   generates change logs
#   generates documentation packs
#   generate tarballs/archives
#   uploads to Source Forge
#   uploads to opalvoip web site

if [ "$1" = "-x" ]; then
  shift
  set -x
fi

#
#  set common globals
#

SVN="svn"
RSYNC="rsync -e ssh -avz"
SVN2CL="svn2cl"
TAR=tar
ZIP=zip

SNAPSHOTS=./snapshots
WEB_HOST=files.opalvoip.org
WEB_HTML_DIR="/mnt/www/files.opalvoip.org/html"
WEB_DOCS_DIR=${WEB_HTML_DIR}/docs
WEB_CHANGELOG_DIR=${WEB_HTML_DIR}/docs/ChangeLogs

BINARY_EXT=( exe dll lib wav png gif ico sw mdb dsp dsw vcp vcw wpj wsp )

COMMAND_NAMES=(show \
               co \
               trytag \
               tag \
               log \
               docs \
               arch \
               upsf \
               web \
               all \
              )
FUNCTIONS=(    show_tags \
               check_out \
               try_tag \
               create_tag \
               create_changelog \
               create_docs \
               create_archive \
               upload_to_sourceforge \
               update_website \
               do_all_for_release \
              )


#
#  parse command line
#

if [ -z "$1" -o -z "$2" -o -z "$3" ]; then
  echo "usage: $0 cmd(s) proj [ "stable" ] tag [ prevtag ]"
  echo "  cmd may be one of: ${COMMAND_NAMES[*]}"
  echo "  proj may be one of: ptlib opal ipp-codecs"
  echo "  tag is either 'next' or 'last' or an aritrary name"
  exit
fi

COMMANDS=$1
shift

base=$1
shift

if [ -z "$SOURCEFORGE_USERNAME" ]; then
  SOURCEFORGE_USERNAME=`whoami`
fi

case "$base" in
  ptlib | opal)
    repository=svn+ssh://$SOURCEFORGE_USERNAME@svn.code.sf.net/p/opalvoip/code/$base
  ;;
  ipp-codecs)
    repository=svn+ssh://$SOURCEFORGE_USERNAME@svn.code.sf.net/p/ippcodecs/code
  ;;
  *)
    echo Unknown base project: $base
    exit
esac

extract_versions_cmd="$SVN ls ${repository}/tags | awk '/v[0-9]+_[0-9]*["
if [ "$1" = "stable" ]; then
  extract_versions_cmd+=02468
  shift
else
  extract_versions_cmd+=13579
fi

extract_versions_cmd+=']_[0-9]+/{gsub("v|/","");gsub("_"," ");print}'
extract_versions_cmd+="' | sort -k1n -k2n -k3n"

release_tag=$1
previous_tag=$2


echo "Releasing ${base}: \"${COMMANDS[*]}\" $release_tag $previous_tag"

#
# get release tag
#

release_version=
previous_version=( `/bin/sh -c "${extract_versions_cmd} | tail -1"` )
if [ -z $previous_version ]; then
  echo Could not find last release!
  exit
fi

if [ "$release_tag" = "next" ]; then
  release_version=( ${previous_version[*]} )
  let release_version[2]++
  release_tag=v${release_version[0]}_${release_version[1]}_${release_version[2]}
elif [ "$release_tag" = "last" ]; then
  release_version=( ${previous_version[*]} )
  previous_version=( `/bin/sh -c "$extract_versions_cmd | tail -2 | head -1"` )
  release_tag=v${release_version[0]}_${release_version[1]}_${release_version[2]}
else
  release_version=( `echo $release_tag | awk '/v[0-9]+_[0-9]+_[0-9]+/{gsub("v","");gsub("_"," ");print}'` )
fi

if [ -z $previous_tag ]; then
  previous_tag=v${previous_version[0]}_${previous_version[1]}_${previous_version[2]}
else
  previous_version=( `echo $previous_tag | awk '/v[0-9]+_[0-9]+_[0-9]+/{gsub("v","");gsub("_"," ");print}'` )
fi

release_branch=branches/v${release_version[0]}_${release_version[1]}
exists=`$SVN info ${repository}/$release_branch 2>/dev/null`
if [ -z "$exists" ]; then
  release_branch=trunk
fi

previous_verstr=${previous_version[0]}.${previous_version[1]}.${previous_version[2]}
release_verstr=${release_version[0]}.${release_version[1]}.${release_version[2]}


#
#  set calculated names
#
SRC_ARCHIVE_TBZ2=${SNAPSHOTS}/${base}-${release_verstr}.tar.bz2
DOC_ARCHIVE_TBZ2_BASE=${base}-${release_verstr}-htmldoc.tar.bz2
DOC_ARCHIVE_TBZ2=${SNAPSHOTS}/${DOC_ARCHIVE_TBZ2_BASE}
BASE_TAR=${base}-${release_verstr}

SRC_ARCHIVE_ZIP=${SNAPSHOTS}/${base}-${release_verstr}-src.zip
DOC_ARCHIVE_ZIP=${SNAPSHOTS}/${base}-${release_verstr}-htmldoc.zip
BASE_ZIP=${base}

SOURCE_FORGE_FILES=( $SRC_ARCHIVE_TBZ2 $DOC_ARCHIVE_TBZ2 $SRC_ARCHIVE_ZIP $DOC_ARCHIVE_ZIP )

CHANGELOG_BASE=ChangeLog-${base}-${release_tag}.txt
CHANGELOG_FILE="${base}/$CHANGELOG_BASE"
VERSION_FILE="${base}/version.h"
REVISION_FILE="${base}/revision.h.in"



#
# show tags
#

function show_tags () {
  echo "Release  tag for $base is $release_tag"
  echo "Previous tag for $base is $previous_tag"
}

#
# check out
#

function check_out () {
  echo "Deleting $base"
  rm -rf $base
  exists=`$SVN info ${repository}/tags/$release_tag 2>/dev/null`
  if [ -n "$exists" ]; then
    echo "Checking out $release_tag of $base"
    $SVN co ${repository}/tags/${release_tag} $base >/dev/null
  else
    echo "Checking out HEAD of $release_branch of $base"
    $SVN co ${repository}/$release_branch $base >/dev/null
  fi
}


#
# switch code to tag or check out of not done yet
#

function switch_to_version () {
  if [ -d $base ]; then
    exists=`$SVN info ${repository}/tags/$release_tag 2>/dev/null`
    if [ -n "$exists" ]; then
      echo "Switching to $release_tag of $base"
      ( cd $base ; $SVN switch ${repository}/tags/${release_tag} )
    else
      echo "Switching to HEAD of $release_branch of $base"
      ( cd $base ; $SVN switch ${repository}/$release_branch >/dev/null )
    fi
  else
    check_out
  fi
}


#
# create new tag
#

function create_tag () {
  exists=`$SVN info ${repository}/tags/$release_tag 2>/dev/null`
  if [ -n "$exists" ]; then
    echo "Tag $release_tag in $base already exists!"
    return
  fi

  if [ -d $base ]; then
    echo "Switching to $release_branch of $base"
    ( cd $base ; $SVN switch ${repository}/$release_branch >/dev/null )
  else
    echo "Checking out $release_branch of $base"
    $SVN co ${repository}/$release_branch $base >/dev/null
  fi

  if [ "$release_branch" = "trunk" -a ${release_version[2]} = 0 ]; then
    release_branch=branches/v${release_version[0]}_${release_version[1]}
    if [ -n "$debug_tagging" ]; then
      echo "Would svn copy from ${repository}/trunk ${repository}/$release_branch"
      echo "Would switch to $release_branch of $base"
    else
      echo "Creating branch $release_tag in $base"
      $SVN copy ${repository}/trunk ${repository}/$release_branch
      echo "Switching to $release_branch of $base"
      ( cd $base ; $SVN switch ${repository}/$release_branch >/dev/null )
    fi
  fi

  if [ -n "$release_version" -a -e "$VERSION_FILE" ]; then
    version_file_changed=0
    file_version=`awk '/MAJOR_VERSION/{printf "%s ",$3};/MINOR_VERSION/{printf "%s ",$3};/BUILD_NUMBER/{printf "%s",$3}' "$VERSION_FILE"`
    if [ "${release_version[*]}" = "${file_version[*]}" ]; then
      echo "Version file $VERSION_FILE already set to ${release_version[*]}"
    else
      echo "Adjusting $VERSION_FILE from ${file_version[*]} to ${release_version[*]}"
      awk "/MAJOR_VERSION/{print \$1,\$2,\"${release_version[0]}\";next};/MINOR_VERSION/{print \$1,\$2,\"${release_version[1]}\";next};/BUILD_NUMBER/{print \$1,\$2,\"${release_version[2]}\";next};/BUILD_TYPE/{print \$1,\$2,\"ReleaseCode\";next};{print}" "$VERSION_FILE" > "$VERSION_FILE.tmp"
      mv -f "$VERSION_FILE.tmp" "$VERSION_FILE"
      version_file_changed=1
    fi

    build_type=`awk '/BUILD_TYPE/{print $3}' "$VERSION_FILE"`
    if [ "$build_type" != "ReleaseCode" ]; then
      echo "Adjusting $VERSION_FILE from $build_type to ReleaseCode"
      awk '/BUILD_TYPE/{print $1,$2,"ReleaseCode";next};{print}' "$VERSION_FILE" > "$VERSION_FILE.tmp"
      mv -f "$VERSION_FILE.tmp" "$VERSION_FILE"
      version_file_changed=1
    fi

    if [ $version_file_changed ]; then
      # Add or remove a space to force check in
      if grep --quiet "revision.h " "$REVISION_FILE" ; then
        sed --in-place "s/revision.h /revision.h/" "$REVISION_FILE"
      else
        sed --in-place "s/revision.h/revision.h /" "$REVISION_FILE"
      fi
      msg="Update release version number to $release_verstr"
      if [ -n "$debug_tagging" ]; then
        echo $msg
        $SVN diff "$VERSION_FILE" "$REVISION_FILE"
        $SVN revert "$VERSION_FILE" "$REVISION_FILE"
      else
        $SVN commit --message "$msg" "$VERSION_FILE" "$REVISION_FILE"
      fi
    fi
  fi

  if [ -n "$debug_tagging" ]; then
    echo "Would svn copy from ${repository}/$release_branch ${repository}/tags/$release_tag"
  else
    echo "Creating tag $release_tag in $base"
    $SVN copy ${repository}/$release_branch ${repository}/tags/$release_tag -m "Tagging $release_tag"
  fi

  if [ -e "$VERSION_FILE" ]; then
    new_version=( ${release_version[*]} )
    let new_version[2]++
    echo "Adjusting $VERSION_FILE from ${release_version[*]} ReleaseCode to ${new_version[*]} BetaCode"
    awk "/BUILD_NUMBER/{print \$1,\$2,\"${new_version[2]}\";next};/BUILD_TYPE/{print \$1,\$2,\"BetaCode\";next};{print}" "$VERSION_FILE" > "$VERSION_FILE.tmp"
    mv -f "$VERSION_FILE.tmp" "$VERSION_FILE"
    msg="Update version number for beta v${new_version[0]}.${new_version[1]}.${new_version[2]}"
    if [ -n "$debug_tagging" ]; then
      echo $msg
      $SVN diff "$VERSION_FILE"
      $SVN revert "$VERSION_FILE"
    else
      $SVN commit --message "$msg" "$VERSION_FILE"
    fi
  fi
}


#
# Test cration of new tag
#

function try_tag () {
  debug_tagging=yes
  echo "Trial tagging, SVN repository will NOT be changed!"
  create_tag
}


#
# create changelog
#

function create_changelog () {
  if [ "$previous_tag" = "nochangelog" ]; then
    echo "Not creating Change Log for $base $release_tag"
    return
  fi

  last_rev=`$SVN log --stop-on-copy ${repository}/tags/${previous_tag} | tail -4 | head -1 | awk '{print substr($1,2,100)}'`
  if [ -z $last_rev ]; then
    echo "Tag $previous_tag in $base does not exist!"
    return
  fi

  let last_rev=last_rev+1

  if [ -d $base ]; then
    output_name="-o $CHANGELOG_FILE"
  else
    output_name=--stdout
  fi

  release_rev=`$SVN log --stop-on-copy ${repository}/tags/${release_tag} 2>/dev/null | tail -4 | head -1 | awk '{print substr($1,2,100)}'`
  if [ -z $release_rev ]; then
    release_rev=HEAD
    echo "Creating $CHANGELOG_FILE between $release_branch HEAD and tag $previous_tag (rev $last_rev)"
  else
    echo "Creating $CHANGELOG_FILE between tags $release_tag (rev $release_rev) and $previous_tag (rev $last_rev)"
  fi

  $SVN2CL --break-before-msg --include-rev ${output_name} -r ${last_rev}:${release_rev} ${repository}/$release_branch
}

#
# create an archive
#

function create_archive () {

  check_out

  echo Creating source archive $SRC_ARCHIVE_TBZ2
  if [ "${base}" != "${BASE_TAR}" ]; then
    mv ${base} ${BASE_TAR}
  fi
  exclusions="--exclude=.svn --exclude=.cvsignore"
  $TAR -cjf $SRC_ARCHIVE_TBZ2 $exclusions ${BASE_TAR} >/dev/null
  if [ "${base}" != "${BASE_TAR}" ]; then
    mv ${BASE_TAR} ${base}
  fi

  echo Creating source archive ${SRC_ARCHIVE_ZIP}
  if [ "${base}" != "${BASE_ZIP}" ]; then
    mv ${base} ${BASE_ZIP}
  fi
  exclusions="-x *.svn* -x .cvsignore"
  for ext in ${BINARY_EXT[*]} ; do
    files=`find ${BASE_ZIP} -name \*.${ext}`
    if [ -n "$files" ]; then
      $ZIP -g $SRC_ARCHIVE_ZIP $files >/dev/null 2>&1
      exclusions="$exclusions -x *.$ext"
    fi
  done
  $ZIP -grl $SRC_ARCHIVE_ZIP ${BASE_ZIP} $exclusions >/dev/null
  if [ "${base}" != "${BASE_ZIP}" ]; then
    mv ${BASE_ZIP} ${base}
  fi
}

#
#  create a documentation set
#
function create_docs () {

  switch_to_version

  rm -f $DOC_ARCHIVE_TBZ2 $DOC_ARCHIVE_ZIP

  echo Creating documents...
  (
    export PTLIBDIR=`pwd`/ptlib
    export OPALDIR=`pwd`/opal
    if [ "$base" != "ptlib" ]; then
      pushd $PTLIBDIR
      ./configure --disable-plugins
      popd
    fi

    cd ${base}
    pwd
    rm -rf html
    if ./configure --disable-plugins ; then
      make graphdocs
    fi
  ) > docs.log 2>&1

  if [ -d ${base}/html ]; then
    echo Creating document archive $DOC_ARCHIVE_TBZ2
    ( cd ${base} ; $TAR -cjf ../$DOC_ARCHIVE_TBZ2 html )
    echo Creating document archive $DOC_ARCHIVE_ZIP
    ( cd ${base} ; $ZIP -r ../$DOC_ARCHIVE_ZIP html ) > /dev/null
  else
    echo Documents not created for $base
  fi
}

#
#  upload files to SourceForge
#

function upload_to_sourceforge () {
  files=
  for f in ${SOURCE_FORGE_FILES[*]}; do
    if [ -e $f ]; then
      files="$files $f"
    fi
  done
  if [ -z "$files" ]; then
    echo "Cannot find any of ${SOURCE_FORGE_FILES[*]}"
  else
    saved_dir="source_forge_dir"
    if [ -e "${saved_dir}" ]; then
      upload_dir=`cat ${saved_dir}`
      read -p "Source Forge sub-directory [${upload_dir}]: "
    else
      read -p "Source Forge sub-directory: "
    fi
    if [ -n "${REPLY}" ]; then
      upload_dir="${REPLY}"
    fi
    if [ -z "${upload_dir}" ]; then
      echo Not uploading.
    else
      echo "Uploading files to Source Forge directory ${upload_dir}"
      rsync -avP -e ssh $files "${SOURCEFORGE_USERNAME},opalvoip@frs.sourceforge.net:/home/frs/project/o/op/opalvoip/${upload_dir}"
    fi
  fi
}


#
# Update the web site
#

function update_website () {
  if [ -e "$CHANGELOG_FILE" ]; then
    echo "Copying $CHANGELOG_FILE to ${WEB_HOST}:$WEB_CHANGELOG_DIR"
    scp "$CHANGELOG_FILE" "${WEB_HOST}:$WEB_CHANGELOG_DIR"
  else
    echo "No $CHANGELOG_FILE, use 'log' command to generate."
  fi

  already_set=`ssh $WEB_HOST "grep v$release_verstr $WEB_CHANGELOG_DIR/.htaccess"`
  if [ -z "$already_set" ]; then
    echo "Adding description for v$previous_verstr to v$release_verstr"
    ssh $WEB_HOST "echo \"AddDescription \"Changes from v$previous_verstr to v$release_verstr of ${base}\" $CHANGELOG_BASE\" >> $WEB_CHANGELOG_DIR/.htaccess"
  else
    echo "Description already added for v$release_verstr"
  fi

  if [ -e $DOC_ARCHIVE_TBZ2 ]; then
    doc_dir="$WEB_DOCS_DIR/${base}-v${release_version[0]}_${release_version[1]}"
    doc_tar="$WEB_DOCS_DIR/$DOC_ARCHIVE_TBZ2_BASE"
    echo "Copying $DOC_ARCHIVE_TBZ2 to ${WEB_HOST}:$WEB_DOCS_DIR"
    scp "$DOC_ARCHIVE_TBZ2" "${WEB_HOST}:$WEB_DOCS_DIR"
    echo "Creating online document directory ${WEB_HOST}:$doc_dir"
    ssh $WEB_HOST "rm -rf $doc_dir ; mkdir $doc_dir ; $TAR -xjf $doc_tar -C $doc_dir --strip-components 1 ; rm $doc_tar"
  else
    echo "No $DOC_ARCHIVE_TBZ2, use 'docs' command to generate."
  fi
}


#
# Do all the parts for a "normal" release
#

function do_all_for_release () {
  check_out
  create_changelog
  create_tag
  create_archive
  create_docs
  update_website
  upload_to_sourceforge
}

#
# Main loop, execute all commands specified.
#

for CMD in ${COMMANDS[*]} ; do
  i=0
  unknownCommand=1
  for NAME in ${COMMAND_NAMES[*]} ; do
    if [ $CMD = $NAME ]; then
      ${FUNCTIONS[i]}
      unknownCommand=0
    fi
    let i=i+1
  done
  if [ $unknownCommand != 0 ]; then
    echo "Unknown command '$CMD'"
  fi
done


