This file explains the build system for PLEXIL version 3.x, and how to build
PLEXIL and its related applications provided in this distribution.

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

Building PLEXIL requires two distinct steps.  First, build the executive.  Then,
build the supporting applications.


Building the PLEXIL Executive
-----------------------------

1. Set up your Unix environment.  Assuming you've placed PLEXIL in ~/plexil-3,
the following environment variables must be defined to build or use PLEXIL
conveniently.  You may wish to put them in your shell's init file.

For bash shell users:

  export PLEXIL_HOME=~/plexil-3
  export PATH=$PLEXIL_HOME/scripts:$PATH

For csh shell users:

  setenv PLEXIL_HOME ~/plexil-3
  setenv PATH $PLEXIL_HOME/scripts:$PATH

In addition, for MAC OS users only, the following addition to PATH will simplify
the use of some applications that use xterm, which is normally found in the Unix
search path.

  export PATH=/usr/X11/bin/$PLEXIL_HOME/bin:$PATH

or 

  setenv PATH /usr/X11/bin/$PLEXIL_HOME/bin:$PATH


2. Change to the source directory:

   cd plexil-3/src

3. Configure for build:

   ./configure --prefix=$PLEXIL_HOME

   The --prefix argument shown will install the generated PLEXIL binaries in
   your PLEXIL installation directory.  By default, they are installed in
   /usr/local, which may require superuser privelege and not be as convenient a
   location.  Addtional options to configure are described below under
   "Configure options".

3. Build the system:

   make

   If you encounter error messages similar to the following (which,
   unfortunately, is likely):

   libtool: ignoring unknown tag CXX
   libtool: Version mismatch error.  This is libtool 2.4.2 ..., but the
   libtool: definition of this LT_INIT comes from an older release.
   libtool: You should recreate aclocal.m4 with macros from libtool 2.4.2 ...
   libtool: and run autoconf again.

   Do the following:

   aclocal --force
   libtoolize --copy --force
   autoreconf
   ./config.status

   These steps may require you to install Gnu tools you don't have, in
   particular 'libtool'.  Follow the indications you see in what's printed at
   each command.

4. Install the system:

   make install


To be sorted out
----------------

 * If the GNU autotools installed on your system are a different
   version from the ones used to build this release, you may encounter
   errors running 'make'. The following commands can replace the bad
   files with compatible ones from your local installation:
     aclocal --force
     libtoolize --force --copy
     autoreconf --force --install
   You should be able to re-run 'make' successfully at this point.

 * The following subsystems in $PLEXIL_HOME/src are no longer built by
   default: 
     - the Test Executive
     - module tests
     - the UDP adapter
     - Gantt viewer support
     - IPC and modules which depend on it: Plexil Simulator, Robosim,
     IPC adapter, et al

   Most of these modules can be enabled via options to 'configure'.
   Type: 
      ./configure --help
   to see these options.  
   
   To include all currently supported PLEXIL systems:
      ./configure --enable-gantt --enable-module-tests --enable-test-exec --enable-udp



 * The default installation location for PLEXIL libraries and
   executables under the GNU autotools is /usr/local. The installation
   location can be specified with the '--prefix=' option to
   'configure'. We recommend installing these files in the PLEXIL
   installation directory itself. E.g.:
    ./configure --prefix=$PLEXIL_HOME [... more options ...]

 * By default, the GNU autotools build both dynamic and static
   libraries.  Static linking currently does not work on some
   systems. To save build time and disk space, we recommend that you
   build only the dynamic (shared) libraries on modern Linux and Mac
   platforms. This is done by supplying the '--disable-static' option
   to 'configure'.

 * The Plexil and Plexilscript compilers are not part of
   $PLEXIL_HOME/src, and need to be built separately. To do this:
    cd $PLEXIL_HOME/compilers/plexil
    make
    cd $PLEXIL_HOME/compilers/plexilscript
    ant

 * The Plexil example plans and applications (found in
   plexil-3/examples) are NOT compiled by default. Plans (.ple files)
   must be compiled manually (with plexilc) to produce .plx files. The
   application directories (examples/sample-app and examples/robosim)
   must be built by typing 'make' -- though note that 'robosim' cannot
   be built at present due to the lack of IPC support mentioned above.

 * For the manually-built PLEXIL subystems and applications mentioned
   above, the dynamic libraries produced (e.g. .dylib files in Mac OS,
   .so files in Linux) might not be automatically copied to, or linked
   to from, the directory in which they are expected to be found,
   which is $PLEXIL_HOME/lib. You may have to make these copies or
   symbolic links manually. 

