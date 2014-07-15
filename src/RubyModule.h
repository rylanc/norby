#ifndef NORBY_RUBY_MODULE_H_
#define NORBY_RUBY_MODULE_H_

#include <nan.h>
#include <ruby.h>

class RubyModule
{
 public:
  static v8::Local<v8::Object> Wrap(VALUE mod);

 private:
 
  static void AddMethods(v8::Handle<v8::ObjectTemplate> tpl, VALUE methods);
  
  static NAN_METHOD(CallMethod);
  static NAN_METHOD(CallNew);
  static NAN_METHOD(DefineMethod);
 
  // Don't instantiate
  RubyModule();
  ~RubyModule();
};

#endif // NORBY_RUBY_MODULE_H_
