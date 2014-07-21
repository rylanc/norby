#ifndef NORBY_RUBY_OBJECT_H_
#define NORBY_RUBY_OBJECT_H_

#include <node.h>
#include <nan.h>
#include <ruby.h>

#ifdef _DEBUG
#include <iostream>
#define LOG(x) std::cout << x << std::endl
#else
#define LOG(x)
#endif

class RubyValue : public node::ObjectWrap
{
 public:
  static ID V8_WRAPPER_ID;
  static VALUE BLOCK_WRAPPER_CLASS;
 
  static void Init();
  
  static v8::Local<v8::Object> New(VALUE obj);
  // TODO: Is this right?
  operator VALUE()
  {
    return m_obj;
  }
  
  static v8::Local<v8::Function> GetRubyValueCtor()
  {
    return NanNew<v8::Function>(s_constructor);
  }

 private:
  RubyValue(VALUE obj);
  ~RubyValue();
  
  static NAN_METHOD(CallMethod);
  static NAN_METHOD(CallMethodWithBlock);
  static NAN_METHOD(SetOwner);
  static NAN_METHOD(GetOwner);
  

  VALUE m_obj;
  // The pure JS object that holds the reference to this
  v8::Persistent<v8::Object>* m_owner;
  
  static VALUE s_wrappedClass;
  static v8::Persistent<v8::Function> s_constructor;
};

#endif // NORBY_RUBY_OBJECT_H_
