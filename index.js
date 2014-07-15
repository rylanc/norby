var bindings = require('bindings')('norby.node')(wrapExisting),
    util = require("util");

module.exports.newInstance = function() {
  var Cls = getClass(arguments[0]);
  return new (Cls.bind.apply(Cls, arguments))();
};

function wrapExisting(RubyClass) {
  return Object.create(getCtor(RubyClass).prototype);
}

// Because of the way node native addons work, we can't inherit directly from a
// wrapped C++ object. Holding the RubyObject as a member is the workaround.
function getCtor(RubyClass) {
  function Cls() {
    var args = [ this ];
    for (var i = 0; i < arguments.length; ++i) {
      args[i+1] = arguments[i];
    }
    this._rubyObj = RubyClass.new.apply(RubyClass, args);
  }
  
  // TODO: Is there a better/faster way to do this? Can we cache these protos?
  function proxyFunc(method) {
    return function() {
      return this._rubyObj[method].apply(this._rubyObj, arguments);
    };
  }
  
  RubyClass.instance_methods().forEach(function(method) {
    Cls.prototype[method] = proxyFunc(method);
  });
  
  Object.keys(RubyClass).forEach(function(key) {
    if (typeof RubyClass[key] === 'function')
      Cls[key] = RubyClass[key].bind(RubyClass);
    else
      Cls[key] = RubyClass[key];
  });
  
  if (typeof Cls.prototype.to_s === 'function') {
    Cls.prototype.toString = Cls.prototype.to_s;
  }
  
  if (typeof Cls.prototype.inspect === 'function') {
    Cls.prototype.inspect = function(depth) {
      return this._rubyObj.inspect();
    };
  }
  
  return Cls;
}

var getClass = module.exports.getClass = function(name) {
  var RubyClass = bindings._getClass(name);
  
  return getCtor(RubyClass);
};

module.exports.inherits = function(ctor, superName) {
  var RubyClass = bindings._defineClass(ctor.name, superName);
  var SuperCtor = getCtor(RubyClass);
  
  // TODO: Do class methods work here? Should they even work?
  
  util.inherits(ctor, SuperCtor);
  
  ctor.defineMethod = function(name, fn) {
    RubyClass._defineMethod(name, fn);
    this.prototype[name] = fn;
  };
};

module.exports.require = bindings.getMethod('require');
module.exports.eval = bindings.getMethod('eval');
module.exports.getMethod = bindings.getMethod;
module.exports.getConstant = bindings.getConstant;
