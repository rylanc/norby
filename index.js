var bindings = require('bindings')('ruby_bridge.node'),
    util = require("util");

module.exports.newInstance = function() {
  var Cls = getClass(arguments[0]);
  return new (Cls.bind.apply(Cls, arguments))();
};

function getCtor(RubyClass) {
  function Cls() {
    this._rubyObj = new RubyClass(this, undefined, arguments);
  }
  
  // TODO: Is there a better/faster way to do this? Can we cache these protos?
  function proxyFunc(func) {
    return function() {
      return func.apply(this._rubyObj, arguments);
    };
  }
  
  var propNames = Object.getOwnPropertyNames(RubyClass.prototype);
  for (var i = 0; i < propNames.length; i++) {
    var key = propNames[i];
    Cls.prototype[key] = proxyFunc(RubyClass.prototype[key]);
  }
  
  if (typeof Cls.prototype.to_s !== 'undefined') {
    Cls.prototype.toString = Cls.prototype.to_s;
  }
  
  if (typeof Cls.prototype.inspect !== 'undefined') {
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
  
  util.inherits(ctor, SuperCtor);
};

module.exports.require = bindings.require;
module.exports.eval = bindings.eval;
module.exports.getFunction = bindings.getFunction;
