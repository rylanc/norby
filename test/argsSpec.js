var expect = require('chai').expect,
    ruby = require('../index');
    
describe('passing arguments', function() {
  ruby.require('./test/test.rb');
  var ArgsInspector = ruby.getClass('ArgsInspector');
  
  it('should pass the correct number of arguments', function() {
    var a = new ArgsInspector();
    
    expect(a.arg_count()).to.equal(0);
    expect(a.arg_count('a')).to.equal(1);
    expect(a.arg_count('a', 2)).to.equal(2);
    expect(a.arg_count('a', 2, 'c')).to.equal(3);
    expect(a.arg_count('a', 2, 'c', 4)).to.equal(4);
  });
  
  it('should handle wrong number of arguments', function() {
    var a = new ArgsInspector();
    
    var fn = function() { return a.get_type(); };
    expect(fn).to.throw(Error);
    
    fn = function() { return a.get_type(1, 2); };
    expect(fn).to.throw(Error);
  });
  
  describe('undefined', function() {
    it('should convert to nil', function() {
      var a = new ArgsInspector();
      var val = undefined;
      
      expect(a.get_type(val)).to.equal('NilClass');
      expect(a.check_nil(val)).to.be.true;
    });
  });
  
  describe('null', function() {
    it('should convert to nil', function() {
      var a = new ArgsInspector();
      var val = null;
      
      expect(a.get_type(val)).to.equal('NilClass');
      expect(a.check_nil(val)).to.be.true;
    });
  });
  
  describe('true', function() {
    it('should convert to true', function() {
      var a = new ArgsInspector();
      var val = true;
      expect(a.get_type(val)).to.equal('TrueClass');
      expect(a.check_true(val)).to.be.true;
    });
  });
  
  describe('false', function() {
    it('should convert to false', function() {
      var a = new ArgsInspector();
      var val = false;
      
      expect(a.get_type(val)).to.equal('FalseClass');
      expect(a.check_false(val)).to.be.true;
    });
  });
  
  describe('string', function() {
    it('should convert to a ruby string', function() {
      var a = new ArgsInspector();
      var val = 'hello';
      
      expect(a.get_type(val)).to.equal('String');
      expect(a.check_string(val)).to.be.true;
    });
  });
  
  describe('array', function() {
    it('should convert to ruby array', function() {
      var a = new ArgsInspector();
      var val = [4, 3, 9];
      
      expect(a.get_type(val)).to.equal('Array');
      expect(a.check_array(val)).to.be.true;
    });
  });
  
  describe('int32', function() {
    it('should convert to fixnum', function() {
      var a = new ArgsInspector();
      var val = -49895;
      
      expect(a.get_type(val)).to.equal('Fixnum');
      expect(a.check_int32(val)).to.be.true;
    });
  });
  
  describe('uint32', function() {
    it('should convert to fixnum', function() {
      var a = new ArgsInspector();
      var val = 49895;
      
      expect(a.get_type(val)).to.equal('Fixnum');
      expect(a.check_uint32(val)).to.be.true;
    });
  });
  
  describe('number', function() {
    it('should convert to float', function() {
      var a = new ArgsInspector();
      var val = 4.359;
      
      expect(a.get_type(val)).to.equal('Float');
      expect(a.check_number(val)).to.be.true;
    });
  });
});

describe('ruby constructor', function() {
  ruby.require('./test/test.rb');
  var ClassTester = ruby.getClass('ClassTester');
    
  it('should pass v8 arguments', function() {
    var tester = new ClassTester('hello');
    expect(tester.get_val()).to.equal('hello');
  });
  
  it('should handle wrong number of arguments', function() {
    var fn = function() { return new ClassTester(); };
    expect(fn).to.throw(Error);
    
    fn = function() { return new ClassTester(1, 2); };
    expect(fn).to.throw(Error);
  });
});
