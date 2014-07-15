var expect = require('chai').expect,
    ruby = require('../index');
    
describe('Ruby objects', function() {
  ruby.require('./test/helpers');
  
  it('should alias to_s to toString', function() {
    var Time = ruby.getClass('Time');
    expect(Time).to.respondTo('toString');
    
    var t = Time.utc(2001, 2, 3);
    var result = t.toString();
    expect(result).to.equal('2001-02-03 00:00:00 UTC');
  });
  
  it('should alias inspect() to inspect(depth)', function() {
    var Time = ruby.getClass('Time');
    expect(Time).to.respondTo('inspect');
    
    var t = Time.utc(2001, 2, 3);
    var result = t.inspect(2);
    expect(result).to.equal('2001-02-03 00:00:00 UTC');
  });
  
  it('should have a working .class method', function() {
    var T1 = ruby.getClass('Time');
    var t = new T1();
    var T2 = t.class();
    
    expect(T1).to.equal(T2);
    expect(t).to.be.an.instanceof(T2);
  });
});
