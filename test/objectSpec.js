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
});
