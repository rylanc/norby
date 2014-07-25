var expect = require('chai').expect,
    ruby = require('../index');
    
before(function() {
  console.log('Ruby version: ' + ruby.getConstant('RUBY_VERSION'));
});
    
describe('.require', function() {
  
  it('shouldn\'t throw on valid file', function() {
    ruby.require('./test/helpers');
    var result = ruby.require('./test/helpers');
    expect(result).to.be.false;
  });
  
  it('should throw on invalid file', function() {
    var fn = function() { ruby.require('./dfgfgnfghjnjhm'); };
    expect(fn).to.throw(Error);
  });
  
  it('should throw a SyntaxError given invalid syntax', function() {
    var fn = function() { ruby.require('./test/invalid'); };
    expect(fn).to.throw(SyntaxError);
  });
  
  it('should throw given the wrong argument type', function() {
    var fn = function() { ruby.require(5); };
    expect(fn).to.throw(TypeError);
  });
});

describe('.getClass', function() {
  ruby.require('./test/helpers');
  
  it('should return a constructor given an existing class name', function() {
    var Returner = ruby.getClass('Returner');
    expect(Returner).to.be.a('function');
  });
  
  it('should throw a ReferenceError given an invalid class name', function() {
    var fn = function() { return ruby.getClass('DoesntExist'); };
    expect(fn).to.throw(ReferenceError);
  });
  
  it('should throw an TypeError given a non-class constant name', function() {
    var fn = function() { return ruby.getClass('RUBY_DESCRIPTION'); };
    expect(fn).to.throw(TypeError);
  });
  
  it('should work with module classes', function() {
    var ModClass = ruby.getClass('MyMod::ModClass');
    var m = new ModClass;
    var result = m.call_me();
    expect(result).to.equal(3.14159);
  });
  
  it('should return the same ctor for subsequent calls', function() {
    var T1 = ruby.getClass('Time');
    var T2 = ruby.getClass('Time');
    
    expect(T1).to.equal(T2);
  });
  
  it('should\'t enter an endless loop when it has a constant of itself',
  function(done) {
    ruby.getClass('SelfRefClass');
    done();
  });
});

describe('.newInstance', function() {
  ruby.require('./test/helpers');
  
  it('should return an instance given an existing class name', function() {
    var r = ruby.newInstance('Returner');
    expect(r).to.be.a('object');
    expect(r).to.have.property('class');
  });
  
  it('should throw a ReferenceError given an invalid class name', function() {
    var fn = function() { return ruby.newInstance('DoesntExist'); };
    expect(fn).to.throw(ReferenceError);
  });
  
  it('should properly forward the given arguments', function() {
    var tester = ruby.newInstance('ClassTester', 'hello');
    expect(tester.get_val()).to.equal('hello');
  });
  
  it('should work with instanceof', function() {
    var ClassTester = ruby.getClass('ClassTester');
    var tester = ruby.newInstance('ClassTester', 'hello');
    expect(tester).to.be.an.instanceof(ClassTester);
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
  
  it('should properly forward all arguments', function() {
    var get_binding = ruby.getMethod('get_binding');
    var result = ruby.eval('str + " Stan!"', get_binding("Hello"));
    expect(result).to.equal('Hello Stan!');
    
    var fn = function() { ruby.eval('end', get_binding('bla'), 'basicSpec.js', 112); };
    expect(fn).to.throw(SyntaxError, /basicSpec.js:112/);
  });
  
  it('should throw given the wrong argument type', function() {
    var fn = function() { ruby.eval(5); };
    expect(fn).to.throw(TypeError);
  });
});

describe('.getMethod', function() {
  it('should return a function given a valid name', function() {
    var non_class_function = ruby.getMethod('non_class_function');
    expect(non_class_function).to.be.a('function');
    
    var result = non_class_function(4, 8, 23);
    expect(result).to.be.a('number');
    expect(result).to.equal(35);
  });
  
  it('should throw a ReferenceError given an invalid name', function() {
    var doesntExist = ruby.getMethod('doesntExist');
    var fn = function() { doesntExist(); };
    expect(fn).to.throw(ReferenceError);
  });
  
  it('should properly forward blocks', function(done) {
    var non_class_yielder = ruby.getMethod('non_class_yielder');
    expect(non_class_yielder).to.be.a('function');
    
    non_class_yielder('Stan', function(str) {
      expect(str).to.equal('Hello, Stan');
      done();
    });
  });
});

describe('.getConstant', function() {
  ruby.require('./test/helpers');
  
  it('should return Object constants', function() {
    var TRUE = ruby.getConstant('TRUE');
    expect(TRUE).to.be.true;
    
    var FALSE = ruby.getConstant('FALSE');
    expect(FALSE).to.be.false;
  });
  
  it('should return module constants', function() {
    var MY_CONST = ruby.getConstant('MyMod::MY_CONST');
    expect(MY_CONST).to.equal('abcde');
  });

  it('should return class constants', function() {
    var MY_CONST = ruby.getConstant('ClassTester::MY_CONST');
    expect(MY_CONST).to.deep.equal([1, 2]);
  });
  
  it('should work recursively', function() {
    var NESTED_CONST = ruby.getConstant('MyMod::ModClass::NESTED_CONST');
    expect(NESTED_CONST).to.equal('12345');
  });
  
  it('should throw a ReferenceError given an invalid name', function() {
    var fn = function() { ruby.getConstant('DOESNT_EXIST'); };
    expect(fn).to.throw(ReferenceError);
  });
});
