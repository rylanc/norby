var ruby = require('./ruby');

var symbols = module.exports = {
  getSym: function(name) {
    var sym = symbols[name];
    if (typeof sym === 'undefined')
      sym = symbols[name] = ruby.getSymbol(name);
    
    return sym;
  }
};

symbols.getSym('const_get');
symbols.getSym('const_set');
symbols.getSym('constants');
symbols.getSym('class');
symbols.getSym('to_s');
symbols.getSym('to_f');
symbols.getSym('inspect');
symbols.getSym('require');
symbols.getSym('eval');
symbols.getSym('new');
symbols.getSym('length');
symbols.getSym('at');
symbols.getSym('instance_methods');
symbols.getSym('singleton_methods');
symbols.getSym('define_method');
symbols.getSym('hash');
symbols.getSym('message');
symbols.getSym('backtrace');
symbols.getSym('binding');

symbols.elem_assign = ruby.getSymbol('[]=');
symbols.is_a = ruby.getSymbol('is_a?');
symbols.equal = ruby.getSymbol('equal?');
symbols.const_defined = ruby.getSymbol('const_defined?');
