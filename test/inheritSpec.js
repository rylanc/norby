var expect = require('chai').expect,
    ruby = require('../index');
    
describe('.inherits', function() {
  ruby.require('./test/helpers');
  ruby.inherits(Derived, 'InheritTester');
  
  function Derived() {
    Derived.super_.call(this);
    this.val = 'hello';
  }

  Derived.defineMethod('call_derived', function() {
    return 49895;
  });
  
  Derived.defineMethod('call_derived_with_this', function(arg1, arg2) {
    return this.val;
  });
  
  Derived.defineMethod('call_derived_with_args', function(arg1, arg2) {
    return arg1 + arg2;
  });
  
  Derived.defineMethod('call_missing', function() {
    return 1234;
  });
  
  it('should throw when the subclass name matches an existing ruby class',
  function() {
    function Time() {
      Time.super_.call(this);
    }
    
    var fn = function() { ruby.inherits(Time, 'InheritTester'); };
    expect(fn).to.throw(TypeError);
  });
  
  describe('the derived class', function() {

    it('should contain base class methods', function() {
      expect(Derived).to.respondTo('make_call');
      expect(Derived).to.respondTo('clone');
    });

    it('should call overridden methods', function() {
      var d = new Derived();
      var result = d.make_call();
      expect(result).to.equal(49895);
    });
    
    it('should call newly defined methods', function() {
      var d = new Derived();
      var result = d.make_missing_call();
      expect(result).to.equal(1234);
    });
    
    it('should call methods with correct \'this\'', function() {
      var d = new Derived();
      var result = d.make_call_with_this();
      expect(result).to.equal('hello');
    });

    it('should pass arguments to derived methods', function() {
      var d = new Derived();
      var result = d.make_call_with_args(8, 5);
      expect(result).to.equal(13);
    });

    it('should pass constructor arguments to super', function() {
      function DerivedArgs() {
        DerivedArgs.super_.apply(this, arguments);
      }
      ruby.inherits(DerivedArgs, 'ArgsInheritTester');

      var d = new DerivedArgs(2001, 2, 3);
      expect(d.get_arg1()).to.equal(2001);
      expect(d.get_arg2()).to.equal(2);
      expect(d.get_arg3()).to.equal(3);
    });

    it('should throw when an undefined function is called', function() {
      var d = new Derived();
      var fn = function() { return d.make_invalid_call(); };

      expect(fn).to.throw(ReferenceError);
    });
    
    it('should work with instanceof', function() {
      var InheritTester = ruby.getClass('InheritTester');
      var d = new Derived();
      expect(d).to.be.an.instanceof(InheritTester);
    });
    
    describe('.defineMethod', function() {
      it('should throw when a non-function is passed in', function() {
        var fn = function() { Derived.defineMethod('invalid', 5); };
        expect(fn).to.throw(TypeError);
      });
      
      it('should throw when a property is later changed to a non-function', function() {
        var d = new Derived();
        d.call_derived = 5;
        var fn = function() { return d.make_call(); };
        expect(fn).to.throw(TypeError);
      });
    });
  });
});
