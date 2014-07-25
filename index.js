var ruby = require('./lib/ruby'),
    convert = require('./lib/convert'),
    symbols = require('./lib/symbols'),
    modules = require('./lib/modules');

var getClass = module.exports.getClass = function(name) {
  var rubyClass = ruby.getConst(name);
  if (rubyClass.getType() !== ruby.types.CLASS)
    throw new TypeError(name + ' is not a class');

  return modules.convertClass(rubyClass);
};

module.exports.newInstance = function() {
  var Cls = getClass(arguments[0]);
  return new (Cls.bind.apply(Cls, arguments))();
};

var Proc = getClass('Proc');
module.exports.inherits = function(ctor, superName) {
  var superClass = ruby.getConst(superName);
  modules.newClass(ctor, superClass);
  
  ctor.defineMethod = function(name, fn) {
    if (typeof fn !== 'function')
      throw new TypeError('fn must be a function: ' + fn);

    var proc = new Proc(function() {
      var self = ruby.getSelf();
      return fn.apply(self, arguments);
    });

    ctor._rubyMod.callMethod(symbols.define_method, symbols.getSym(name),
      proc._rubyObj);
    this.prototype[name] = fn;
  };
};

var getMethod = module.exports.getMethod = function(name) {
  var methodSym = symbols.getSym(name);
  return function() {
    var rbArgs = convert.argsToRuby.apply(methodSym, arguments);
    return convert.rubyToJS(ruby.Object.callMethodWithArgs(rbArgs));
  };
};

module.exports.getConstant = function(name) {
  return convert.rubyToJS(ruby.getConst(name));
};

module.exports.require = getMethod('require');
module.exports.eval = getMethod('eval');
