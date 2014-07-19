var expect = require('chai').expect,
    ruby = require('../index');
    
describe('Ruby modules', function() {
  ruby.require('./test/helpers');
  
  it('should convert Ruby modules to objects', function() {
    var MyMod = ruby.getConstant('MyMod');
    expect(MyMod).to.be.an('object');
  });
  
  it('should expose class methods', function() {
    var MyMod = ruby.getConstant('MyMod');
    expect(MyMod.mod_func).to.be.a('function');
    var result = MyMod.mod_func();
    expect(result).to.deep.equal([1, 2, 3, 4]);
  });
  
  it('should expose inherited class methods', function() {
    var MyMod = ruby.getConstant('MyMod');
    expect(MyMod.name).to.be.a('function');
    var result = MyMod.name();
    expect(result).to.equal('MyMod');
  });
  
  it('should expose constants', function() {
    var MyMod = ruby.getConstant('MyMod');
    expect(MyMod.MY_CONST).to.be.a('string');
    expect(MyMod.MY_CONST).to.equal('abcde');
  });
  
  it('should expose module classes', function() {
    var MyMod = ruby.getConstant('MyMod');
    expect(MyMod.ModClass).to.be.a('function');
    
    var mc = new MyMod.ModClass();
    var result = mc.call_me();
    expect(result).to.equal(3.14159);
  });
  
  it('should alias inspect() to inspect(depth)', function() {
    var MyMod = ruby.getConstant('MyMod');
    expect(MyMod).itself.to.respondTo('inspect');
    
    var result = MyMod.inspect(2);
    expect(result).to.equal('MyMod');
  });
  
  it('should return the same object for subsequent calls', function() {
    var M1 = ruby.getConstant('MyMod');
    var M2 = ruby.getConstant('MyMod');
    
    expect(M1).to.equal(M2);
  });
});
