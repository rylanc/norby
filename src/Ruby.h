#pragma once

#include <node.h>
#include <nan.h>

class Ruby : public node::ObjectWrap
{
 public:
  static void Init(v8::Handle<v8::Object> module);
  static void Cleanup(void*);
  
  static v8::Local<v8::Function> GetCtor(v8::Local<v8::Function> rubyClass);
 
 private:
  
  static NAN_METHOD(New);
  static NAN_METHOD(GetClass);
  static NAN_METHOD(GCStart);
  static NAN_METHOD(DefineClass);
  static NAN_METHOD(Require);
  static NAN_METHOD(Eval);
  static NAN_METHOD(GetFunction);
  static NAN_METHOD(GetConstant);

  static v8::Persistent<v8::Function> s_getCtor;
  
  // Don't instantiate
  Ruby();
  ~Ruby();
};
