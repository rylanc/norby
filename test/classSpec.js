var expect = require('chai').expect,
    ruby = require('../index');
    
describe('Ruby classes', function() {
  ruby.require('./test/helpers');
  
  it('should expose class methods', function() {
    var Time = ruby.getClass('Time');
    
    expect(Time).itself.to.respondTo('utc');
    var t = Time.utc(2001, 2, 3);
    expect(t.year()).to.equal(2001);
    expect(t.month()).to.equal(2);
    expect(t.mday()).to.equal(3);
    
    expect(Time).itself.to.respondTo('at');
    var t = Time.at(946702800);
    t.utc();
    expect(t.year()).to.equal(2000);
    expect(t.month()).to.equal(1);
    expect(t.mday()).to.equal(1);
  });
  
  it('should expose inherited class methods', function() {
    var ClassTester = ruby.getClass('ClassTester');
    var result = ClassTester.const_get('MY_CONST');
    expect(result).to.deep.equal([1, 2]);
  });
  
  it('should rename the Ruby \'name\' method to \'rubyName\'', function() {
    var Time = ruby.getClass('Time');
    expect(Time).itself.to.respondTo('rubyName');
    var result = Time.rubyName();
    expect(result).to.equal('Time');
  });
  
  it('should expose class constants', function() {
    var ClassTester = ruby.getClass('ClassTester');
    var result = ClassTester.MY_CONST;
    expect(result).to.deep.equal([1, 2]);
  });
  
  it('should call overridden \'new\'', function() {
    var Proc = ruby.getClass('Proc');
    var p = new Proc(function(name) {
      return 'hello, ' + name;
    });
    
    var result = p.call('stan');
    expect(result).to.equal('hello, stan');
  });
  
  it.skip('should properly unwrap Ruby classes when passed as args', function() {
    var Time = ruby.getClass('Time');
    var t = new Time();
    var result = t['is_a?'](Time);
    expect(result).to.be.true;
  });
  
  it('should alias inspect() to inspect(depth)', function() {
    var Time = ruby.getClass('Time');
    expect(Time).itself.to.respondTo('inspect');
    
    var result = Time.inspect(2);
    expect(result).to.equal('Time');
  });
});
