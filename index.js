var ruby = require('./lib/ruby'),
    convert = require('./lib/convert'),
    symbols = require('./lib/symbols'),
    modules = require('./lib/modules'),
    util = require('util');

var getClass = module.exports.getClass = function(name) {
  var rubyClass = ruby.getConst(name);
  if (!rubyClass.isA(ruby.Class))
    throw new TypeError(name + ' is not a class');

  return modules.convertClass(rubyClass);
};

module.exports.newInstance = function() {
  var Cls = getClass(arguments[0]);
  return new (Cls.bind.apply(Cls, arguments))();
};

var Proc = modules.convertClass(ruby.Proc);
module.exports.inherits = function(ctor, superName) {
  var superClass = ruby.getConst(superName);
  var rubyClass = ruby.Class.callMethod(symbols.new, superClass);
  ruby.Object.callMethod(symbols.getSym('const_set'), ruby.v8StrToRuby(ctor.name), rubyClass);
  var SuperCtor = modules.convertClass(rubyClass);
  //var SuperCtor = ruby._defineClass(ctor.name, superName);
  
  // TODO: Do class methods work here? Should they even work?
  
  util.inherits(ctor, SuperCtor);
  
  ctor.defineMethod = function(name, fn) {
    if (typeof fn !== 'function')
      throw new TypeError('fn must be a function: ' + fn);

    var proc = new Proc(function() {
      var self = convert.rubyToV8(ruby.Object.callMethod(symbols.eval,
        ruby.v8StrToRuby('self'),
        ruby.Object.callMethod(symbols.getSym('binding'))));
      console.log('self: ' + self.toString());
      return fn.apply(self, arguments);
    });

    SuperCtor._rubyMod.callMethod(symbols.getSym('define_method'),
      symbols.getSym(name), proc._rubyObj);
    this.prototype[name] = fn;
  };
};

module.exports.require = function(name) {
  ruby.Object.callMethod(symbols.require, ruby.v8StrToRuby(name));
};

// TODO: Other arguments. Also, we should add unit tests for those
module.exports.eval = function(code) {
  return convert.rubyToV8(ruby.Object.callMethod(symbols.eval,
    ruby.v8StrToRuby(code)));
};

module.exports.getMethod = function(name) {
  var methodSym = symbols.getSym(name);
  return function() {
    var rbArgs = convert.argsToRuby.apply(methodSym, arguments);
    return convert.rubyToV8(ruby.Object.callMethodWithArgs(rbArgs));
  };
};

module.exports.getConstant = function(name) {
  return convert.rubyToV8(ruby.getConst(name));
};
