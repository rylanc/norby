var expect = require('chai').expect,
    ruby = require('../index');
    
// describe.skip('.inherits', function() {
//   function Derived() {
//     Derived.super_.call(this);
//     this.val = 'hello';
//   }

//   Derived.prototype.call_derived = function() {
//     return 49895;
//   };
  
//   ruby.require('./test/test.rb');
//   ruby.inherits(Derived, 'InheritTester');
  
//   it('should contain base class methods', function() {
//     expect(Derived).to.respondTo('make_call');
//     expect(Derived).to.respondTo('clone');
//   });
  
//   it('should call derived methods', function() {
//     var d = new Derived();
//     var result = d.make_call();
//     expect(result).to.equal(49895);
//   });
  
//   it('should pass constructor arguments to super', function() {
//     function DerivedDate() {
//       DerivedDate.super_.call(this, 2001, 2, 3);
//     }
//     ruby.inherits(DerivedDate, 'Date');
    
//     var d = new DerivedDate();
//     expect(d.year()).to.equal(2001);
//     expect(d.month()).to.equal(2);
//     expect(d.mday()).to.equal(3);
//   });
// });
