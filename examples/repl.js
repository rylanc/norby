var ruby = require('../index'),
    repl = require('repl');
    
var TOPLEVEL_BINDING = ruby.getConstant('TOPLEVEL_BINDING');

repl.start({
  prompt: ruby.getConstant('RUBY_VERSION') + ' > ',
  eval: function(cmd, context, filename, callback) {
    var err, result;
    try {
      result = ruby.eval(cmd.replace(/[()]/g, ''), TOPLEVEL_BINDING);
    } catch (e) {
      err = e;
    }
    
    callback(err, result);
  }
});
