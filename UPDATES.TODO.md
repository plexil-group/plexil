# Updating plexil-6 with work from the plexil-4 family of branches

plexil-6 hasn't had much attention since 2020, while the plexil-4
branches have had quite a few improvements and bug fixes.  It's time
to bring plexil-6 up to date.

Meanwhile, the PLEXIL-73 branch (based on releases/plexil-4.6) has
undergone some significant changes itself.  Some of the lessons
learned there may be applicable to plexil-6 as well.

## Surveying the changes

### Build system

* Minor tweaks to the top level Makefile.  First pass complete, some
  changes must wait for other build system updates.

* makeinclude directory changes require careful review

* GNU autotools build system has been fairly stable

* There's a new top level CMakeLists.txt file, relocated from src
  directory

### PLEXIL Executive

* Most significant change is the source level documentation added in
  the PLEXIL-73 work.

* General issue of `#if defined(HAVE_Cxxxx)` needing to go away.  It's
  a **standard** include file, there shouldn't be a need to test for
  it!

#### src/utils

* Error.hh: Updated PLEXIL_NORETURN macro to use
  `__has_cpp_attribute()`.  See
  https://en.cppreference.com/w/cpp/language/attributes/noreturn

* Removed `#ifdef PLEXIL_NO_ERROR_EXCEPTIONS` from Error.cc.

* LinkedQueue.hh has had Doxygen comments added from PLEXIL-73 branch.

* Review unit tests.

#### src/value

Sadly, `std::variant` is a C++17 feature.  (Maybe can adopt for
develop branch?)

* CommandHandle has 2 new values: `COMMAND_ABORTED` and
  `COMMAND_ABORT_FAILED`

* Minor differences in Value.cc?

* Maybe later: think about using templates to reduce boilerplate

#### src/expr

* Would seem to be a lot of opportunity for refactoring with
  templates.

* PLEXIL-73 work removed constructors, destructor from these stateless
  abstract base classes: Expression, Assignable, GetValueImpl
  (class template),

* ExpressionListener, Listenable, and Operator in PLEXIL-73 still have
  virtual destructors, though they has no state of their own.
  Probably because they are bases of widely used types.  Expression
  derives from Listenable, so this is probably the rationale I used.

* Active checks were deliberately removed in 2016.  See plexil-6
  (plexil-x?)  commit 160a0196ba7e79d981446f465d87cdcd03e9ea72.

* checkArgCount(), checkArgTypes() methods on operator classes.  The
  latter were added in a 2016 plexil-4 commit:
  a597d3897cc1305b0a49218c526f137d44890d3b.

* Variable (base class) in plexil-6 defines API for concrete
  variables.

* The change of return type for Assignable::getBaseVariable() from
  Assignable to Variable in plexil-6 was deliberate, in 2021: see
  commit 460efb055f6f0ac12f2ade0d870e67c645bc6635.

* ArrayVariableImpl initializer type checks seem to have been removed
  from plexil-4 branch.  This would be consistent with moving as much
  error checking to the parser as practical.

* NodeConnector class moved from src/expr to src/intfc in 2018, then
  to src/exec (plexil-4), back in 2021 (plexil-6)!

#### src/intfc

This is one area where plexil-6 has evolved considerably from
plexil-4.6, so caution is even more important here.

* CachedValue is a pure virtual base class in plexil-6.

* Better use of type aliases (e.g. Integer) in plexil-4.6 version of
  CachedValueImpl.cc.
  
* Command class looks to have evolved quite a bit in plexil-4.6
  relative to plexil-6.

* CommandFunction, CommandOperator classes in plexil-4.6.

* CommandHandleVariable in plexil-6 has not been updated to use
  CommandHandle enum!

* Some differences in Lookup classes may reflect improvements in
  plexil-4.6.

* Resource arbitration will require a careful review.

#### src/exec

* Significant refactoring of Node classes in plexil-4.6.  Maybe more
  significant in PLEXIL-73 branch.

* PLEXIL-73 moved NodeConnector class here.

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

#### TestExec

### TestExec test suite

### Schema

* 

### Schema validator

Changes appear negligible.

### Compilers and translators

* Jar files need updating: ANTLR 3, Saxon HE (see feature/PLEXIL-123
  branch)

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
