var ruby = require('./ruby'),
    convert = require('./convert'),
    symbols = require('./symbols');
    
var modMap = {
  _buckets: {},
  set: function(rbMod, jsMod) {
    var hash = this.hash(rbMod);
    var bucket = this._buckets[hash];
    if (!bucket)
      bucket = this._buckets[hash] = [];
      
    jsMod._rubyMod = rbMod;
    bucket.push(jsMod);
  },
  get: function(rbMod) {
    var bucket = this._buckets[this.hash(rbMod)];
    if (!bucket)
      return null;
    
    for (var i = 0; i < bucket.length; i++) {
      if (ruby.rubyBoolToJS(rbMod.callMethod(symbols.equal, bucket[i]._rubyMod)))
        return bucket[i];
    }
    
    return null;
  },
  hash: function(rbMod) {
    return ruby.rubyFixnumToJS(rbMod.callMethod(symbols.hash));
  }
};

function addClassProperties(rbMod, jsMod) {
  // Class methods
  function addClassMethod(methodSym) {
    var methodStr = methodSym.toString();
    if (methodStr == 'new')
      return;
      
    // TODO: Fix this
    if (methodStr == 'name' && rbMod.getType() === ruby.types.CLASS)
      methodStr = 'rubyName';
    
    jsMod[methodStr] = function() {
      var rbArgs = convert.argsToRuby.apply(methodSym, arguments);
      return convert.rubyToJS(rbMod.callMethodWithArgs(rbArgs));
    };
  }
  
  var singletonMethods = rbMod.callMethod(symbols.singleton_methods);
  ruby.arrayForEach(singletonMethods, addClassMethod);
  
  var clsInstanceMethods =
    rbMod.callMethod(symbols.class).callMethod(symbols.instance_methods);
  ruby.arrayForEach(clsInstanceMethods, addClassMethod);
  
  if (typeof jsMod.to_s === 'function')
    jsMod.toString = jsMod.to_s;
  
  // TODO: Is this right?
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

// Because of the way node native addons work, we can't inherit directly from a
// wrapped C++ object. Holding the RubyValue as a member is the workaround.
module.exports.convertClass = function(rubyClass) {
  var existing = modMap.get(rubyClass);
  if (existing)
    return existing;
  
  function Cls() {
    var rbArgs = convert.argsToRuby.apply(symbols.new, arguments);
    this._rubyObj = rubyClass.callMethodWithArgs(rbArgs);
    this._rubyObj.setOwner(this);
  }

  // Prototype (instance methods)
  var instanceMethods = rubyClass.callMethod(symbols.instance_methods);
  ruby.arrayForEach(instanceMethods, function(methodSym) {
    var methodStr = methodSym.toString();
    // TODO: Is there a better/faster way to do this?
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
  
  modMap.set(rubyClass, Cls);
  return Cls;
};

module.exports.convertModule = function(module) {
  var existing = modMap.get(module);
  if (existing)
    return existing;
    
  var jsMod = {};
  addClassProperties(module, jsMod);
  
  modMap.set(module, jsMod);
  return jsMod;
};

// TODO: newClass?
module.exports.defineClass = function(ctor, superClass) {
  var rbName = ruby.jsStrToRuby(ctor.name);
  if (ruby.rubyBoolToJS(ruby.Object.callMethod(symbols.const_defined, rbName)))
    throw new TypeError('const ' + ctor.name + ' already exists');
  
  var rubyClass = ruby.Class.callMethod(symbols.new, superClass);
  ruby.Object.callMethod(symbols.const_set, rbName, rubyClass);
  ctor.super_ = function() {
    var rbArgs = convert.argsToRuby.apply(symbols.new, arguments);
    this._rubyObj = rubyClass.callMethodWithArgs(rbArgs);
    this._rubyObj.setOwner(this);
  };
  
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
