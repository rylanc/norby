#ifndef NORBY_RUBY_OBJECT_H_
#define NORBY_RUBY_OBJECT_H_

#include <node.h>
#include <nan.h>
#include <ruby.h>
#include <map>

class RubyObject : public node::ObjectWrap
{
 public:
  static const char* RUBY_OBJECT_TAG;
  static ID V8_WRAPPER_ID;
 
  static void Init();
  static void Cleanup();
  
  static v8::Local<v8::Object> ToV8(VALUE rbObj, v8::Local<v8::Object> owner);
  static VALUE FromV8(v8::Handle<v8::Object> owner);
  
  static inline v8::Local<v8::Object> RubyUnwrap(VALUE self)
  {
    VALUE wrappedObj = rb_attr_get(self, V8_WRAPPER_ID);
    if (wrappedObj == Qnil) {
      return v8::Local<v8::Object>();
    }
    else {
      RubyObject* obj;
      Data_Get_Struct(wrappedObj, RubyObject, obj);
      return NanNew<v8::Object>(*obj->m_owner);
    }
  }
  
  static VALUE CallV8Method(int argc, VALUE* argv, VALUE self);

 private:
  RubyObject(VALUE obj, v8::Local<v8::Object> owner);
  ~RubyObject();
  
  static v8::Local<v8::Function> GetCtor(VALUE klass);

  static NAN_METHOD(CallInstanceMethod);

  VALUE m_obj;
  // The pure JS object that holds the reference to this
  v8::Persistent<v8::Object>* m_owner;
#if (NODE_MODULE_VERSION > 0x000B)
  typedef std::map<ID, v8::CopyablePersistentTraits<v8::FunctionTemplate>::CopyablePersistent> TplMap;
#else
  typedef std::map<ID, v8::Persistent<v8::FunctionTemplate> > TplMap;
#endif
  static TplMap s_functionTemplates;
  static VALUE s_wrappedClass;
};

#endif // NORBY_RUBY_OBJECT_H_
