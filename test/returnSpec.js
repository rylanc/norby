var expect = require('chai').expect,
    ruby = require('../index');

describe('return', function() {
  ruby.require('./test/test.rb');
  var Returner = ruby.getClass('Returner');
  
  describe('nothing', function() {
    it('should return undefined', function() {
      var t = new Returner();
      var result = t.ret_nothing();
      expect(result).to.be.undefined;
    });
  });
  
  describe('nil', function() {
    it('should return undefined', function() {
      var t = new Returner();
      var result = t.ret_nil();
      expect(result).to.be.undefined;
    });
  });
  
  describe('float', function() {
    it('should return 4.359', function() {
      var t = new Returner();
      var result = t.ret_float();
      expect(result).to.be.a('number');
      expect(result).to.equal(4.359);
    });
  });
  
  describe('fixnum', function() {
    it('should return 49895', function() {
      var t = new Returner();
      var result = t.ret_fixnum();
      expect(result).to.be.a('number');
      expect(result).to.equal(49895);
    });
  });
  
  describe('true', function() {
    it('should return true', function() {
      var t = new Returner();
      var result = t.ret_true();
      expect(result).to.be.a('boolean');
      expect(result).to.be.true;
    });
  });
  
  describe('false', function() {
    it('should return true', function() {
      var t = new Returner();
      var result = t.ret_false();
      expect(result).to.be.a('boolean');
      expect(result).to.be.false;
    });
  });
  
  describe('string', function() {
    it('should return \'hello\'', function() {
      var t = new Returner();
      var result = t.ret_string();
      expect(result).to.be.a('string');
      expect(result).to.equal('hello');
    });
  });
  
  describe('array', function() {
    it('should return [4, 3, 9]', function() {
      var t = new Returner();
      var result = t.ret_array();
      expect(result).to.be.an('array');
      expect(result).to.deep.equal([4, 3, 9]);
    });
  });
});
