#ifndef NORBY_RUBY_H_
#define NORBY_RUBY_H_

#include <node.h>
#include <nan.h>
#include <ruby.h>

class Ruby : public node::ObjectWrap
{
 public:
  static VALUE BLOCK_WRAPPER_CLASS;
 
  static void Init(v8::Handle<v8::Object> module);
  static void Cleanup(void*);
 
 private:
  
  static NAN_METHOD(New);
  static NAN_METHOD(GetClass);
  static NAN_METHOD(DefineClass);
  static NAN_METHOD(GetMethod);
  static NAN_METHOD(GetConstant);
  
  // Don't instantiate
  Ruby();
  ~Ruby();
};

#endif // NORBY_RUBY_H_
