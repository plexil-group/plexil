This file explains how to build and install PLEXIL version 2.x and its related
applications included in this distribution.


Start Here
----------

Everyone must set up their Unix environment to build and run PLEXIL.  Complete
the Setup section below before proceeding.

If this is a prebuilt distribution of PLEXIL, you are now ready to run PLEXIL.
See the Running PLEXIL section below.

If you have checked out PLEXIL as source code, you must first build it.  See the
Building PLEXIL section below.  Then see the Running PLEXIL section.


Setup
-----

Assuming you've placed PLEXIL in ~/plexil (substitute your actual installation
directory below if needed), the following environment variables must be defined
to build or use PLEXIL.  You may wish to put them in your shell's init file.

A) PLEXIL_HOME variable.  This specifies where PLEXIL is installed.

   For bash shell users:

   export PLEXIL_HOME=~/plexil

   For csh shell users:

   setenv PLEXIL_HOME ~/plexil

B) PATH variable.  For convenience in starting PLEXIL, its executable directory
   should be added to your Unix search path.  Use one of the following commands,
   depending on your shell.

   setenv PATH $PLEXIL_HOME/bin:$PATH
   export PATH=$PLEXIL_HOME/bin:$PATH

C) PATH variable (MacOS only).  In addition, for MacOS users only, the X11
   directory should be added to your path if not already present, as some
   PLEXIL applications launch xterm windows.  Substitute one of the following
   commands for those in section B above.

   export PATH=/usr/X11/bin:$PLEXIL_HOME/bin:$PATH
   setenv PATH /usr/X11/bin:$PLEXIL_HOME/bin:$PATH


Building PLEXIL
---------------

You can skip this section if you have a pre-built distribution and simply want
to run PLEXIL.  In this case see the Running PLEXIL section below.

To build PLEXIL and its related applications, change to ~/plexil and type
'make'.


Running PLEXIL
--------------

This section describes briefly, using specific examples, how to run the main
PLEXIL applications.  See the PLEXIL reference manual on Sourceforge
(plexil.sourceforge.net) for a complete description.

* Execute a plan using the Test Executive.  Change to <tt>plexil/examples/basic</tt>
  and type:

  plexiltest -p DriveToTarget.plx

  You can try this with other <tt>.plx</tt> files in this directory.

* Compile a plan into XML.  E.g. for the plan above:

  plexilc DriveToTarget.ple

* Execute a plan with the Plexil Viewer.  Using the same example:

  plexiltest -v -p DriveToTarget.plx

  The Plexil Viewer should open and display the running plan.
