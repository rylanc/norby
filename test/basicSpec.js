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
  
  it('should throw a SyntaxError given invalid syntax', function() {
    var fn = function() { ruby.require('./test/invalid.rb'); };
    expect(fn).to.throw(SyntaxError);
  });
});

describe('.getClass', function() {
  ruby.require('./test/test.rb');
  
  it('should return a constructor given an existing class name', function() {
    var Returner = ruby.getClass('Returner');
    expect(Returner).to.be.a('function');
  });
  
  it('should throw an exception given an invalid class name', function() {
    var fn = function() { return ruby.getClass('DoesntExist'); };
    expect(fn).to.throw(ReferenceError);
  });
  
  if('should alias to_s to toString', function() {
    var Time = ruby.getClass('Time');
    expect(Time).to.respondTo('toString');
    
    var t = new Time(2001, 2, 3);
    var result = t.toString();
    expect(result).to.equal('2001-02-03 00:00:00 -0500');
  });
  
  if('should alias inspect() to inspect(depth)', function() {
    var Time = ruby.getClass('Time');
    expect(Time).to.respondTo('inspect');
    
    var t = new Time(2001, 2, 3);
    var result = t.inspect(2);
    expect(result).to.equal('2001-02-03 00:00:00 -0500');
  });
});

describe('.newInstance', function() {
  ruby.require('./test/test.rb');
  
  it('should return an instance given an existing class name', function() {
    var r = ruby.newInstance('Returner');
    expect(r).to.be.a('object');
    expect(r).to.have.property('class');
  });
  
  it('should throw an exception given an invalid class name', function() {
    var fn = function() { return ruby.newInstance('DoesntExist'); };
    expect(fn).to.throw(ReferenceError);
  });
  
  it('should properly forward the given arguments', function() {
    var tester = ruby.newInstance('ClassTester', 'hello');
    expect(tester.get_val()).to.equal('hello');
  });
});

describe('.eval', function() {
  it('should properly evaluate given ruby strings', function() {
    var result = ruby.eval('b = "hello" \n b');
    expect(result).to.be.a('string');
    expect(result).to.equal('hello');
  });
  
  it('should throw a SyntaxError given invalid syntax', function() {
    var fn = function() { ruby.eval('end'); };
    expect(fn).to.throw(SyntaxError);
  });
});

describe('.getFunction', function() {
  it('should return a function given a valid name', function() {
    var non_class_function = ruby.getFunction('non_class_function');
    expect(non_class_function).to.be.a('function');
    
    var result = non_class_function(4, 8, 23);
    expect(result).to.be.a('number');
    expect(result).to.equal(35);
  });
  
  it('should throw an error given an invalid name', function() {
    var doesntExist = ruby.getFunction('doesntExist');
    var fn = function() { doesntExist(); };
    expect(fn).to.throw(ReferenceError);
  });
});
