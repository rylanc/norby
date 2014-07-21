norby
=====

[![NPM version](https://badge.fury.io/js/norby.svg)](http://badge.fury.io/js/norby)
[![Build Status](https://travis-ci.org/rylanc/norby.svg?branch=master)](https://travis-ci.org/rylanc/norby)

Call your Ruby libraries from node.js

```js
var ruby = require('norby');

var Time = ruby.getClass('Time');
var t = new Time(2014, 7, 2);
console.log('Year: ' + t.year()); // => 'Year: 2014'
console.log(t); // => '2014-07-02 00:00:00 -0400'
```

## To install

Prerequisites:

    * node.js >= 0.10
    * ruby >= 1.9

Install using npm:

```sh
npm install norby
```

Compile from repository:

```sh
git clone https://github.com/rylanc/norby.git
cd norby
npm install
```

Run unit tests:
```sh
npm test
```

## What's missing

norby is currently in an early beta state. Check back for updates as features
are implemented. Feel free to add an issue for any bugs or missing features.

 - Windows support. node.js is built with Visual Studio while most Windows Ruby
   installers use [MinGW](http://www.mingw.org). It may work if you build Ruby
   with VS, but I haven't tried it yet.
 - Support for Ruby version 1.8.X
 - Support for Ruby including/extending
 - Support for Ruby hashes
 - Conversion of JS objects (that aren't wrapped Ruby objects)
 - Support for Ruby structs
 - Support for Ruby global variables

## API

### ruby

  Exposed by `require('norby')`.
  
### ruby#require(name:String)

  Calls Ruby's [require](http://www.ruby-doc.org/core/Kernel.html#method-i-require)
  method with the specified `name`.
  
### ruby#eval(code:String [, binding:Binding [, filename:String [, lineno:Number]]])
  Calls Ruby's [eval](http://www.ruby-doc.org/core/Kernel.html#method-i-eval) method
  with the specified parameters.

### ruby#getClass(name:String)

  Returns a wrapped Ruby class specified by `name`. The `new` class method will
  be called when the constructor is called.
  
```js
var Time = ruby.getClass('Time');
var t = new Time(2014, 7, 2);
```
  
  To get a class within a module, separate the module and class with `::`.
  
```js
var ZlibInflate = ruby.getClass('Zlib::Inflate');
```

  Class methods and constants are exposed as properties of the constructor.

```js
var Time = ruby.getClass('Time');
var t = Time.utc(2014, 7, 2);

var File = ruby.getClass('File');
console.log(File.SEPARATOR); // => '/'
```

### ruby#newInstance(className:String [, …])

  Returns a new instance of a Ruby object specified by `className`. Any
  additional arguments will be passed on to the class's   `new` method.

```js
var t = ruby.newInstance('Time', 2014, 7, 2);
```

### ruby#inherits(derived:Constructor, superName:String)
  
  Creates a new Ruby class (named `derived.name`) who's superclass is specified
  by `superName`. All public instance methods of `superName` will be added to
  the derived class's prototype. To add methods to the derived class, call its
  `defineMethod` function. This will add the method to the class's prototype and
  override the Ruby superclass's method. Adding the method to the prototype only
  will fail to override the superclass's method.

```ruby
# base.rb
class Base
  def call_me
  end
  def make_call
    call_me
  end
end
```
```js
ruby.require('./base');

function Derived() {
  Derived.super_.apply(this, arguments);
  this.val = 'Hello';
}
ruby.inherits(Derived, 'Base');

Derived.defineMethod('call_me', function() {
  console.log('In JS: ' + this.val);
});

var d = new Derived();
d.make_call(); // => 'In JS: Hello'
```

### ruby#getMethod(name:String)

  Returns a JS function that wraps the Ruby method specified by `name`. This currently only works with [Kernel](http://www.ruby-doc.org/core/Kernel.html) methods.

```ruby
# hello.rb
def my_func (name)
  puts "Hello, #{name}!"
end
```

```js
ruby.require('./hello');
var my_func = ruby.getMethod('my_func');
my_func('Stan');
```

### ruby#getConstant(name:String)
  
  Returns the Ruby constant specified by `name`. To get a constant within a module or class, separate the module and constant with ::. Without the separator, it returns an [Object](http://www.ruby-doc.org/core/Object.html) constant. `getConstant` can also be used to return modules.

```ruby
# const.rb
module MyMod
  MY_CONST = "abcde"
end

class MyClass
  OTHER_CONST = "fghi"
end
```
  
```js
ruby.require('./const');
var RUBY_VERSION = ruby.getConstant('RUBY_VERSION');
console.log(ruby.getConstant('MyMod::MY_CONST')); // => 'abcde'
console.log(ruby.getConstant('MyClass::OTHER_CONST')); // => 'fghi'
```

## Ruby objects

### Methods

Wrapped Ruby objects expose their public instance methods through function
properties.

```js
var Time = ruby.getClass('Time');
var t = new Time(2014, 7, 2);
console.log(t.year()); // => '2014'
```

The Ruby `to_s` method is mapped to the JS `toString()` function.

```js
console.log(t.toString()); // => '2014-07-02 00:00:00 -0400'
```

Since node's `console.log()` function calls `inspect` with  a `depth` argument,
it is ignored when passed to the Ruby `inspect` method.

```js
console.log(t); // => '2014-07-02 00:00:00 -0400'
```

### Blocks

If the last argument in a method call is a function, it is passed to the method
as a Ruby block.

```js
var Regexp = ruby.getClass('Regexp');
var pat = new Regexp('.at');
pat.match('cat', function() {
  console.log('match!');
}); // => 'match!'
```

## Ruby Modules

To retrieve Ruby modules, call `ruby#getConstant`. Modules are returned as JS objects
with their class methods and constants as properties.

```ruby
# mod.rb

module MyMod
  MY_CONST = "abcde"
  
  def MyMod.say_hi
    puts "Hello"
  end
end
```

```js
ruby.require('./mod');
var MyMod = ruby.getConstant('MyMod');

console.log(MyMod.MY_CONST); // => 'abcde'
MyMod.say_hi(); // => 'Hello'
```

## Type conversion

### From node

#### `null` and `undefined`

Are converted to `nil`.

#### booleans

Are converted to Ruby booleans.

#### numbers

If the number can be determined to be an integer (`v8::Value::IsInt32()`), it's
converted to a Ruby `Fixnum`. Otherwise, it's converted to a `Float`.

#### arrays

Are converted to Ruby arrays. Their contents are recursively converted.

#### strings

Are converted to Ruby strings.

#### Ruby objects

Wrapped Ruby objects are unwrapped.

### From Ruby

#### `nil`

Is converted to `undefined`.

#### booleans

Are converted to JS booleans.

#### `Float`s

Are converted to JS numbers.

#### `Fixnum`s

Are converted to JS numbers. Keep in mind that JS stores numbers as double
precision floating point numbers, meaning that `Fixnum`s (and `Bignum`s) will
lose precision above 2<sup>53</sup>.

#### `Bignum`s

Are converted to JS numbers if they are less than or equal to `Number.MAX_VALUE`. Otherwise, they are converted into JS strings. See the above note about precision.

#### arrays

Are converted to JS arrays. Their contents are recursively converted.

#### strings

Are converted to JS strings.

#### `Symbol`s

Are converted to JS strings.

#### Ruby objects

Are wrapped as JS objects.

#### Ruby exceptions

Are caught within norby native code, converted to v8 exceptions and rethrown.
The thrown v8 exceptions have a `rubyStack` property that holds the result of
calling `Exception#backtrace` on the original Ruby exceptions.

## License

The MIT License (MIT)

Copyright (c) 2014 Rylan Collins

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
