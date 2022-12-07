# Updating plexil-6 with work from the plexil-4 family of branches

plexil-6 hasn't had much attention since 2020, while the plexil-4
branches have had quite a few improvements and bug fixes.  It's time
to bring plexil-6 up to date.

Meanwhile, the PLEXIL-73 branch (based on releases/plexil-4.6) has
undergone some significant changes itself.  Some of the lessons
learned there may be applicable to plexil-6 as well.

## Surveying the changes

### Build system

* Top level Makefile successfully updated.

* makeinclude directory changes require careful review

* GNU autotools build system has been fairly stable, but we can remove
  some of the checks for C include files.

* CMake build structure had to be overhauled to build PLEXIL as an
  external project.  Most of those changes should be rolled back into
  plexil-4.6 as well.

* ...And several additional changes had to be merged forward, notably
  setting RPATH (RUNPATH) on shared libraries.

* Found and fixed problems with the UdpAdapter in the CMake statically
  linked build.

### PLEXIL Executive

* Most significant change is the source level documentation added in
  the PLEXIL-73 work.

* Also worth importing from PLEXIL-73: some of the tweaks to abstract
  base classes.

* General issue of `#if defined(HAVE_Cxxxx)` needing to go away.  It's
  a **standard** include file, there shouldn't be a need to test for
  it!

#### src/utils

* Error.hh: Updated PLEXIL_NORETURN macro to use
  `__has_cpp_attribute()`.  See
  https://en.cppreference.com/w/cpp/language/attributes/noreturn

* Removed `#ifdef PLEXIL_NO_ERROR_EXCEPTIONS` from Error.cc.

* LinkedQueue.hh has had Doxygen comments added from PLEXIL-73 branch.

* Unit tests reviewed.

* Cleaned up conditional includes.

#### src/value

Sadly, `std::variant` is a C++17 feature.  (Maybe can adopt for
develop branch?)

* Doxygen comments added from PLEXIL-73 branch.

* CommandHandle has 2 new values: `COMMAND_ABORTED` and
  `COMMAND_ABORT_FAILED` (Done)

* Value class constructor for typed unknown now only takes one
  parameter, the ValueType. (Done)

* Unit tests updated.

* Maybe later: think about using templates to reduce boilerplate.

#### src/expr

* Would seem to be a lot of opportunity for refactoring with
  templates.  Think about where function templates would make more
  sense.

* PLEXIL-73 work removed constructors, destructor from these stateless
  abstract base classes: Expression, Assignable, GetValueImpl
  (class template),

* ExpressionListener, Listenable, and Operator in PLEXIL-73 still have
  virtual destructors, though they have no state of their own, because
  they are base classes with many widely used derived classes.
  (Done.)

* Active checks were deliberately removed in 2016.  See plexil-6
  (plexil-x?) commit 160a0196ba7e79d981446f465d87cdcd03e9ea72.

* checkArgCount(), checkArgTypes() methods on operator classes.  The
  latter were added in a 2016 plexil-4 commit:
  a597d3897cc1305b0a49218c526f137d44890d3b.  (Done.)

* New Variable abstract base class in plexil-6 defines API for
  concrete variables.  The change of return type for
  Assignable::getBaseVariable() from Assignable to Variable in
  plexil-6 was deliberate, in 2021: see commit
  460efb055f6f0ac12f2ade0d870e67c645bc6635.

* ArrayVariableImpl initializer type checks seem to have been removed
  from plexil-4 branch.  This would be consistent with moving as much
  error checking to the parser as practical.

* NodeConnector class moved from src/expr to src/intfc in 2018, then
  to src/exec (plexil-4), back in 2021 (plexil-6)!  It belongs here
  because it's referenced from Reservable.cc.

* Updates seem to be complete and working.

#### src/intfc

This is one area where plexil-6 has evolved considerably from
plexil-4.6, so caution is even more important here.

* CachedValue is a pure virtual base class in plexil-6.

* Better use of type aliases (e.g. Integer) in plexil-4.6 version of
  CachedValueImpl.cc.

* Command class looks to have evolved quite a bit in plexil-4.6
  relative to plexil-6.  Worth a second look.

* CommandFunction, CommandOperator classes in plexil-6 had far more
  complexity than was justified, and have now been trimmed back.
  CommandOperatorImpl has been deleted altogether.

* CommandHandleVariable in plexil-6 had not been updated to use
  CommandHandle enum!  (Fixed.)

* Some differences in Lookup classes may reflect improvements in
  plexil-4.6?

* As part of this update, commandUtils.hh was merged into Command.hh,
  and commandUtils.cc was merged into CommandImpl.cc.

* As part of this work, the Update class was split into interface and
  implementation, and the implementation class moved into the Exec
  module.

* Resource arbitration has now been gone over with a fine toothed
  comb, tuned somewhat, and documentation has been added.

#### src/exec

* Significant refactoring of Node classes in plexil-4.6.  Maybe more
  significant in PLEXIL-73 branch.

* PLEXIL-73 moved NodeConnector class here.  plexil-6 moved it back to
  the expr module.  Leave it there.

* PLEXIL-73 made Node a pure virtual base class.  Not even a
  destructor.  This is the interface seen by e.g. expressions and
  interface modules.

* Concrete Node classes in plexil-4.6 are based on NodeImpl.

* ExecConnector class in plexil-6 - what is it there for?

* PlexilExec class in plexil-4.6 is abstract base class.
  Implementation moved entirely to PlexilExecImpl class.

* Must preserve the updated state transition algorithm in plexil-6.

#### src/xml-parser

#### Application framework

#### Standard interface library

* IPC build system had to be updated again for Linux.

#### TestExec

### TestExec test suite

### Schema

### Schema validator

* Changes appear negligible.

* Probably needs updating for latest version of xmlschema package.

### Compilers and translators

* Jar files need updating: ANTLR 3 (done), Saxon HE (see
  feature/PLEXIL-123 branch)
  
* Standard PLEXIL compiler has been updated with ant build system.
  PlexilCompiler and PlexilCompilerDebug scripts have also been
  updated.

* Standard PLEXIL compiler merger in progress - many changes in
  plexil-4 branches, but the mutex features introduced in plexil-6
  must be retained.

* Plexilscript compiler?

* Extended PLEXIL translator also requires some care.

### Scripts

* plexilc script updated

### Examples



### Utilities

No changes in plexil-mode.el.  (Though it's out of date relative to
recent GNU Emacs releases.)

### Workshop

Horribly out of date in both releases.
