var expect = require('chai').expect,
    ruby = require('../index');
    
describe('.require', function() {
  
  it('shouldn\'t throw on valid file', function() {
    ruby.require('./test/test.rb');
  });
  
  it('should throw on invalid file', function() {
    var fn = function() { ruby.require('./dfgfgnfghjnjhm.rb'); };
    expect(fn).to.throw(Error);
  });
  
  it('should throw a SyntaxError given invalid syntax', function() {
    var fn = function() { ruby.require('./test/invalid.rb'); };
    expect(fn).to.throw(SyntaxError);
  });
});

describe('.getClass', function() {
  ruby.require('./test/test.rb');
  
  it('should return a constructor given an existing class name', function() {
    var Returner = ruby.getClass('Returner');
    expect(Returner).to.be.a('function');
  });
  
  it('should throw an exception given an invalid class name', function() {
    var fn = function() { return ruby.getClass('DoesntExist'); };
    expect(fn).to.throw(ReferenceError);
  });
});

describe('.newInstance', function() {
  ruby.require('./test/test.rb');
  
  it('should return an instance given an existing class name', function() {
    var r = ruby.newInstance('Returner');
    expect(r).to.be.a('object');
    expect(r).to.have.property('class');
  });
  
  it('should throw an exception given an invalid class name', function() {
    var fn = function() { return ruby.newInstance('DoesntExist'); };
    expect(fn).to.throw(ReferenceError);
  });
  
  it('should properly forward the given arguments', function() {
    var tester = ruby.newInstance('ClassTester', 'hello');
    expect(tester.get_val()).to.equal('hello');
  });
});
