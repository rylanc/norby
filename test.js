var ruby = require('./build/Debug/ruby'),
    util = require('util');

//var time = ruby.newInstance('Time', 1990);

//time['eql?'](2);

//var arr = time.to_a();
//console.log(arr);

//console.log(time.to_s());

function TestClass() {
  this.bla = 'bla';
}

TestClass.prototype.call_me = function() {
  console.log("in JS!");
}

ruby.require('./greet.rb');

var t = ruby.inherits(TestClass, 'Greeter');
t.try();
