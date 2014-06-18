#include "RubyObject.h"
#include "common.h"
#include <vector>

#include <iostream>
using namespace std;

using namespace v8;

// TODO:
// - Where do we need to rb_rescue?
// - Modules?
// - Class inheritance/extending?

VALUE trueArg = Qtrue;
RubyObject::TplMap RubyObject::s_functionTemplates;

void dumpRubyArgs(int argc, VALUE* argv)
{
  for (int i = 0; i < argc; i++) {
    cout << i << ": " << StringValueCStr(argv[i]) << endl;
  }
}

Local<Function> RubyObject::GetClass(VALUE klass)
{
  HandleScope scope;
  
  Persistent<FunctionTemplate> persistTpl;
  TplMap::iterator it = s_functionTemplates.find(klass);
  if (it == s_functionTemplates.end()) {
    cout << "Creating new class: " << rb_class2name(klass) << endl;
    
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
    cout << "Getting existing class: " << rb_class2name(klass) << endl;
    
    persistTpl = it->second;
  }

  Local<Function> ctor = persistTpl->GetFunction();
  
  return scope.Close(ctor);
}

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

Handle<Value> RubyObject::New(const Arguments& args)
{
  HandleScope scope;
  
  if (args.IsConstructCall()) {
    VALUE klass = VALUE(External::Unwrap(args.Data()));
    
    std::vector<VALUE> rubyArgs(args.Length());
    for (int i = 0; i < args.Length(); i++) {
      rubyArgs[i] = v8ToRuby(args[i]);
    }
    
    cout << "Creating new " << rb_class2name(klass) << " with " << rubyArgs.size() << " args" << endl;
    
    VALUE ex;
    VALUE obj = SafeRubyCall(NewInstanceCaller(rubyArgs, klass), ex);
    if (ex != Qnil) {
      ThrowException(rubyExToV8(ex));
      return scope.Close(Undefined());
    }
    
    // Wrap the obj immediately to prevent it from being garbage collected
    RubyObject *self = new RubyObject(obj);
    self->Wrap(args.This());
    
    return scope.Close(args.This());
  }
  else {
    cerr << "Blerrrrg!" << endl;
    
    return scope.Close(Undefined());
  }
}

RubyObject::RubyObject(VALUE obj) : m_obj(obj)
{
  rb_gc_register_address(&m_obj);
}

RubyObject::~RubyObject()
{
  cout << "~RubyObject" << endl;
  rb_gc_unregister_address(&m_obj);
}

struct MethodCaller
{
  MethodCaller(VALUE o, VALUE m, const Arguments& args) :
    obj(o), methodID(m), rubyArgs(args.Length())
  {
    // TODO: Is this right? Is there a way to determine if a block is expected?
    if (args.Length() > 0 && args[args.Length()-1]->IsFunction()) {
      cout << "Got a func!" << endl;
      block = args[args.Length()-1].As<Function>();
      rubyArgs.resize(args.Length()-1);
    }
    
    for (size_t i = 0; i < rubyArgs.size(); i++) {
      rubyArgs[i] = v8ToRuby(args[i]);
    }
  }

  VALUE operator()() const
  {
    cout << "Calling method: " << rb_id2name(methodID) << " with " <<
      rubyArgs.size() << " args";
      
    if (block.IsEmpty()) {
      cout << endl;
      
      return rb_funcall2(obj, methodID, rubyArgs.size(), (VALUE*)&rubyArgs[0]);
    }
    else {
      cout << " and a block" << endl;
      
      // TODO: Probably not available in Ruby < 1.9
      return rb_block_call(obj, methodID, rubyArgs.size(),
                           (VALUE*)&rubyArgs[0], RUBY_METHOD_FUNC(BlockFunc),
                           (VALUE)this);
    }
  }

  static VALUE BlockFunc(VALUE, VALUE data, int argc, const VALUE* rbArgv)
  {
    HandleScope scope;
    
    std::vector<Handle<Value> > v8Args(argc);
    for (int i = 0; i < argc; i++) {
      v8Args[i] = rubyToV8(rbArgv[i]);
    }

    MethodCaller* self = reinterpret_cast<MethodCaller*>(data);
    Handle<Value> ret = node::MakeCallback(Context::GetCurrent()->Global(),
                                           self->block, argc, &v8Args[0]);

    return v8ToRuby(ret);
  }

  VALUE obj;
  VALUE methodID;
  std::vector<VALUE> rubyArgs;
  // TODO: Should this be persistent?
  Local<Function> block;
};

Handle<Value> RubyObject::CallMethod(const Arguments& args)
{
  HandleScope scope;

  RubyObject *self = node::ObjectWrap::Unwrap<RubyObject>(args.This());
  ID methodID = ID(External::Unwrap(args.Data()));

  VALUE ex;
  VALUE res = SafeRubyCall(MethodCaller(self->m_obj, methodID, args), ex);
  if (ex != Qnil) {
    return scope.Close(ThrowException(rubyExToV8(ex)));
  }

  return scope.Close(rubyToV8(res));
}
