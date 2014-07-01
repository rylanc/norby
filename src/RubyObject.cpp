#include "RubyObject.h"
#include "common.h"
#include <vector>

#include <iostream>
using namespace std;

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

Local<Function> RubyObject::GetClass(VALUE klass)
{
  NanEscapableScope();

  Local<FunctionTemplate> tpl;
  TplMap::iterator it = s_functionTemplates.find(klass);
  if (it == s_functionTemplates.end()) {
    log("Creating new class: " << rb_class2name(klass) << endl);
    
    tpl = NanNew<FunctionTemplate>(New, EXTERNAL_WRAP((void*)klass));
    tpl->SetClassName(NanNew<String>(rb_class2name(klass)));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    VALUE methods = rb_class_public_instance_methods(1, &trueArg, klass);
    for (int i = 0; i < RARRAY_LEN(methods); i++) {
      ID methodID = SYM2ID(rb_ary_entry(methods, i));
      Local<String> methodName = NanNew<String>(rb_id2name(methodID));

      Local<FunctionTemplate> methodTemplate =
        NanNew<FunctionTemplate>(CallMethod, EXTERNAL_WRAP((void*)methodID));
      tpl->PrototypeTemplate()->Set(methodName, methodTemplate->GetFunction());
    }

#if (NODE_MODULE_VERSION > 0x000B)
    s_functionTemplates[klass].Reset(v8::Isolate::GetCurrent(), tpl);
#else
    NanAssignPersistent(s_functionTemplates[klass], tpl);
#endif
  }
  else {
    log("Getting existing class: " << rb_class2name(klass) << endl);
    
#if (NODE_MODULE_VERSION > 0x000B)    
    tpl = Local<FunctionTemplate>::New(v8::Isolate::GetCurrent(), it->second);
#else
    tpl = NanNew<FunctionTemplate>(it->second);
#endif
  }

  return NanEscapeScope(tpl->GetFunction());
}

NAN_WEAK_CALLBACK(OwnerWeakCB)
{}

struct NewInstanceCaller
{
  NewInstanceCaller(std::vector<VALUE> &r, VALUE k) : rubyArgs(r), klass(k) {}

  VALUE operator()() const
  {
    return rb_class_new_instance(rubyArgs.size(), &rubyArgs[0], klass);
  }

  std::vector<VALUE>& rubyArgs;
  VALUE klass;
};

NAN_METHOD(RubyObject::New)
{
  NanScope();
  
  if (args.IsConstructCall()) {
    VALUE klass = VALUE(EXTERNAL_UNWRAP(args.Data()));
    
    VALUE obj = Qnil;
    if (args[1]->IsUndefined()) {
      Local<Array> v8Args = args[2].As<Array>();
      std::vector<VALUE> rubyArgs(v8Args->Length());
      for (uint32_t i = 0; i < v8Args->Length(); i++) {
        rubyArgs[i] = v8ToRuby(v8Args->Get(i));
      }
    
      log("Creating new " << rb_class2name(klass) << " with " << rubyArgs.size() << " args" << endl);
    
      VALUE ex;
      obj = SafeRubyCall(NewInstanceCaller(rubyArgs, klass), ex);
      if (ex != Qnil) {
        NanThrowError(rubyExToV8(ex));
        NanReturnUndefined();
      }
    }
    else {
      obj = VALUE(EXTERNAL_UNWRAP(args[1]));
    }
    
    // Wrap the obj immediately to prevent it from being garbage collected
    RubyObject *self = new RubyObject(obj, args[0]);
    self->Wrap(args.This());
    args.This()->SetHiddenValue(NanNew<String>(RUBY_OBJECT_TAG), NanTrue());
    
    NanReturnValue(args.This());
  }
  else {
    // TODO: Do we even need this?
    
    std::vector<Handle<Value> > argv(args.Length());
    for (int i = 0; i < args.Length(); i++) {
      argv[i] = args[i];
    }
    
    VALUE klass = VALUE(EXTERNAL_UNWRAP(args.Data()));
    Local<Function> cons = RubyObject::GetClass(klass);
    
    NanReturnValue(cons->NewInstance(args.Length(), &argv[0]));
  }
}

RubyObject::RubyObject(VALUE obj, Local<v8::Value> owner) :
  m_obj(obj), m_owner(NULL)
{
  rb_gc_register_address(&m_obj);
  
  if (!owner->IsUndefined()) {
    // TODO: Does this get properly cleaned up?
    m_owner = &NanMakeWeakPersistent(owner.As<Object>(), (void*)NULL,
                                     OwnerWeakCB)->persistent;
    // TODO: Keep this?
    m_owner->MarkIndependent();
  }
  
  VALUE wrappedObj = Data_Wrap_Struct(s_wrappedClass, NULL, NULL, this);
  rb_ivar_set(obj, V8_WRAPPER_ID, wrappedObj);
}

RubyObject::~RubyObject()
{
  log("~RubyObject" << endl);
  rb_gc_unregister_address(&m_obj);
}

NAN_METHOD(RubyObject::CallMethod)
{
  NanScope();

  RubyObject *self = node::ObjectWrap::Unwrap<RubyObject>(args.This());
  NanReturnValue(CallRubyFromV8(self->m_obj, args));
}
