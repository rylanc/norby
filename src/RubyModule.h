#ifndef NORBY_RUBY_MODULE_H_
#define NORBY_RUBY_MODULE_H_

#include <nan.h>
#include <ruby.h>
#include <map>

class RubyModule
{
 public:
  static v8::Local<v8::Object> ToV8(VALUE mod);
  
  static inline void SetCreateCtor(v8::Local<v8::Function> createCtor)
  {
    NanAssignPersistent(s_createCtor, createCtor); 
  }

 private:
 
  static void AddMethods(v8::Handle<v8::ObjectTemplate> tpl, VALUE methods);
  
  static NAN_METHOD(CallMethod);
  static NAN_METHOD(CallNew);
  static NAN_METHOD(DefineMethod);
  
#if (NODE_MODULE_VERSION > 0x000B)
  typedef std::map<VALUE, v8::CopyablePersistentTraits<v8::Object>::CopyablePersistent> ObjMap;
#else
  typedef std::map<VALUE, v8::Persistent<v8::Object> > ObjMap;
#endif

  static ObjMap s_objCache;
  static v8::Persistent<v8::Function> s_createCtor;
 
  // Don't instantiate
  RubyModule();
  ~RubyModule();
};

#endif // NORBY_RUBY_MODULE_H_
