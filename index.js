var bindings = module.exports = require('bindings')('ruby_bridge.node'),
    util = require("util");

module.exports.newInstance = function() {
  var Cls = bindings.getClass(arguments[0]);
  return new (Cls.bind.apply(Cls, arguments))();
};

module.exports.inherits = function(ctor, superName) {
  var RubyClass = bindings._rubyInherits(ctor, superName);
  function SuperCtor() {
    //this._rubyObj = new RubyClass(this);
    this._rubyObj = new RubyClass();
  }
  
  // TODO: Is there a better/faster way to do this?
  function proxyFunc(func) {
    return function() {
      return func.apply(this._rubyObj, arguments);
    };
  }
  
  var propNames = Object.getOwnPropertyNames(RubyClass.prototype);
  for (var i = 0; i < propNames.length; i++) {
    var key = propNames[i];
    SuperCtor.prototype[key] = proxyFunc(RubyClass.prototype[key]);
  }
  
  util.inherits(ctor, SuperCtor);
};
