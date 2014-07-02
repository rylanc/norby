var ruby = require('../index'),
    readline = require('readline');

var replEval = ruby.getFunction('eval');
var TOPLEVEL_BINDING = ruby.getConstant('TOPLEVEL_BINDING');

var rl = readline.createInterface(process.stdin, process.stdout);

rl.setPrompt('> ');
rl.prompt();

rl.on('line', function(line) {
  try {
    var res = replEval(line.trim(), TOPLEVEL_BINDING);
    console.log('=> ' + res);
  } catch(e) {
    console.log(e.stack || e);
  }
  
  rl.prompt();
}).on('error', function(err) {
  console.log('Error! ' + err);
}).on('close', function() {
  console.log('Goodbye!');
});
