var bindings = require('bindings')('ruby_bridge.node'),
    util = require("util");

module.exports.newInstance = function() {
  var Cls = getClass(arguments[0]);
  return new (Cls.bind.apply(Cls, arguments))();
};

function addToProto(Cls, RubyClass) {
  // TODO: Is there a better/faster way to do this?
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
}

var getClass = module.exports.getClass = function(name) {
  var RubyClass = bindings._getClass(name);
  
  function Cls() {
    this._rubyObj = new RubyClass(undefined, arguments);
  }
  
  addToProto(Cls, RubyClass);
  
  return Cls;
};

module.exports.inherits = function(ctor, superName) {
  var RubyClass = bindings._defineClass(ctor.name, superName);
  
  function SuperCtor() {
    this._rubyObj = new RubyClass(this, arguments);
  }
  
  addToProto(SuperCtor, RubyClass);
  
  util.inherits(ctor, SuperCtor);
};

module.exports.require = bindings.require;
