This file explains the build system for PLEXIL version 3.x, and how to build
PLEXIL and its related applications provided in this distribution.

See the CAVEATS file for addtional build-related information.

Overview
--------

The build process of this plexil-3 branch is *very* different from that of the
SVN trunk version of PLEXIL (and all pre-3 versions of PLEXIL.  This build
system was introduced to better support cross-platform builds. Unfortunately it
is more complex, and requires GNU tools that might not be installed on your
system by default. Here's a summary of the differences.

   Previous PLEXIL distributions:
     - use the generic 'make' facility
     - all of PLEXIL is built by typing 'make' in the top level 'plexil'
       directory

   plexil-3 branch:
     - uses a hybrid of GNU "autotools" (autoconf/automake/libtool/etc.),
       which auto-generate Makefiles, and generic 'make'.

     - supports cross-compilation via the --host option and various
       environment variables (for details, see the GNU autoconf
       documentation)  

     - building the PLEXIL executive is completely based on GNU autotools and
       requires several commands.  Building the related applications requires
       several more commands, and uses generic 'make'.


Building PLEXIL
---------------

Building PLEXIL requires two overall steps.  First, build the executive.  Then,
build the supporting applications.


Building the PLEXIL Executive
-----------------------------

1. Set up your Unix environment.  Assuming you've placed PLEXIL in ~/plexil-3
(substitute your actual installation directory below if needed), the following
environment variables must be defined to build or use PLEXIL conveniently.  You
may wish to put them in your shell's init file.

  A) PLEXIL_HOME variable.  This specifies where PLEXIL is installed.

     For bash shell users:

       export PLEXIL_HOME=~/plexil-3

     For csh shell users:

       setenv PLEXIL_HOME ~/plexil-3

  B) PATH variable.  For convenience in starting PLEXIL, its executable
  directory should be added to your Unix search path.  Use one of the following
  commands, depending on your shell.

  setenv PATH $PLEXIL_HOME/scripts:$PATH
  export PATH=$PLEXIL_HOME/scripts:$PATH

  C) PATH variable (MacOS only).  In addition, for MacOS users only, the X11
     directory should be added to your path if not already present, as some
     PLEXIL applications launch xterm windows.  Substitute one of the following
     commands for those in section B above.

     export PATH=/usr/X11/bin:$PLEXIL_HOME/scripts:$PATH
     setenv PATH /usr/X11/bin:$PLEXIL_HOME/scripts:$PATH

2. Change to the source directory:

   cd plexil-3/src

3. Configure for build:

   ./configure --prefix=$PLEXIL_HOME

   The --prefix argument shown will install the generated PLEXIL binaries in
   your PLEXIL installation directory.  By default, they are installed in
   /usr/local, which may require superuser privelege and not be as convenient a
   location.  

   Addtional options to configure of interest are described below under
   "Configure options".

3. Build the system:

   make

   If you encounter error messages similar to the following (which,
   unfortunately, is not unlikely):

   libtool: ignoring unknown tag CXX
   libtool: Version mismatch error.  This is libtool 2.4.2 ..., but the
   libtool: definition of this LT_INIT comes from an older release.
   libtool: You should recreate aclocal.m4 with macros from libtool 2.4.2 ...
   libtool: and run autoconf again.

   Do the following:

   aclocal --force
   libtoolize --force --copy
   autoreconf --force --install

   These steps may require you to install Gnu tools you don't have, in
   particular 'libtool'.  Follow the indications you see in what's printed at
   each command.

4. Install the system:

   make install


Building PLEXIL applications
----------------------------

All the PLEXIL-related applications provided in this distribution may be built
in one step.  Change to your top level PLEXIL installation directory and type
'make'.

See the Makefile in this directory if you wish to only build certain
applications.

Note that you must first build the PLEXIL executive, as described in the
previous section.


Configure options
-----------------

Some PLEXIL subsystems and related applications in $PLEXIL_HOME/src are no
longer built by default.  Most of these can be built easily as described in the
previous section.  

One subsystem, the executive module tests used for regression testing
(of interest mainly to PLEXIL developers), must be enabled by 'configure' prior
to building the PLEXIL executive:

  ./configure --enable-module-tests

Some, but not all, the remaining PLEXIL subsystems and applications, can also be
built by first configuring for them.  Type the following to see the options:

    ./configure --help

For example:

  ./configure --enable-gantt --enable-module-tests --enable-test-exec


By default, the GNU autotools build both dynamic and static ibraries.  Static
linking currently does not work on some systems. To save build time and disk
space, we recommend that you build only the dynamic (shared) libraries on modern
Linux and Mac platforms. This is done by supplying the '--disable-static' option
to 'configure'.


Misc
----

Most, but not all, PLEXIL subsystems and applications can easily build
individually.  It is easiest to follow the steps in the previous section,
however you may wish to build various subdirectories manually.  For example, the
Plexil and Plexilscript compilers can be built as follows.

    cd $PLEXIL_HOME/compilers/plexil
    make
    cd $PLEXIL_HOME/compilers/plexilscript
    ant

See the CAVEATS file for addtional build-related information.
