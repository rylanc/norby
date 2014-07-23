var ruby = require('./ruby'),
    modules = require('./modules'),
    symbols = require('./symbols');

var rubyToJS = module.exports.rubyToJS = function(val) {
  //console.log('Class is ' + val.callMethod(symbols.class));
  //console.log('Converting ' + val + ' to JS');

  switch (val.type) {
    case ruby.types.NIL:
      return undefined;
    case ruby.types.FIXNUM:
      return ruby.rubyFixnumToJS(val);
    case ruby.types.FLOAT:
      return ruby.rubyFloatToJS(val);
    case ruby.types.STRING:
      return ruby.rubyStrToJS(val);
    case ruby.types.TRUE:
      return true;
    case ruby.types.FALSE:
      return false;
    case ruby.types.ARRAY: {
      var jsArr = [];
      ruby.arrayForEach(val, function(el) {
        jsArr.push(rubyToJS(el));
      });
    
      return jsArr;
    }
    case ruby.types.BIGNUM: {
      var jsVal = ruby.rubyFloatToJS(val.callMethod(symbols.to_f));
      if (jsVal === Infinity)
        jsVal = val.toString();
    
      return jsVal;
    }
    case ruby.types.CLASS:
      return modules.convertClass(val);
    case ruby.types.MODULE:
      return modules.convertModule(val);
    default: {
      var owner = val.getOwner();
      if (typeof owner !== 'undefined') {
        return owner;
      }
    
      var cls = val.callMethod(symbols.class);
      var ctor = modules.convertClass(cls);
      var jsObj = Object.create(ctor.prototype);
      jsObj._rubyObj = val;
      jsObj._rubyObj.setOwner(jsObj);
      return jsObj;
    }
  }
  
  console.error('Unknown ruby type: ' + val);
};

var jsToRuby = function(val) {
  switch (typeof val) {
    case 'string':
      return ruby.jsStrToRuby(val);
    case 'number':
      return ruby.jsNumToRuby(val);
    case 'undefined':
      return ruby.nil;
    case 'boolean':
      return (val ? ruby.true : ruby.false);
    case 'object':
      if (val === null)
        return ruby.nil;
      else if (Array.isArray(val)) {
        var rubyArr = ruby.Array.callMethod(symbols.new,
          ruby.jsNumToRuby(val.length));
        val.forEach(function(el, i) {
          var rubyVal = jsToRuby(el);
          rubyArr.callMethod(symbols.elem_assign, ruby.jsNumToRuby(i), rubyVal);
        });
        
        return rubyArr;
      }
      // TODO: Fix this
      else if (val._rubyObj instanceof ruby.RubyValue) {
        return val._rubyObj;
      }
      
      console.error("Unknown JS type: " + val + " (" + typeof val + ")");
      
      break;
    case 'function':
      if (val._rubyMod)
        return val._rubyMod;
      console.error("Unknown JS type: " + val + " (" + typeof val + ")");
  }
  
  return ruby.nil;
};

module.exports.rubyExToJS = function(rbErr) {
  var msg = ruby.rubyStrToJS(rbErr.callMethod(symbols.message));
  var jsErr;
  
  if (rbErr.isA(ruby.ArgError) || rbErr.isA(ruby.LoadError))
    jsErr = new Error(msg);
  else if (rbErr.isA(ruby.NameError) || rbErr.isA(ruby.NoMethodError))
    jsErr = new ReferenceError(msg);
  else if (rbErr.isA(ruby.TypeError))
    jsErr = new TypeError(msg);
  else if (rbErr.isA(ruby.SyntaxError))
    jsErr = new SyntaxError(msg);
  else {
    console.error('Unknown ruby exception: ' +
      rbErr.callMethod(symbols.class).toString());
    jsErr = new Error(msg);
  }

  jsErr.rubyStack = rubyToJS(rbErr.callMethod(symbols.backtrace));
  
  return jsErr;
};

function wrapBlockFunc(blockFunc) {
  return function() {
    var jsArgs = [];
    for (var i = 0; i < arguments.length; ++i) {
      jsArgs.push(rubyToJS(arguments[i]));
    }
    return jsToRuby(blockFunc.apply(this, jsArgs));
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
    rbArgs.push(jsToRuby(arguments[i]));
  }
  
  if (rbArgs.block) {
    rbArgs.push(wrapBlockFunc(arguments[arguments.length-1]));
  }
  
  return rbArgs;
};
