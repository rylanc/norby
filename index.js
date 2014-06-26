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
  
  function proxyFunc(key) {
    return function() {
      return RubyClass.prototype[key].apply(this._rubyObj, arguments);
    };
  }
  
  var propNames = Object.getOwnPropertyNames(RubyClass.prototype);
  for (var i = 0; i < propNames.length; i++) {
    //SuperCtor.prototype[key] = RubyClass.prototype[key].bind(this._rubyObj);
    var key = propNames[i];
    SuperCtor.prototype[key] = proxyFunc(key);
    // var func = RubyClass.prototype[key];
    // SuperCtor.prototype[key] = function() {
    //   return RubyClass.prototype[key].call(this._rubyObj);
    // };
  }
  
  util.inherits(ctor, SuperCtor);
};

module.exports.inherits2 = function(ctor, superName) {
  var RubyClass = bindings._rubyInherits(ctor, superName);
  function SuperCtor() {
    //this._rubyObj = new RubyClass(this);
    this._rubyObj = new RubyClass();
  }
  
  function proxyFunc(func) {
    return function() {
      return func.apply(this._rubyObj, arguments);
    };
  }
  
  var propNames = Object.getOwnPropertyNames(RubyClass.prototype);
  for (var i = 0; i < propNames.length; i++) {
    //SuperCtor.prototype[key] = RubyClass.prototype[key].bind(this._rubyObj);
    var key = propNames[i];
    SuperCtor.prototype[key] = proxyFunc(RubyClass.prototype[key]);
    // var func = RubyClass.prototype[key];
    // SuperCtor.prototype[key] = function() {
    //   return RubyClass.prototype[key].call(this._rubyObj);
    // };
  }
  
  util.inherits(ctor, SuperCtor);
};
