var expect = require('chai').expect,
    ruby = require('../index');
    
describe('Ruby exceptions', function() {
  ruby.require('./test/helpers');
  //var ErrorRaiser = ruby.getClass('ErrorRaiser');
  var raiser = ruby.newInstance('ErrorRaiser');
  
  it('should include the Ruby stack in the \'rubyStack\' property', function() {
    var err;
    try {
      raiser.wrong_args(5, 6);
    }
    catch (e) {
      err = e;
    }
    
    expect(err).to.be.an.instanceof(Error);
    expect(err).to.have.property('rubyStack')
      .that.is.an('array').with.length(1);
    expect(err.rubyStack[0]).to.be.a('string');
  });
  
  describe('ArgumentError', function() {
    it('should convert to JS Error', function() {
      var fn = function() { raiser.wrong_args(5, 6); };
      expect(fn).to.throw(Error, /wrong number/);
    });
  });
  
  describe('LoadError', function() {
    it('should convert to JS Error', function() {
      var fn = function() { require('./sdffggh') };
      expect(fn).to.throw(Error, /Cannot find/);
    });
  });
  
  describe('NameError', function() {
    it('should convert to JS ReferenceError', function() {
      var fn = function() { raiser.name_error() };
      expect(fn).to.throw(ReferenceError, 'hi');
    });
  });
  
  describe('NoMethodError', function() {
    it('should convert to JS ReferenceError', function() {
      var fn = function() { raiser.no_method_error() };
      expect(fn).to.throw(ReferenceError, 'undefined method');
    });
  });
  
  describe('TypeError', function() {
    it('should convert to JS TypeError', function() {
      var fn = function() { raiser.type_error() };
      expect(fn).to.throw(TypeError, /conversion/);
    });
  });
  
  describe('SyntaxError', function() {
    it('should convert to JS SyntaxError', function() {
      var fn = function() { ruby.eval('1+1=2'); };
      expect(fn).to.throw(SyntaxError, /unexpected/);
    });
  });
  
  describe('non-standard error', function() {
    it('should convert to JS Error', function() {
      var fn = function() { raiser.test_error(); };
      expect(fn).to.throw(Error, 'hello');
    });
  });
});
