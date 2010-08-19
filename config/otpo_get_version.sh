#!/bin/sh
#
# otpo_get_version is created from otpo_get_version.m4 and otpo_get_version.m4sh.
#
# Copyright (c) 2004-2006 The Trustees of Indiana University and Indiana
#                         University Research and Technology
#                         Corporation.  All rights reserved.
# Copyright (c) 2004-2005 The University of Tennessee and The University
#                         of Tennessee Research Foundation.  All rights
#                         reserved.
# Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
#                         University of Stuttgart.  All rights reserved.
# Copyright (c) 2004-2005 The Regents of the University of California.
#                         All rights reserved.
# Copyright (c) 2008      Cisco Systems, Inc.  All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#



# OTPO_GET_VERSION(version_file, variable_prefix)
# -----------------------------------------------
# parse version_file for version information, setting
# the following shell variables:
#
#  prefix_VERSION
#  prefix_BASE_VERSION
#  prefix_MAJOR_VERSION
#  prefix_MINOR_VERSION
#  prefix_RELEASE_VERSION
#  prefix_GREEK_VERSION
#  prefix_WANT_SVN
#  prefix_SVN_R
#  prefix_RELEASE_DATE



srcfile="$1"
option="$2"

case "$option" in
    # svnversion can take a while to run.  If we don't need it, don't run it.
    --major|--minor|--release|--greek|--base|--help)
        otpo_ver_need_svn=0
        ;;
    *)
        otpo_ver_need_svn=1
esac


if test -z "$srcfile"; then
    option="--help"
else

    : ${otpo_ver_need_svn=1}
    : ${svnversion_result=-1}

        if test -f "$srcfile"; then
        srcdir=`dirname $srcfile`
        otpo_vers=`sed -n "
	t clear
	: clear
	s/^major/OTPO_MAJOR_VERSION/
	s/^minor/OTPO_MINOR_VERSION/
	s/^release/OTPO_RELEASE_VERSION/
	s/^greek/OTPO_GREEK_VERSION/
	s/^want_svn/OTPO_WANT_SVN/
	s/^svn_r/OTPO_SVN_R/
	s/^date/OTPO_RELEASE_DATE/
	t print
	b
	: print
	p" < "$srcfile"`
	eval "$otpo_vers"

        # Only print release version if it isn't 0
        if test $OTPO_RELEASE_VERSION -ne 0 ; then
            OTPO_VERSION="$OTPO_MAJOR_VERSION.$OTPO_MINOR_VERSION.$OTPO_RELEASE_VERSION"
        else
            OTPO_VERSION="$OTPO_MAJOR_VERSION.$OTPO_MINOR_VERSION"
        fi
        OTPO_VERSION="${OTPO_VERSION}${OTPO_GREEK_VERSION}"
        OTPO_BASE_VERSION=$OTPO_VERSION

        if test $OTPO_WANT_SVN -eq 1 && test $otpo_ver_need_svn -eq 1 ; then
            if test "$svnversion_result" != "-1" ; then
                OTPO_SVN_R=$svnversion_result
            fi
            if test "$OTPO_SVN_R" = "-1" ; then

                d=`date '+%m-%d-%Y'`
                if test -d "$srcdir/.svn" ; then
                    OTPO_SVN_R=r`svnversion "$srcdir"`
                    if test $? != 0; then
                        # The following is too long for Fortran
                        # OTPO_SVN_R="unknown svn version (svnversion not found); $d"
                        OTPO_SVN_R="? (no svnversion); $d"
                    fi
                elif test -d "$srcdir/.hg" ; then
                    # Check to see if we can find the hg command
                    # remember that $? reflects the status of the
                    # *last* command in a pipe change, so if "hg ..
                    # cut ..." runs and "hg" is not found, $? will
                    # reflect the status of "cut", not hg not being
                    # found.  So test for hg specifically first.
                    hg --version > /dev/null 2>&1
                    if test $? = 0; then
                        OTPO_SVN_R=hg`hg -v -R "$srcdir" tip | grep ^changeset: | head -n 1 | cut -d: -f3`
                    else
                        # The following is too long for Fortran
                        # OTPO_SVN_R="unknown hg version (hg not found); $d"
                        OTPO_SVN_R="? (no hg); $d"
                    fi
                fi
                if test "OTPO_SVN_R" = ""; then
                    OTPO_SVN_R="svn$d"
                fi

            fi
            OTPO_VERSION="${OTPO_VERSION}${OTPO_SVN_R}"
        fi
    fi


    if test "$option" = ""; then
	option="--full"
    fi
fi

case "$option" in
    --full|-v|--version)
	echo $OTPO_VERSION
	;;
    --major)
	echo $OTPO_MAJOR_VERSION
	;;
    --minor)
	echo $OTPO_MINOR_VERSION
	;;
    --release)
	echo $OTPO_RELEASE_VERSION
	;;
    --greek)
	echo $OTPO_GREEK_VERSION
	;;
    --svn)
	echo $OTPO_SVN_R
	;;
    --base)
        echo $OTPO_BASE_VERSION
        ;;
    --release-date)
        echo $OTPO_RELEASE_DATE
        ;;
    --all)
        echo ${OTPO_VERSION} ${OTPO_MAJOR_VERSION} ${OTPO_MINOR_VERSION} ${OTPO_RELEASE_VERSION} ${OTPO_GREEK_VERSION} ${OTPO_SVN_R}
        ;;
    -h|--help)
	cat <<EOF
$0 <srcfile> <option>

<srcfile> - Text version file
<option>  - One of:
    --full         - Full version number
    --major        - Major version number
    --minor        - Minor version number
    --release      - Release version number
    --greek        - Greek (alpha, beta, etc) version number
    --svn          - Subversion repository number
    --all          - Show all version numbers, separated by :
    --base         - Show base version number (no svn number)
    --release-date - Show the release date
    --help         - This message
EOF
        ;;
    *)
        echo "Unrecognized option $option.  Run $0 --help for options"
        ;;
esac

# All done

exit 0
