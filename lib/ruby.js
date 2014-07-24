var ruby = module.exports = require('bindings')('norby.node'),
    symbols = require('./symbols');
    
ruby.RubyValue.prototype.getConst = function(name) {
  var split = name.indexOf('::');
  if (split > -1) {
    var inner = this.callMethod(symbols.const_get,
                                ruby.jsStrToRuby(name.substring(0, split)));
    return inner.getConst(name.substring(split+2));
  }
  
  // TODO: Should we use symbols or strings here?
  return this.callMethod(symbols.const_get, ruby.jsStrToRuby(name));
};

ruby.getConst = function(name) {
  return ruby.Object.getConst(name);
};

ruby.rubyBoolToJS = function(val) {
  return val.getType() === ruby.types.TRUE;
};

// Ruby classes
ruby.Class = ruby.getConst('Class');
ruby.Array = ruby.getConst('Array');

// Ruby exceptions
ruby.ArgError = ruby.getConst('ArgumentError');
ruby.LoadError = ruby.getConst('LoadError');
ruby.NameError = ruby.getConst('NameError');
ruby.NoMethodError = ruby.getConst('NoMethodError');
ruby.TypeError = ruby.getConst('TypeError');
ruby.SyntaxError = ruby.getConst('SyntaxError');

// Ruby values
ruby.true = ruby.getConst('TRUE');
ruby.false = ruby.getConst('FALSE');
ruby.nil = ruby.getConst('NIL');

ruby.arrayForEach = function(rubyArr, cb) {
  var len = ruby.rubyFixnumToJS(rubyArr.callMethod(symbols.length));
  for (var i = 0; i < len; ++i) {
    var el = rubyArr.callMethod(symbols.at, ruby.jsNumToRuby(i));
    cb(el, i);
  }
};

var selfStr = ruby.jsStrToRuby('self');
ruby.getSelf = function() {
  return convert.rubyToJS(ruby.Object.callMethod(symbols.eval, selfStr,
    ruby.Object.callMethod(symbols.binding)));
};

var convert = require('./convert');

var callMethod = ruby.RubyValue.prototype.callMethod;
ruby.RubyValue.prototype.callMethod = function() {
  // TODO: Remove when confident?
  for (var i = 0; i < arguments.length; ++i) {
    if (!(arguments[i] instanceof ruby.RubyValue))
      throw new TypeError('arguments must be RubyValues');
  }

  var res = callMethod.apply(this, arguments);
  if (res.error)
    throw convert.rubyExToJS(res.error);
  
  return res;
};

var callMethodWithBlock = ruby.RubyValue.prototype.callMethodWithBlock;
ruby.RubyValue.prototype.callMethodWithBlock = function() {
  // TODO: Remove when confident?
  for (var i = 0; i < arguments.length-1; ++i) {
    if (!(arguments[i] instanceof ruby.RubyValue))
      throw new TypeError('arguments must be RubyValues');
  }
  
  var res = callMethodWithBlock.apply(this, arguments);
  if (res.error)
    throw convert.rubyExToJS(res.error);
    
  return res;
};

ruby.RubyValue.prototype.callMethodWithArgs = function(rbArgs) {
  return (rbArgs.block ? this.callMethodWithBlock.apply(this, rbArgs) :
                         this.callMethod.apply(this, rbArgs));
};

ruby.RubyValue.prototype.isA = function(cls) {
  return ruby.rubyBoolToJS(this.callMethod(symbols.is_a, cls));
};

ruby.RubyValue.prototype.toString = function() {
  return ruby.rubyStrToJS(this.callMethod(symbols.to_s));
};
