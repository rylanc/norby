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
  
  it.skip('should expose module classes', function() {
    var MyMod = ruby.getConstant('MyMod');
    expect(MyMod.ModClass).to.be.a('function');
  });
});
