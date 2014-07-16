#include "RubyModule.h"
#include "common.h"
#include "RubyObject.h"
#include <string>
#include <cstring>

using namespace v8;

RubyModule::ObjMap RubyModule::s_objCache;
Persistent<Function> RubyModule::s_createCtor;

Local<Object> RubyModule::ToV8(VALUE mod)
{
  NanEscapableScope();
  assert(TYPE(mod) == T_MODULE || TYPE(mod) == T_CLASS);
  
  Local<Object> v8Mod;
  ObjMap::iterator it = s_objCache.find(mod);
  if (it == s_objCache.end()) {
    log("Creating new module/class: " << rb_class2name(mod));
    
    Local<ObjectTemplate> tpl = NanNew<ObjectTemplate>();
    tpl->SetInternalFieldCount(1);
    v8Mod = tpl->NewInstance();
    NanSetInternalFieldPointer(v8Mod, 0, (void*)mod);
  
    // Class methods
    AddMethods(v8Mod, rb_class_public_instance_methods(0, NULL, CLASS_OF(mod)));
    AddMethods(v8Mod, rb_obj_singleton_methods(0, NULL, mod));
  
    if (TYPE(mod) == T_CLASS) {
      // TODO: new or newInstance?
      ID newID = rb_intern("new");
      Local<FunctionTemplate> newTemplate =
        NanNew<FunctionTemplate>(CallNew, EXTERNAL_WRAP((void*)newID));
      v8Mod->Set(NanNew<String>("new"), newTemplate->GetFunction());
    
      Local<FunctionTemplate> defMethTpl =
        NanNew<FunctionTemplate>(DefineMethod);
      v8Mod->Set(NanNew<String>("_defineMethod"), defMethTpl->GetFunction());
      
      Handle<Value> argv[] = { v8Mod };
      Local<Function> createCtor = NanNew<Function>(s_createCtor);
      v8Mod = NanMakeCallback(NanGetCurrentContext()->Global(),
                              createCtor, 1, argv).As<Object>();
    }
                           
#if (NODE_MODULE_VERSION > 0x000B)
    s_objCache[mod].Reset(v8::Isolate::GetCurrent(), v8Mod);
#else
    NanAssignPersistent(s_objCache[mod], v8Mod);
#endif

    // Constants
    VALUE constants = rb_mod_constants(0, NULL, mod);
    for (int i = 0; i < RARRAY_LEN(constants); i++) {
      ID constantID = SYM2ID(rb_ary_entry(constants, i));
      
      VALUE val = rb_const_get(mod, constantID);
      v8Mod->Set(NanNew<String>(rb_id2name(constantID)), rubyToV8(val));
    }
  }
  else {
    log("Getting existing module/class: " << rb_class2name(mod));
    
#if (NODE_MODULE_VERSION > 0x000B)    
    v8Mod = Local<Object>::New(v8::Isolate::GetCurrent(), it->second);
#else
    v8Mod = NanNew<Object>(it->second);
#endif
  }
  
  return NanEscapeScope(v8Mod);
}

inline void RubyModule::AddMethods(Handle<Object> tpl, VALUE methods)
{
  for (int i = 0; i < RARRAY_LEN(methods); i++) {
    ID methodID = SYM2ID(rb_ary_entry(methods, i));
    VALUE methodName = rb_id2str(methodID);
    
    if (std::strncmp(RSTRING_PTR(methodName), "new",
                     RSTRING_LEN(methodName)) != 0) {
      Local<FunctionTemplate> methodTemplate =
        NanNew<FunctionTemplate>(CallMethod, EXTERNAL_WRAP((void*)methodID));
      tpl->Set(NanNew<String>(RSTRING_PTR(methodName), RSTRING_LEN(methodName)),
               methodTemplate->GetFunction());
    }
  }
}

NAN_METHOD(RubyModule::CallMethod)
{
  NanScope();
  VALUE klass = VALUE(NanGetInternalFieldPointer(args.Holder(), 0));
  NanReturnValue(CallRubyFromV8(klass, args));
}

NAN_METHOD(RubyModule::CallNew)
{
  NanScope();
  VALUE klass = VALUE(NanGetInternalFieldPointer(args.Holder(), 0));
  assert(args[0]->IsObject());
  Local<Object> owner = args[0].As<Object>();

  log("Creating new " << rb_class2name(klass) << " with " <<
      args.Length() << " args");
  
  VALUE obj;
  SAFE_RUBY_CALL(obj, MethodCaller(klass, args, 1));

  NanReturnValue(RubyObject::ToV8(obj, owner));
}

// TODO: Should we hide this from non-created classes?
NAN_METHOD(RubyModule::DefineMethod)
{
  NanScope();
  
  VALUE klass = VALUE(NanGetInternalFieldPointer(args.Holder(), 0));
  
  log("Defining method " << rb_class2name(klass) << "." <<
      *String::Utf8Value(args[0]));

  rb_define_method(klass, *String::Utf8Value(args[0]),
                   RUBY_METHOD_FUNC(RubyObject::CallV8Method), -1);
  
  NanReturnUndefined();
}
