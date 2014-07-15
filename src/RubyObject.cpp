#include "RubyObject.h"
#include "common.h"
#include <vector>
#include <string>

using namespace v8;

VALUE trueArg = Qtrue;

const char* RubyObject::RUBY_OBJECT_TAG = "_IsRubyObject";
ID RubyObject::V8_WRAPPER_ID;
RubyObject::TplMap RubyObject::s_functionTemplates;
VALUE RubyObject::s_wrappedClass;

void RubyObject::Init()
{
  s_wrappedClass = rb_define_class("WrappedRubyObject", rb_cObject);
  V8_WRAPPER_ID = rb_intern("@_wrappedObject");
}

void RubyObject::Cleanup()
{
  for (TplMap::iterator it = s_functionTemplates.begin();
       it != s_functionTemplates.end(); ++it) {
#if (NODE_MODULE_VERSION > 0x000B)
    it->second.Reset();
#else
    NanDisposePersistent(it->second);
#endif
  }
}

Local<Object> RubyObject::ToV8(VALUE rbObj, Local<Object> owner)
{
  NanEscapableScope();
  
  Local<Object> v8Obj = GetCtor(CLASS_OF(rbObj))->NewInstance();
  RubyObject* self = new RubyObject(rbObj, owner);
  self->Wrap(v8Obj);
  v8Obj->SetHiddenValue(NanNew<String>(RUBY_OBJECT_TAG), NanTrue());
  
  return NanEscapeScope(v8Obj);
}

VALUE RubyObject::FromV8(Handle<Object> owner)
{
  NanScope();
  
  // TODO: Is this the best way to do this? Should we add the hidden prop to
  // the owner object?
  Local<Value> wrappedVal = owner->Get(NanNew<String>("_rubyObj"));
  if (wrappedVal->IsObject()) {
    Local<Object> wrappedObj = wrappedVal.As<Object>();
    Local<Value> tag =
      wrappedObj->GetHiddenValue(NanNew<String>(RubyObject::RUBY_OBJECT_TAG));
    if (!tag.IsEmpty() && tag->IsTrue()) {
      RubyObject* self = node::ObjectWrap::Unwrap<RubyObject>(wrappedObj);
      return self->m_obj;
    }
  }
  
  return Qnil;
}

Local<Function> RubyObject::GetCtor(VALUE klass)
{
  NanEscapableScope();

  Local<FunctionTemplate> tpl;
  TplMap::iterator it = s_functionTemplates.find(klass);
  if (it == s_functionTemplates.end()) {
    log("Creating new class: " << rb_class2name(klass));

    tpl = NanNew<FunctionTemplate>();
    tpl->SetClassName(NanNew<String>(rb_class2name(klass)));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Instance methods
    VALUE methods = rb_class_public_instance_methods(1, &trueArg, klass);
    for (int i = 0; i < RARRAY_LEN(methods); i++) {
      ID methodID = SYM2ID(rb_ary_entry(methods, i));
      Local<String> methodName = NanNew<String>(rb_id2name(methodID));

      Local<FunctionTemplate> methodTemplate =
        NanNew<FunctionTemplate>(CallInstanceMethod,
                                 EXTERNAL_WRAP((void*)methodID));
      tpl->PrototypeTemplate()->Set(methodName, methodTemplate->GetFunction());
    }
    
#if (NODE_MODULE_VERSION > 0x000B)
    s_functionTemplates[klass].Reset(v8::Isolate::GetCurrent(), tpl);
#else
    NanAssignPersistent(s_functionTemplates[klass], tpl);
#endif
  }
  else {
    log("Getting existing class: " << rb_class2name(klass));
    
#if (NODE_MODULE_VERSION > 0x000B)    
    tpl = Local<FunctionTemplate>::New(v8::Isolate::GetCurrent(), it->second);
#else
    tpl = NanNew<FunctionTemplate>(it->second);
#endif
  }

  return NanEscapeScope(tpl->GetFunction());
}

NAN_WEAK_CALLBACK(OwnerWeakCB) {}

RubyObject::RubyObject(VALUE obj, Local<Object> owner) :
  m_obj(obj), m_owner(NULL)
{
  rb_gc_register_address(&m_obj);
  
  assert(!owner->IsUndefined());
  m_owner = &NanMakeWeakPersistent(owner, (void*)NULL, OwnerWeakCB)->persistent;
  m_owner->MarkIndependent();
  
  VALUE wrappedObj = Data_Wrap_Struct(s_wrappedClass, NULL, NULL, this);
  rb_ivar_set(obj, V8_WRAPPER_ID, wrappedObj);
}

RubyObject::~RubyObject()
{
  log("~RubyObject");
  rb_gc_unregister_address(&m_obj);
}

NAN_METHOD(RubyObject::CallInstanceMethod)
{
  NanScope();

  RubyObject *self = node::ObjectWrap::Unwrap<RubyObject>(args.This());
  NanReturnValue(CallRubyFromV8(self->m_obj, args));
}

VALUE RubyObject::CallV8Method(int argc, VALUE* argv, VALUE self)
{
  log("In CallV8Method: " <<  rb_id2name(rb_frame_this_func()));
  
  NanScope();
  
  Local<Object> owner = RubyObject::RubyUnwrap(self);
  VALUE rbName = rb_id2str(rb_frame_this_func());
  Local<String> v8Name =
    NanNew<String>(RSTRING_PTR(rbName), RSTRING_LEN(rbName));
  Local<Value> func = owner->Get(v8Name);
  
  if (func->IsFunction())
    return CallV8FromRuby(owner, func.As<Function>(), argc, argv);
  else {
    rb_raise(rb_eTypeError, "Property '%s' of object %s is not a function",
             rb_id2name(rb_frame_this_func()), *String::Utf8Value(owner));
    return Qnil;
  }
}
