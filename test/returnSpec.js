var expect = require('chai').expect,
    ruby = require('../index');

describe('return', function() {
  ruby.require('./test/helpers');
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
    
    it('should handle positive values up to 2^53', function() {
      var t = new Returner();
      
      var result = t.ret_max_int32()
      expect(result).to.be.a('number');
      expect(result).to.equal(2147483647);
      
      result = t.ret_max_precise_double();
      expect(result).to.be.a('number');
      expect(result).to.equal(9007199254740992);
    });
    
    it('should handle negative values down to -2^53', function() {
      var t = new Returner();
      
      var result = t.ret_min_int32()
      expect(result).to.be.a('number');
      expect(result).to.equal(-2147483648);
      
      result = t.ret_min_precise_double();
      expect(result).to.be.a('number');
      expect(result).to.equal(-9007199254740992);
    });
  });
  
  // TODO: Should we test negative numbers too?
  describe('bignum', function() {
    // Even though we lose precision due to conversion to floating point,
    // Bignums should still convert
    it('should return a number when it fits', function() {
      var t = new Returner();
      var result = t.ret_bignum();
      expect(result).to.be.a('number');
      expect(result).to.equal(5234567890987654321);
    });
    
    it('should return a string when it can\'t fit', function() {
      var t = new Returner();
      var result = t.ret_hugenum();
      expect(result).to.be.a('string');
      expect(result).to.equal('359538626972463141629054847463408713596141135051689993197834953606314521560057077521179117265533756343080917907028764928468642653778928365536935093407075033972099821153102564152490980180778657888151737016910267884609166473806445896331617118664246696549595652408289446337476354361838599762500808052368249716736');
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
    
    it.skip('should properly convert Unicode strings', function() {
      var t = new Returner();
      var result = t.ret_unicode();
      console.log(result.length);
      expect(result).to.be.a('string');
      expect(result).to.equal('ҊҊҊҊҊҊ');
    });
    
    it('should properly convert ISO-8859-1 strings', function() {
      var t = new Returner();
      var result = t.ret_iso_8859_1();
      expect(result).to.be.a('string');
      expect(result).to.equal('÷÷÷÷÷÷÷÷');
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
  
  describe('ruby object', function() {
    it('should return a valid Time object', function() {
      var r = new Returner();
      var t = r.ret_time();
      
      expect(t.year()).to.equal(2001);
      expect(t.month()).to.equal(2);
      expect(t.mday()).to.equal(3);
    });
    
    it('should return the same object for subsequent calls', function() {
      var r = new Returner();
      var t1 = ruby.newInstance('Time', 2005, 8, 7);
      r.set_time(t1);
      var t2 = r.get_time();
      
      expect(t2.year()).to.equal(2005);
      expect(t2.month()).to.equal(8);
      expect(t2.mday()).to.equal(7);
      expect(t2).to.equal(t1);
    });
  });
});
