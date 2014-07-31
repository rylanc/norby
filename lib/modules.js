var ruby = require('./ruby'),
    convert = require('./convert'),
    symbols = require('./symbols'),
    ModuleMap = require('./module_map');
    
var modMap = new ModuleMap();

module.exports.convertClass = function(rubyClass) {
  var existing = modMap.get(rubyClass);
  if (existing)
    return existing;
  
  var Cls = getCtor(rubyClass);
  modMap.set(rubyClass, Cls);

  // Instance methods
  var instanceMethods = rubyClass.callMethod(symbols.instance_methods);
  ruby.arrayForEach(instanceMethods, function(methodSym) {
    var methodStr = methodSym.toString();
    Cls.prototype[methodStr] = function() {
      var rbArgs = convert.argsToRuby.apply(methodSym, arguments);
      return convert.rubyToJS(this._rubyObj.callMethodWithArgs(rbArgs));
    };
  });

  
  if (typeof Cls.prototype.to_s === 'function')
    Cls.prototype.toString = Cls.prototype.to_s;
  
  if (typeof Cls.prototype.inspect === 'function') {
    Cls.prototype.inspect = function() {
      return convert.rubyToJS(this._rubyObj.callMethod(symbols.inspect));
    };
  }
  
  addClassProperties(rubyClass, Cls);

  return Cls;
};

module.exports.convertModule = function(module) {
  var existing = modMap.get(module);
  if (existing)
    return existing;
    
  var jsMod = {};
  modMap.set(module, jsMod);
  addClassProperties(module, jsMod);

  return jsMod;
};

module.exports.newClass = function(ctor, superClass) {
  var rbName = ruby.jsStrToRuby(ctor.name);
  if (ruby.rubyBoolToJS(ruby.Object.callMethod(symbols.const_defined, rbName)))
    throw new TypeError('constant ' + ctor.name + ' already exists');
  
  var rubyClass = ruby.Class.callMethod(symbols.new, superClass);
  ruby.Object.callMethod(symbols.const_set, rbName, rubyClass);
  ctor.super_ = getCtor(rubyClass);
  
  var superCtor = module.exports.convertClass(superClass);
  ctor.prototype = Object.create(superCtor.prototype, {
    constructor: {
      value: ctor,
      enumerable: false,
      writable: true,
      configurable: true
    }
  });
  
  modMap.set(rubyClass, ctor);
};

function getCtor(rubyClass) {
  return function Cls() {
    var rbArgs = convert.argsToRuby.apply(symbols.new, arguments);
    this._rubyObj = rubyClass.callMethodWithArgs(rbArgs);
    this._rubyObj.setOwner(this);
  };
}

function addClassProperties(rbMod, jsMod) {
  // Class methods
  function addClassMethod(methodSym) {
    var methodStr = methodSym.toString();
    if (methodStr === 'new')
      return;
      
    if (methodStr === 'name' && rbMod.getType() === ruby.types.CLASS)
      methodStr = 'rubyName';
    
    jsMod[methodStr] = function() {
      var rbArgs = convert.argsToRuby.apply(methodSym, arguments);
      return convert.rubyToJS(rbMod.callMethodWithArgs(rbArgs));
    };
  }
  
  var singletonMethods = rbMod.callMethod(symbols.singleton_methods);
  ruby.arrayForEach(singletonMethods, addClassMethod);
  
  var clsInstanceMethods = rbMod.class().callMethod(symbols.instance_methods);
  ruby.arrayForEach(clsInstanceMethods, addClassMethod);
  
  if (typeof jsMod.to_s === 'function')
    jsMod.toString = jsMod.to_s;
  
  if (typeof jsMod.inspect === 'function') {
    jsMod.inspect = function() {
      return convert.rubyToJS(rbMod.callMethod(symbols.inspect));
    };
  }
  
  // Class constants
  var constants = rbMod.callMethod(symbols.constants);
  ruby.arrayForEach(constants, function(constSym) {
    var constStr = constSym.toString();
    jsMod[constStr] =
      convert.rubyToJS(rbMod.callMethod(symbols.const_get, constSym));
  });
}
