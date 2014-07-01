var expect = require('chai').expect,
    ruby = require('../index');
    
describe('.inherits', function() {
  ruby.require('./test/test.rb');
  ruby.inherits(Derived, 'InheritTester');
  
  function Derived() {
    Derived.super_.call(this);
    this.val = 'hello';
  }

  Derived.prototype.call_derived = function() {
    return 49895;
  };
  
  Derived.prototype.call_derived_with_args = function(arg1, arg2) {
    return arg1 + arg2;
  };
  
  it('should contain base class methods', function() {
    expect(Derived).to.respondTo('make_call');
    expect(Derived).to.respondTo('clone');
  });
  
  it('should call derived methods', function() {
    var d = new Derived();
    var result = d.make_call();
    expect(result).to.equal(49895);
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
  
  it('should implement \'respond_to?\' that handles strings', function() {
    var d = new Derived();
    var result = d['respond_to?']('call_derived');
    expect(result).to.be.true;
    
    result = d['respond_to?']('doesnt_exist');
    expect(result).to.be.false;
    
    var fn = function() { return d['respond_to?'](); };
    expect(fn).to.throw(Error);
  });
  
  it('should implement \'respond_to?\' that handles symbols', function() {
    var d = new Derived();
    var result = d.valid_responds();
    expect(result).to.be.true;
    
    result = d.invalid_responds();
    expect(result).to.be.false;
  });
});
