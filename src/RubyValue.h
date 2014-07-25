#ifndef NORBY_RUBY_VALUE_H_
#define NORBY_RUBY_VALUE_H_

#include <node.h>
#include <nan.h>
#include <ruby.h>

class RubyValue : public node::ObjectWrap
{
 public:
  static v8::Local<v8::Object> New(VALUE obj);
  static inline VALUE Unwrap(v8::Handle<v8::Object> handle)
  {
    return node::ObjectWrap::Unwrap<RubyValue>(handle)->m_obj;
  }
  
  static void Init();
  
  static v8::Local<v8::Function> GetCtor()
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
  static NAN_METHOD(GetType);
  
  NAN_WEAK_CALLBACK(OwnerWeakCB);

  VALUE m_obj;
  // The pure JS object that holds the reference to this
  v8::Persistent<v8::Object>* m_owner;
  
  static VALUE s_wrappedClass;
  static ID s_wrapperID;
  static v8::Persistent<v8::Function> s_constructor;
  static VALUE s_globalHash;
};

#endif // NORBY_RUBY_VALUE_H_
