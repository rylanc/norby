# norby

Call your ruby classes from node.js

## To install

Prerequisites:

    * node.js >= 0.10
    * ruby > = 1.X.X

Install using npm:

```sh
npm install norby
```

Compile from repository:

```sh
git clone REPO
npm install
```

Run unit tests:
```sh
npm test
```

## How to use

```js
var ruby = require('norby');

var Time = ruby.getClass('Time');
var t = new Time(2014, 7, 2);
console.log('Year: ' + t.year());
console.log(t);
```

## API

### ruby

  Exposed by `require('norby')`.
  
### ruby#require(name:String)

  Calls ruby's [require](http://www.ruby-doc.org/core/Kernel.html#method-i-require) function with the specified `name`.
  
### ruby#eval(code:String)
  
  Evaluates the Ruby expression(s) in `code`.

### ruby#getClass(name:String)

  Returns a wrapped ruby class specified by `name`. The `new` class method will be called when the constructor is called.
  
```js
var Time = ruby.getClass('Time');
var t = new Time(2014, 7, 2);
```

### ruby#newInstance(className:String[, …])

  Returns a new instance of a ruby object specified by `className`. Any additional arguments will be passed on to the class's   `new` method.

```js
var t = ruby.newInstance('Time', 2014, 7, 2);
```

**TODO:** Should we call them methods? Should they be global?
### ruby#getFunction(name:String)
  
  Returns a JS function that wraps the ruby function specified by `name`.

```ruby
# hello.rb
def my_func (name)
  puts "Hello, #{name}!"
end
```
```js
ruby.require('./hello');
var my_func = ruby.getFunction('my_func');
my_func('Stan');
```

### ruby#getConstant(name:String)
  
  Returns the ruby constant specified by `name` (i.e. an [Object](http://www.ruby-doc.org/core/Object.html) constant).
  
```js
var RUBY_VERSION = ruby.getConstant('RUBY_VERSION');
```
