var expect = require('chai').expect,
    ruby = require('../index');
    
describe('passing blocks', function() {
  ruby.require('./test/helpers');
  var BlockTester = ruby.getClass('BlockTester');
  
  describe('one', function() {
    it('should yield', function(done) {
      var tester = new BlockTester();
      tester.yield_one(function(arg) {
        expect(arg).to.equal('hello');
        done();
      });
    });
  });
  
  describe('two', function() {
    it('should yield with one arg', function(done) {
      var tester = new BlockTester();
      tester.yield_two(function(arg1) {
        expect(arg1).to.equal('hello');
        done();
      });
    });
    
    it('should yield with two args', function(done) {
      var tester = new BlockTester();
      tester.yield_two(function(arg1, arg2) {
        expect(arg1).to.equal('hello');
        expect(arg2).to.equal(2);
        done();
      });
    });
  });
  
  describe('iterating', function() {
    it('should yield', function() {
      var tester = new BlockTester();
      var i = 0;
      tester.iter(function(arg1, arg2) {
        expect(arg1).to.equal('hi');
        expect(arg2).to.equal(i++);
      });
    });
  });
  
  describe('with argument', function() {
    it('should yield', function(done) {
      var tester = new BlockTester();
      var val = 'stan';
      tester.echo(val, function(arg) {
        expect(arg).to.equal(val);
        done();
      });
    });
  });
  
  describe('persistent', function() {
    it('should hold onto the passed function for the lifetime of the block',
    function() {
      var tester = new BlockTester();
      tester.define_singleton_method('my_method', function() {
        return 12345;
      });
      var result = tester.send('my_method');
      expect(result).to.equal(12345);
    });
  });
  
  describe('throwing exceptions', function() {
    it('should work with uncaught exception handlers', function(done) {
      var d = require('domain').create();
      d.on('error', function() {});
      d.run(function() {
        var tester = new BlockTester();
        var result = tester.yield_one(function(arg) {
          throw new Error();
        });
        expect(result).to.be.undefined;
        done();
      });
      d.dispose();
    });
  });
});
