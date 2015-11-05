#!/bin/sh 

# This script does all the magic calls to automake/autoconf and
# friends that are needed to configure a cvs checkout.  
#
# If you are compiling from a released tarball you don't need these
# tools and you shouldn't use this script.  Just call ./configure
# directly.


PROJECT="centroidvorony"
TEST_TYPE=-d
FILE=src

LIBTOOL_REQUIRED_VERSION=1.5
LIBTOOL_WIN32=1.5
AUTOCONF_REQUIRED_VERSION=2.59
AUTOMAKE_REQUIRED_VERSION=1.7
ACLOCAL_FLAGS="$ACLOCAL_FLAGS"

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.
ORIGDIR=`pwd`
cd $srcdir

check_version ()
{
    if expr $1 \>= $2 > /dev/null; then
	echo "yes (version $1)"
    else
	DIE=1
    fi
}

run() {
        echo "Running $1 ..."
        $1
}

echo
echo "I am testing that you have the required versions of libtool, autoconf, " 
echo "and automake. This test is not foolproof, so if anything goes wrong, it"
echo "will make your life easier to have the following packages (or newer "
echo "versions) installed:"
echo
echo "  * GNU autoconf 2.59"
echo "    - ftp://ftp.gnu.org/gnu/autoconf/"
echo "  * GNU automake 1.7  (1.8 and 1.6 will also work)"
echo "    - ftp://ftp.gnu.org/gnu/automake/"
echo "  * GNU libtool 1.5 "
echo "    - ftp://ftp.gnu.org/gnu/libtool/"
echo

DIE=0

OS=`uname -s`
case $OS in 
    *YGWIN* | *INGW*)
	echo "Looks like Win32, you will need libtool $LIBTOOL_WIN32 or newer."
	echo
	LIBTOOL_REQUIRED_VERSION=$LIBTOOL_WIN32
	;;
esac


echo -n "checking for libtool >= $LIBTOOL_REQUIRED_VERSION ... "
if (libtoolize --version) < /dev/null > /dev/null 2>&1; then
    VER=`libtoolize --version \
         | grep libtool | sed "s/.* \([0-9.]*\)[-a-z0-9]*$/\1/"`
    check_version $VER $LIBTOOL_REQUIRED_VERSION
else
    echo
    echo "  You must have libtool installed to compile $PROJECT."
    echo "  Install the appropriate package for your distribution,"
    echo "  or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
    DIE=1;
fi

echo -n "checking for autoconf >= $AUTOCONF_REQUIRED_VERSION ... "
if (autoconf --version) < /dev/null > /dev/null 2>&1; then
    VER=`autoconf --version \
         | grep -iw autoconf | sed "s/.* \([0-9.]*\)[-a-z0-9]*$/\1/"`
    check_version $VER $AUTOCONF_REQUIRED_VERSION
else
    echo
    echo "  You must have autoconf installed to compile $PROJECT."
    echo "  Download the appropriate package for your distribution,"
    echo "  or get the source tarball at ftp://ftp.gnu.org/pub/gnu/autoconf/"
    DIE=1;
fi

echo -n "checking for automake >= $AUTOMAKE_REQUIRED_VERSION ... "
if (automake-1.7 --version) < /dev/null > /dev/null 2>&1; then
   AUTOMAKE=automake-1.7
   ACLOCAL=aclocal-1.7
elif (automake-1.8 --version) < /dev/null > /dev/null 2>&1; then
   AUTOMAKE=automake-1.8
   ACLOCAL=aclocal-1.8
elif (automake --version) < /dev/null > /dev/null 2>&1; then
   AUTOMAKE=automake
   ACLOCAL=aclocal
else
    echo
    echo "  You must have automake 1.6 or newer installed to compile $PROJECT."
    echo "  Download the appropriate package for your distribution,"
    echo "  or get the source tarball at ftp://ftp.gnu.org/pub/gnu/automake/"
    DIE=1
fi

if test x$AUTOMAKE != x; then
    VER=`$AUTOMAKE --version \
         | grep automake | sed "s/.* \([0-9.]*\)[-a-z0-9]*$/\1/"`
    check_version $VER $AUTOMAKE_REQUIRED_VERSION
    if test "$DIE" -eq 1; then
       DIE=0
       check_version $VER 1.10
    fi 
fi

if test "$DIE" -eq 1; then
    echo
    echo "Please install/upgrade the missing tools and call me again."
    echo	
    exit 1
fi


test $TEST_TYPE $FILE || {
    echo
    echo "You must run this script in the top-level $PROJECT directory."
    echo
    exit 1
}

if test -z "$*"; then
    echo
    echo "I am going to run ./configure with no arguments - if you wish "
    echo "to pass any to it, please specify them on the $0 command line."
    echo
fi

run "$ACLOCAL $ACLOCAL_FLAGS"
RC=$?
if test $RC -ne 0; then
   echo "$ACLOCAL gave errors. Please fix the error conditions and try again."
   exit 1
fi

run "libtoolize --force" || exit 1

# optionally feature autoheader
(autoheader --version)  < /dev/null > /dev/null 2>&1 && run "autoheader" || exit 1

run "$AUTOMAKE --add-missing" || exit 1

run "autoconf" || exit 1

cd $ORIGDIR

if $srcdir/configure --enable-maintainer-mode "$@"; then
  echo
  echo "Now type 'make' to compile $PROJECT."
else
  echo
  echo "Configure failed or did not finish!"
  exit 1
fi

