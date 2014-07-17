0.1.2 / 2014-07-16
==================

 * Fixed conversion of Bignums
 * FIxed a crash when a class/module has a constant of itself (e.g. Object)
 * Fixed a potential crash on initialization

0.1.1 / 2014-07-15
==================

 * Added support for Ruby modules
 * Added support for Ruby class constants to .getClass
 * Fixed passing Ruby classes as arguments
 * Added conversion of returned Ruby classes
 * Fixed .toString and .inspect for Ruby classes
 * Added caching of wrapped Ruby classes and modules
 * .eval now directly calls Kernel#eval
 * Updated documentation
 * Updated repl.js example

0.1.0 / 2014-07-09
==================

 * Initial release
