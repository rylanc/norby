var Benchmark = require('benchmark'),
    ruby = require('./index');

var suite = new Benchmark.Suite;
//console.log(Benchmark.options);

function Derived() {
  Derived.super_.call(this);
}

function Derived2() {
  Derived2.super_.call(this);
}

ruby.require('./greet.rb');
var regular = ruby.newInstance('BenchmarkHelper');

ruby.inherits(Derived, 'BenchmarkHelper');
var inherited = new Derived();

ruby.inherits2(Derived2, 'BenchmarkHelper');
var inherited2 = new Derived2();

// add tests
suite.add('Base', function() {
  return regular.go();
})
.add('Derived', function() {
  return inherited.go();
})
.add('Derived2', function() {
  return inherited2.go();
})
// add listeners
.on('cycle', function(event) {
  console.log(String(event.target));
})
.on('complete', function() {
  console.log('Fastest is ' + this.filter('fastest').pluck('name'));
})
// run async
.run({ 'async': true, maxTime: 10 });
