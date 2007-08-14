#! /bin/sh 
#
# $HEADER$
#
# Some helper functions
#

#
# Subroutine to check for the existence of various standard GNU tools
#
test_for_existence() {
    tfe_prog="$1"
    tfe_foo="`$tfe_prog --version`"
    if test "$?" != 0; then
	cat <<EOF

You must have GNU autoconf, automake, and libtool installed to build
the developer's version of LAM/MPI.  You can obtain these packages
from ftp://ftp.gnu.org/gnu/.

EOF
	# Stupid emacs: '
	exit 1
    fi
    unset tfe_prog tfe_foo
}


#
# Subroutine to execite the standard GNU tools, and if they fail,
# print out a warning.
#
run_and_check() {
    rac_progs="$*"
    echo "$rac_progs"
    eval $rac_progs
    if test "$?" != 0; then
	cat <<EOF

It seems that the execution of "$progs" has failed.
I am gonna abort.  :-(

This may be caused by an older version of one of the required
packages.  Please make sure you are using at least the following
versions:

GNU Autoconf 2.52
GNU Automake 1.5
GNU Libtool  1.4.2

EOF
	exit 1
    fi
    unset rac_progs
}

#
# Subroutine to look for standard files in a number of common places
# (e.g., ./config.guess, config/config.guess, dist/config.guess), and
# delete it.  If it's not found there, look for AC_CONFIG_AUX_DIR in
# the configure.ac script and try there.  If it's not there, oh well.
#
find_and_delete() {
    fad_file="$1"

    # Look for the file in "standard" places

    if test -f $fad_file; then
	rm -f $fad_file
    elif test -d config/$fad_file; then
	rm -f config/$fad_file
    elif test -d dist/$fad_file; then
	rm -f dist/$fad_file
    else

	# Didn't find it -- look for an AC_CONFIG_AUX_DIR line in
	# configure.[in|ac]

	if test -f configure.in; then
	    fad_cfile=configure.in
	elif test -f configure.ac; then
	    fad_cfile=configure.ac
	fi
	auxdir="`grep AC_CONFIG_AUX_DIR $fad_cfile | cut -d\( -f 2 | cut -d\) -f 1`"
	if test -f "$auxdir/$fad_file"; then
	    rm -f "$auxdir/$fad_file"
	fi
	unset fad_cfile
    fi
    unset fad_file
}


#
# Subroutine to actually do the GNU tool setup in the proper order, etc.
#
run_gnu_tools() {
    rgt_dir="$1"
    rgt_cur_dir="`pwd`"
    if test -d "$rgt_dir"; then
	cd "$rgt_dir"

	# See if the package doesn't want us to set it up

	if test -f .lam_no_gnu; then
	    cat <<EOF

*** Found .lam_no_gnu file -- skipping GNU setup in:
***   `pwd`

EOF
        else
	    cat <<EOF

*** Running GNU tools in directory: 
***   `pwd`

EOF

	    # Find and delete the GNU helper script files

	    find_and_delete config.guess
	    find_and_delete config.sub
	    find_and_delete depcomp
	    find_and_delete install-sh
	    find_and_delete ltconfig
	    find_and_delete ltmain.sh
	    find_and_delete missing
	    find_and_delete mkinstalldirs
	    find_and_delete libtool

            # Run the GNU tools

	    run_and_check aclocal
	    run_and_check autoheader
	    run_and_check autoconf
	    run_and_check libtoolize --automake --copy
	    run_and_check automake --foreign -a --copy --include-deps
	
	    # Go back to the original directory

	    cd "$rgt_cur_dir"
	fi
    fi
    unset rgt_dir rgt_cur_dir
}

##########################################################################
# Main
##########################################################################

#
# Are we in the right directory?  We must be in the top-level LAM
# directory.
#

if test -f autogen.sh -a -f configure.ac ; then
    bad=0
else
    cat <<EOF

You must run this script from the top-level lamtest directory.

EOF
    exit 1
fi

test_for_existence autoconf
test_for_existence automake
test_for_existence libtool

# Run the config in the top-level directory

run_gnu_tools .

# All done

exit 0
