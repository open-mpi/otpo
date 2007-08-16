#
# Copyright (c) 2007 Cisco, Inc. All rights reserved.
#

# -*- shell-script -*- 
# 
# $HEADER$
#  

#
# Init autoconf
#

AC_INIT(otpo.c)
AC_PREREQ(2.52)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR(config)

echo "Configuring OTPO"

#
# The third argument to AM_INIT_AUTOMAKE surpresses the PACKAGE and
# VERSION macors
#

AM_INIT_AUTOMAKE(otpo, 1.0, 'no')

# Setup the reporting/otpo_config.h file

AH_TOP([/* -*- c -*-
 *
 *	otpo configuation header file.
 *      See the top-level LICENSE file for license and copyright
 *      information.
 *
 *	$Id: configure.in,v 1.35 2004/03/09 20:00:23 adharurk Exp $
 *
 * Function: - OS, CPU and compiler dependent configuration 
 */

#ifndef OTPO_CONFIG_H
#define OTPO_CONFIG_H
])
AH_BOTTOM([#endif /* OTPO_CONFIG_H */])

#
# Make automake clean emacs ~ files for "make clean"
#

CLEANFILES="*~"
AC_SUBST(CLEANFILES)

#
# This is useful later
#

AC_CANONICAL_HOST
AC_DEFINE_UNQUOTED(OTPO_ARCH, "$host", 
    [Architecture that we are compiled for])

#
# Get various programs
# Bias towards mpicc/mpic++/mpif77
# C compiler
#

if test "$CC" != ""; then
    BASE="`basename $CC`"
else
    BASE=
fi
if test "$BASE" = "" -o "$BASE" = "." -o "$BASE" = "cc" -o \
    "$BASE" = "gcc" -o "$BASE" = "xlc" -o "$BASE" = "pgcc" -o \
    "$BASE" = "icc"; then
    AC_CHECK_PROG(HAVE_MPICC, mpicc, yes, no)
    if test "$HAVE_MPICC" = "yes"; then
        CC=mpicc
        export CC
    fi
fi

CFLAGS_save="$CFLAGS"
AC_PROG_CC
CFLAGS="$CFLAGS_save"

#
# Because these are meant to be used for debugging, after all
#

if test -z "$CFLAGS"; then
    CFLAGS="-g"
fi

#
# Find ranlib
#

AC_PROG_RANLIB

#
# Ensure that we can compile and link an MPI program
#

AC_CHECK_HEADERS([mpi.h])
AC_CHECK_FUNCS([MPI_Initialized])

#
# Check Open-MPI is being used
#
AC_TRY_COMPILE([],
        [#ifdef OPEN_MPI 
         return 0;
         #else
         printf("FAIL TO COMPILE, NEED OPEN-MPI\n");
         exit(1);
         #endif
            ],
            [],
            [])

#
# checking for ADCL
#
AC_MSG_CHECKING(for ADCL main directory)
AC_ARG_WITH(adcl_dir,
    AC_HELP_STRING([--with-adcl-dir=dir],[Main ADCL directory (default=$PWD/ADCL)]),
    adcl_dir="${with_adcl_dir}",adcl_dir="${PWD}/ADCL")
AC_MSG_RESULT(${adcl_dir})

AC_MSG_CHECKING(for ADCL lib directory)
AC_ARG_WITH(adcl_lib_dir,
    AC_HELP_STRING([--with-adcl-lib-dir=dir],[ADCL Lib directory (default=ADCL_DIR/lib)]),
    adcl_lib_dir="${with_adcl_lib_dir}",adcl_lib_dir="${adcl_dir}/lib")
AC_MSG_RESULT(${adcl_lib_dir})

AC_MSG_CHECKING(for ADCL library name)
AC_ARG_WITH(adcl_lib_name,
    AC_HELP_STRING([--with-adcl-lib-name=name],[ADCL library name (default=libadcl.a)]),
    adcl_lib_name="${with_lib_name}",adcl_lib_name="libadcl.a")
AC_MSG_RESULT(${adcl_lib_name})

if test "${adcl_dir}" = "${PWD}/ADCL"; then
   AC_MSG_CHECKING([for subdir args])
   OTPO_CONFIG_SUBDIR_ARGS([adcl_subdir_args])
   AC_MSG_RESULT([$adcl_subdir_args])
   adcl_subdir_args="$adcl_subdir_args --with-num-tests=1 --enable-printf-tofile --enable-userlevel-timings --enable-dummy-mpi"
   OTPO_CONFIG_SUBDIR(ADCL, [$adcl_subdir_args], [HAPPY=1], [HAPPY=0])
fi

AM_CONDITIONAL([WANT_INTERNAL_ADCL], [test "${adcl_dir}" = "${PWD}/ADCL"])

AC_MSG_CHECKING([if we want MPI_LEAVE_PINNED])
AC_ARG_ENABLE(leave-pinned,
AC_HELP_STRING ([--enable-leave-pinned],
	       [enable the usage of mpi_leave_pinned (default:enabled)]))
if test "$enable_leave_pinned" != "no"; then 
   AC_MSG_RESULT([yes])
   PINNED=1
else
   AC_MSG_RESULT([no])
   PINNED=0
fi

AC_DEFINE_UNQUOTED([LEAVE_PINNED],$PINNED,[Wether we enable MPI_LEAVE_PINNED])
		       
AC_MSG_CHECKING([if we want MPI_PAFFINITY_ALONE])
AC_ARG_ENABLE(paffinity-alone,
AC_HELP_STRING ([--enable-paffinity-alone],
	       [enable the usage of mpi_paffinity_alone (default:enabled)]))
if test "$enable_paffinity_alone" != "no"; then 
   AC_MSG_RESULT([yes])
   PAFFINITY=1
else
   AC_MSG_RESULT([no])
   PAFFINITY=0
fi

AC_DEFINE_UNQUOTED([PAFFINITY_ALONE],$PAFFINITY,[Wether we enable MPI_PAFFINITY_ALONE])

CPPFLAGS="-I${adcl_dir}/include"
LDFLAGS="-L${adcl_lib_dir}"
LIBS="-ladcl"

AC_CHECK_HEADERS([ADCL.h])
AC_CHECK_FUNCS([ADCL_Init])

#
# Party on
#

AM_CONFIG_HEADER([otpo_config.h])
AC_CONFIG_FILES([
    Makefile
])
AC_OUTPUT
