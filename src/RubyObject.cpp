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

Local<Function> RubyObject::GetClass(VALUE klass)
{
  NanEscapableScope();

  Local<FunctionTemplate> tpl;
  TplMap::iterator it = s_functionTemplates.find(klass);
  if (it == s_functionTemplates.end()) {
    log("Creating new class: " << rb_class2name(klass));
    
    tpl = NanNew<FunctionTemplate>(New, EXTERNAL_WRAP((void*)klass));
    tpl->SetClassName(NanNew<String>(rb_class2name(klass)));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    VALUE methods = rb_class_public_instance_methods(1, &trueArg, klass);
    for (int i = 0; i < RARRAY_LEN(methods); i++) {
      ID methodID = SYM2ID(rb_ary_entry(methods, i));
      Local<String> methodName = NanNew<String>(rb_id2name(methodID));

      Local<FunctionTemplate> methodTemplate =
        NanNew<FunctionTemplate>(CallInstanceMethod, EXTERNAL_WRAP((void*)methodID));
      tpl->PrototypeTemplate()->Set(methodName, methodTemplate->GetFunction());
    }
    
    tpl->PrototypeTemplate()->SetInternalFieldCount(1);
    // TODO: Is this right? Not sure we get all the methods that the Class obj would expose.. (e.g. define_method)
    methods = rb_obj_singleton_methods(1, &trueArg, klass);
    for (int i = 0; i < RARRAY_LEN(methods); i++) {
      ID methodID = SYM2ID(rb_ary_entry(methods, i));
      Local<String> methodName = NanNew<String>(rb_id2name(methodID));
      
      Local<FunctionTemplate> methodTemplate =
        NanNew<FunctionTemplate>(CallClassMethod, EXTERNAL_WRAP((void*)methodID));
      tpl->Set(methodName, methodTemplate->GetFunction());
    }
    
    // TODO: Should we expose this to clients?
    Local<FunctionTemplate> defineMethodTpl = NanNew<FunctionTemplate>(DefineMethod);
    tpl->Set(NanNew<String>("_defineMethod"), defineMethodTpl->GetFunction());
      

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
  
  // TODO: Should we be caching the functions instead of the templates?
  Local<Function> fn = tpl->GetFunction();
  Local<Object> proto = fn->Get(NanNew<String>("prototype")).As<Object>();
  assert(proto->InternalFieldCount() > 0);
  NanSetInternalFieldPointer(proto, 0, (void*)klass);
  
  return NanEscapeScope(fn);
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

// TODO: Idea! What if instead of calling new RubyClass, we called the Ruby Class's .new function...
NAN_METHOD(RubyObject::New)
{
  NanScope();
  
  if (args.IsConstructCall()) {
    VALUE klass = VALUE(EXTERNAL_UNWRAP(args.Data()));
    
    Local<Array> v8Args = args[1].As<Array>();
    VALUE obj = Qnil;
    if (v8Args->Length() == 1 && v8Args->Get(0)->IsExternal()) {
      log("Wrapping existing " << rb_class2name(klass));
      obj = VALUE(EXTERNAL_UNWRAP(v8Args->Get(0)));
    }
    else {
      std::vector<VALUE> rubyArgs(v8Args->Length());
      for (uint32_t i = 0; i < v8Args->Length(); i++) {
        rubyArgs[i] = v8ToRuby(v8Args->Get(i));
      }
    
      log("Creating new " << rb_class2name(klass) << " with " << rubyArgs.size() << " args");
      SAFE_RUBY_CALL(obj, NewInstanceCaller(rubyArgs, klass));
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
  log("~RubyObject");
  rb_gc_unregister_address(&m_obj);
}

NAN_METHOD(RubyObject::CallInstanceMethod)
{
  NanScope();

  RubyObject *self = node::ObjectWrap::Unwrap<RubyObject>(args.This());
  NanReturnValue(CallRubyFromV8(self->m_obj, args));
}

NAN_METHOD(RubyObject::CallClassMethod)
{
  NanScope();
  
  Local<Object> proto =
    args.This()->Get(NanNew<String>("prototype")).As<Object>();
  VALUE klass = VALUE(NanGetInternalFieldPointer(proto, 0));

  NanReturnValue(CallRubyFromV8(klass, args));
}

NAN_METHOD(RubyObject::DefineMethod)
{
  NanScope();
  
  Local<Object> proto =
    args.This()->Get(NanNew<String>("prototype")).As<Object>();
  VALUE klass = VALUE(NanGetInternalFieldPointer(proto, 0));
  
  log("Defining method " << rb_class2name(klass) << "." << *String::Utf8Value(args[0]));
  
  if (!args[1]->IsFunction()) {
    // TODO: Should we do this check in JS?
    std::string errMsg("fn must be a function: ");
    errMsg.append(*String::Utf8Value(args[1]));
    NanThrowTypeError(errMsg.c_str());
    NanReturnUndefined();
  }

  rb_define_method(klass, *String::Utf8Value(args[0]),
                   RUBY_METHOD_FUNC(CallV8Method), -1);
  
  NanReturnUndefined();
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
