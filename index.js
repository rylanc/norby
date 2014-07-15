var ctors = require('./lib/ctors'),
    bindings = require('bindings')('norby.node')(ctors),
    util = require("util");

module.exports.newInstance = function() {
  var Cls = bindings._getClass(arguments[0]);
  return new (Cls.bind.apply(Cls, arguments))();
};

module.exports.inherits = function(ctor, superName) {
  var SuperCtor = bindings._defineClass(ctor.name, superName);
  
  // TODO: Do class methods work here? Should they even work?
  
  util.inherits(ctor, SuperCtor);
  
  ctor.defineMethod = function(name, fn) {
    if (typeof fn !== 'function')
      throw new TypeError('fn must be a function: ' + fn);
    
    SuperCtor._defineMethod(name, fn);
    this.prototype[name] = fn;
  };
};

module.exports.getClass = bindings._getClass;
module.exports.require = bindings.getMethod('require');
module.exports.eval = bindings.getMethod('eval');
module.exports.getMethod = bindings.getMethod;
module.exports.getConstant = bindings.getConstant;
