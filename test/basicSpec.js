var expect = require('chai').expect,
    ruby = require('../index');
    
describe('.require', function() {
  
  it('shouldn\'t throw on valid file', function() {
    ruby.require('./test/test.rb');
  });
  
  it('should throw on invalid file', function() {
    var fn = function() { ruby.require('./dfgfgnfghjnjhm.rb'); };
    expect(fn).to.throw(Error);
  });
});

describe('.getClass', function() {
  
  it('should return a constructor given an existing class', function() {
    ruby.require('./test/test.rb');
    var Returner = ruby.getClass('Returner');
    expect(Returner).to.be.a('function');
  });
});
