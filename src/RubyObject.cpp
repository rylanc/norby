#include "RubyObject.h"
#include "common.h"
#include <vector>

#include <iostream>
using namespace std;

using namespace v8;

VALUE trueArg = Qtrue;

const char* RubyObject::RUBY_OBJECT_TAG = "_IsRubyObject";
RubyObject::TplMap RubyObject::s_functionTemplates;

Local<Function> RubyObject::GetClass(VALUE klass)
{
  HandleScope scope;
  
  Persistent<FunctionTemplate> persistTpl;
  TplMap::iterator it = s_functionTemplates.find(klass);
  if (it == s_functionTemplates.end()) {
    log("Creating new class: " << rb_class2name(klass) << endl);
    
    Local<FunctionTemplate> tpl = FunctionTemplate::New(New, External::Wrap((void*)klass));
    tpl->SetClassName(String::NewSymbol(rb_class2name(klass)));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    VALUE methods = rb_class_public_instance_methods(1, &trueArg, klass);
    for (int i = 0; i < RARRAY_LEN(methods); i++) {
      ID methodID = SYM2ID(rb_ary_entry(methods, i));
      Local<String> methodName = String::New(rb_id2name(methodID));

      Local<FunctionTemplate> methodTemplate =
        FunctionTemplate::New(CallMethod, External::Wrap((void*)methodID));
      tpl->PrototypeTemplate()->Set(methodName, methodTemplate->GetFunction());
    }

    persistTpl = s_functionTemplates[klass] = Persistent<FunctionTemplate>::New(tpl);
  }
  else {
    log("Getting existing class: " << rb_class2name(klass) << endl);
    
    persistTpl = it->second;
  }

  Local<Function> ctor = persistTpl->GetFunction();
  
  return scope.Close(ctor);
}

void OwnerWeakCB(Persistent<Value> value, void *data)
{
  assert(value.IsNearDeath());
  value.ClearWeak();
  value.Dispose();
  value.Clear();
  
  Persistent<Object>* owner = static_cast<Persistent<Object>*>(data);
  delete owner;
}

struct NewInstanceCaller
{
  NewInstanceCaller(std::vector<VALUE> &r, VALUE k, void* d) :
    rubyArgs(r), klass(k), data(d) {}

  VALUE operator()() const
  {
    if (data == NULL) {
      return rb_class_new_instance(rubyArgs.size(), &rubyArgs[0], klass);
    }
    else {
      VALUE obj = Data_Wrap_Struct(klass, NULL, NULL, data);
      rb_obj_call_init(obj, rubyArgs.size(), &rubyArgs[0]);
      return obj;
    }
  }

  std::vector<VALUE>& rubyArgs;
  VALUE klass;
  void* data;
};

Handle<Value> RubyObject::New(const Arguments& args)
{
  HandleScope scope;
  
  if (args.IsConstructCall()) {
    VALUE klass = VALUE(External::Unwrap(args.Data()));

    Persistent<Object>* owner = NULL;
    if (!args[0]->IsUndefined()) {
      owner = new Persistent<Object>(Persistent<Object>::New(args[0].As<Object>()));
      owner->MakeWeak(owner, OwnerWeakCB);
      // TODO: Keep this?
      owner->MarkIndependent();
    }
    
    Local<Array> v8Args = args[1].As<Array>();
    std::vector<VALUE> rubyArgs(v8Args->Length());
    for (uint32_t i = 0; i < v8Args->Length(); i++) {
      rubyArgs[i] = v8ToRuby(v8Args->Get(i));
    }
    
    log("Creating new " << rb_class2name(klass) << " with " << rubyArgs.size() << " args" << endl);
    
    VALUE ex;
    VALUE obj = SafeRubyCall(NewInstanceCaller(rubyArgs, klass, owner), ex);
    if (ex != Qnil) {
      ThrowException(rubyExToV8(ex));
      return scope.Close(Undefined());
    }
    
    // Wrap the obj immediately to prevent it from being garbage collected
    RubyObject *self = new RubyObject(obj);
    self->Wrap(args.This());
    
    args.This()->SetHiddenValue(String::New(RUBY_OBJECT_TAG), True());
    
    return scope.Close(args.This());
  }
  else {
    // TODO: Do we even need this?
    
    std::vector<Handle<Value> > argv(args.Length());
    for (int i = 0; i < args.Length(); i++) {
      argv[i] = args[i];
    }
    
    VALUE klass = VALUE(External::Unwrap(args.Data()));
    Local<Function> cons = RubyObject::GetClass(klass);
    
    return scope.Close(cons->NewInstance(args.Length(), &argv[0]));
  }
}

RubyObject::RubyObject(VALUE obj) : m_obj(obj)
{
  rb_gc_register_address(&m_obj);
}

RubyObject::~RubyObject()
{
  log("~RubyObject" << endl);
  rb_gc_unregister_address(&m_obj);
}

Handle<Value> RubyObject::CallMethod(const Arguments& args)
{
  HandleScope scope;

  RubyObject *self = node::ObjectWrap::Unwrap<RubyObject>(args.This());
  return scope.Close(CallRubyFromV8(self->m_obj, args));
}
