var bindings = module.exports = require('bindings')('ruby_bridge.node'),
    util = require("util");

module.exports.newInstance = function() {
  var Cls = bindings.getClass(arguments[0]);
  return new (Cls.bind.apply(Cls, arguments))();
};

module.exports.inherits = function(ctor, superName) {
  var superCtor = bindings._rubyInherits(ctor, superName);
  util.inherits(ctor, superCtor);
};
