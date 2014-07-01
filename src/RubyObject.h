#pragma once

#include <node.h>
#include <nan.h>
#include <v8.h>
#include <ruby.h>
#include <map>

#include "common.h"

class RubyObject : public node::ObjectWrap
{
 public:
  static const char* RUBY_OBJECT_TAG;
  static ID V8_WRAPPER_ID;
 
  static void Init();
  static void Cleanup();
  
  static v8::Local<v8::Function> GetClass(VALUE klass);
  VALUE GetObject() { return m_obj; }
  
  v8::Local<v8::Object> GetOwner()
  {
    return NanNew<v8::Object>(*m_owner);
  }

 private:
  RubyObject(VALUE obj, v8::Local<v8::Value> owner);
  ~RubyObject();

  static NAN_METHOD(New);
  static NAN_METHOD(CallMethod);

  VALUE m_obj;
  v8::Persistent<v8::Object>* m_owner;
#if (NODE_MODULE_VERSION > 0x000B)
  typedef std::map<ID, v8::CopyablePersistentTraits<v8::FunctionTemplate>::CopyablePersistent> TplMap;
#else
  typedef std::map<ID, v8::Persistent<v8::FunctionTemplate> > TplMap;
#endif
  static TplMap s_functionTemplates;
  static VALUE s_wrappedClass;
};
