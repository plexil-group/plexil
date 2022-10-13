# Updating plexil-6 with work from the plexil-4 family of branches

plexil-6 hasn't had much attention since 2020, while the plexil-4
branches have had quite a few improvements and bug fixes.  It's time
to bring plexil-6 up to date.

Meanwhile, the PLEXIL-73 branch (based on releases/plexil-4.6) has
undergone some significant changes itself.  Some of the lessons
learned there may be applicable to plexil-6 as well.

## Surveying the changes

### Build system

* Minor tweaks to the top level Makefile

* makeinclude directory changes require careful review

* GNU autotools build system has been fairly stable

* There's a new top level CMakeLists.txt file

### PLEXIL Executive

* Most significant change is the source level documentation added in
the PLEXIL-73 work.

* General issue of `#if defined(HAVE_Cxxxx)` needing to go away.  It's
  a **standard** include file, there shouldn't be a need to test for
  it!

#### src/utils

* Error.hh, .cc need a careful review

* LinkedQueue.hh needs review

* ParserException?

* ThreadSemaphore?

#### src/value

Sadly, `std::variant` is a C++17 feature.

* CommandHandle has 2 new values: `COMMAND_ABORTED` and `COMMAND_ABORT_FAILED`

* Minor differences in Value.cc?

* Maybe later: think about using templates to reduce boilerplate

#### src/expr

#### src/intfc

#### src/exec

#### src/xml-parser

#### Application framework and standard interface library

#### TestExec

### TestExec test suite

### Schema

* 

### Schema validator

Changes appear negligible.

### Compilers and translators

* Jar files need updating: ANTLR 3, Saxon HE (see feature/PLEXIL-123 branch)

* Standard PLEXIL compiler needs to be merged - many changes in
  plexil-4 branches, but the mutex features introduced in plexil-6
  must be retained.
  
* Plexilscript compiler?

* Extended PLEXIL translator also requires some care

### Scripts



### Examples



### Utilities

No changes in plexil-mode.el.  (Though it's out of date relative to
recent GNU Emacs releases.)

### Workshop

Horribly out of date in both releases.
