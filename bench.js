var Benchmark = require('benchmark'),
    ruby = require('./index');

var suite = new Benchmark.Suite;
//console.log(Benchmark.options);

function Derived() {
  Derived.super_.call(this);
}

ruby.require('./greet.rb');
var regular = ruby.newInstance('BenchmarkHelper');

ruby.inherits(Derived, 'BenchmarkHelper');
var inherited = new Derived();

// add tests
suite.add('Base', function() {
  return regular.go();
})
.add('Derived', function() {
  return inherited.go();
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
