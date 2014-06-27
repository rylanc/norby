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
});
