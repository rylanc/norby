var ruby = require('./ruby'),
    modules = require('./modules'),
    symbols = require('./symbols');

var rubyToV8 = module.exports.rubyToV8 = function(val) {
  //console.log('Class is ' + val.callMethod(symbols.class));
  //console.log('Converting ' + val + ' to v8');
  
  // TODO: Should we have a 'getType' method that returns an int? It would be
  // much more efficient, but it would mean more C++ code...
  if (val.isA(ruby.Fixnum) || val.isA(ruby.Float))
    return ruby.rubyNumToV8(val);
  else if (val.isA(ruby.String))
    return ruby.rubyStrToV8(val);
  else if (val.isA(ruby.TrueClass))
    return true;
  else if (val.isA(ruby.FalseClass))
    return false;
  else if (val.isA(ruby.Bignum)) {
    var v8Val = ruby.rubyNumToV8(val);
    if (v8Val === Infinity)
      v8Val = val.toString();
    
    return v8Val;
  }
  else if (val.isA(ruby.Array)) {
    var v8Arr = [];
    ruby.arrayForEach(val, function(el) {
      v8Arr.push(rubyToV8(el));
    });
    
    return v8Arr;
  }
  else if (val.isNil())
    return undefined;
  else if (val.isA(ruby.Class))
    return modules.convertClass(val);
  else if (val.isA(ruby.Module)) {
    return modules.convertModule(val);
  }
  else if (val.isA(ruby.Object)) {
    var owner = val.getOwner();
    if (typeof owner !== 'undefined') {
      return owner;
    }
    
    var cls = val.callMethod(symbols.class);
    var ctor = modules.convertClass(cls);
    var v8Obj = Object.create(ctor.prototype);
    v8Obj._rubyObj = val;
    v8Obj._rubyObj.setOwner(v8Obj);
    return v8Obj;
  }
  else {
    console.error('Unknown ruby type: ' + val);
  }
};

var v8ToRuby = module.exports.v8ToRuby = function(val) {
  switch (typeof val) {
    case 'string':
      return ruby.v8StrToRuby(val);
    case 'number':
      return ruby.v8NumToRuby(val);
    case 'undefined':
      return ruby.nil;
    case 'boolean':
      return (val ? ruby.true : ruby.false);
    case 'object':
      if (val === null)
        return ruby.nil;
      else if (Array.isArray(val)) {
        var rubyArr = ruby.Array.callMethod(symbols.new,
          ruby.v8NumToRuby(val.length));
        val.forEach(function(el, i) {
          var rubyVal = v8ToRuby(el);
          rubyArr.callMethod(symbols.elem_assign, ruby.v8NumToRuby(i), rubyVal);
        });
        
        return rubyArr;
      }
      // TODO: Fix this
      else if (val._rubyObj instanceof ruby.RubyValue) {
        return val._rubyObj;
      }
      
      console.error("Unknown v8 type: " + val + " (" + typeof val + ")");
      
      break;
    case 'function':
      if (val._rubyMod)
        return val._rubyMod;
      console.error("Unknown v8 type: " + val + " (" + typeof val + ")");
  }
  
  return ruby.nil;
};

module.exports.rubyExToV8 = function(rbErr) {
  var msg = ruby.rubyStrToV8(rbErr.callMethod(symbols.message));
  var v8Err;
  
  if (rbErr.isA(ruby.ArgError) || rbErr.isA(ruby.LoadError))
    v8Err = new Error(msg);
  else if (rbErr.isA(ruby.NameError) || rbErr.isA(ruby.NoMethodError))
    v8Err = new ReferenceError(msg);
  else if (rbErr.isA(ruby.TypeError))
    v8Err = new TypeError(msg);
  else if (rbErr.isA(ruby.SyntaxError))
    v8Err = new SyntaxError(msg);
  else {
    console.error('Unknown ruby exception: ' +
      rbErr.callMethod(symbols.class).toString());
    v8Err = new Error(msg);
  }

  v8Err.rubyStack = rubyToV8(rbErr.callMethod(symbols.backtrace));
  
  return v8Err;
};

function wrapBlockFunc(blockFunc) {
  return function() {
    var v8Args = [];
    for (var i = 0; i < arguments.length; ++i) {
      v8Args.push(rubyToV8(arguments[i]));
    }
    return v8ToRuby(blockFunc.apply(this, v8Args));
  };
}

module.exports.argsToRuby = function() {
  var rbArgs = [ this ];
  var len = arguments.length;
  
  if (arguments.length > 0 &&
      typeof arguments[arguments.length-1] === 'function' &&
      !arguments[arguments.length-1]._rubyMod) {
    --len;
    rbArgs.block = true;
  }
  
  for (var i = 0; i < len; ++i) {
    rbArgs.push(v8ToRuby(arguments[i]));
  }
  
  if (rbArgs.block) {
    rbArgs.push(wrapBlockFunc(arguments[arguments.length-1]));
  }
  
  return rbArgs;
};
