// Because of the way node native addons work, we can't inherit directly from a
// wrapped C++ object. Holding the RubyObject as a member is the workaround.
module.exports.createCtor = function(RubyClass) {
  function Cls() {
    var args = [ this ];
    for (var i = 0; i < arguments.length; ++i) {
      args[i+1] = arguments[i];
    }
    this._rubyObj = RubyClass.new.apply(RubyClass, args);
  }
  
  Cls._rubyClass = RubyClass;
  
  // TODO: Is there a better/faster way to do this? Can we cache these protos?
  function proxyFunc(method) {
    return function() {
      return this._rubyObj[method].apply(this._rubyObj, arguments);
    };
  }
  
  // Prototype (instance methods)
  RubyClass.instance_methods().forEach(function(method) {
    Cls.prototype[method] = proxyFunc(method);
  });
  
  if (typeof Cls.prototype.to_s === 'function')
    Cls.prototype.toString = Cls.prototype.to_s;
  
  if (typeof Cls.prototype.inspect === 'function') {
    Cls.prototype.inspect = function(depth) {
      return this._rubyObj.inspect();
    };
  }
  
  // Class (class methods)
  Object.keys(RubyClass).forEach(function(key) {
    if (typeof RubyClass[key] === 'function')
      Cls[key] = RubyClass[key].bind(RubyClass);
    else
      Cls[key] = RubyClass[key];
  });
  
  if (typeof Cls.to_s === 'function')
    Cls.toString = Cls.to_s;
  
  if (typeof Cls.inspect === 'function') {
    Cls.inspect = function(depth) {
      return RubyClass.inspect();
    };
  }
  
  if (typeof RubyClass.name === 'function')
    Cls.rubyName = RubyClass.name.bind(RubyClass);
  
  return Cls;
};

// module.exports.wrapExisting = function(RubyClass) {
//   return Object.create(getCtor(RubyClass).prototype);
// };
