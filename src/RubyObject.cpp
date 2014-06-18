#include "RubyObject.h"
#include "common.h"

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
    
    Local<FunctionTemplate> tpl = FunctionTemplate::New(New2);
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
  NewInstanceCaller(int c, VALUE* v, VALUE k) : argc(c), argv(v), klass(k) {}
  VALUE operator()() const
  {
    return rb_class_new_instance(argc, argv, klass);
  }

  int argc;
  VALUE* argv;
  VALUE klass;
};

Local<Value> RubyObject::New(VALUE klass, int argc, VALUE* argv)
{
  // TODO: If the created type is a builtin (string, array, etc.) what should this return?
  HandleScope scope;

  const char* className = rb_class2name(klass);
  cout << "Creating new " << className << " with " << argc << " args" << endl;

  VALUE ex;
  VALUE obj = SafeRubyCall(NewInstanceCaller(argc, argv, klass), ex);
  if (ex != Qnil) {
    ThrowException(rubyExToV8(ex));
    return scope.Close(Undefined());
  }

  // Wrap the obj immediately to prevent it from being garbage collected
  RubyObject *self = new RubyObject(obj);

  Local<Function> ctor = RubyObject::GetClass(klass);
  Local<Object> rubyObj = ctor->NewInstance();
  self->Wrap(rubyObj);

  return scope.Close(rubyObj);
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
    obj(o), methodID(m), argc(args.Length()), argv(NULL)
  {
    if (argc > 0) {
      argv = new VALUE[argc];

      // TODO: Is this right? Is there a way to determine if a block is expected?
      if (args[argc-1]->IsFunction()) {
        cout << "Got a func!" << endl;
        block = args[--argc].As<Function>();
      }

      for (int i = 0; i < argc; i++) {
        argv[i] = v8ToRuby(args[i]);
      }
    }
  }
  ~MethodCaller()
  {
    delete [] argv;
  }

  VALUE operator()() const
  {
    if (block.IsEmpty())
      return rb_funcall2(obj, methodID, argc, argv);
    else {
      // TODO: Probably not available in Ruby < 1.9
      return rb_block_call(obj, methodID, argc, argv, RUBY_METHOD_FUNC(BlockFunc), (VALUE)this);
    }
  }

  static VALUE BlockFunc(VALUE, VALUE data, int argc, const VALUE* rbArgv)
  {
    HandleScope scope;
    
    Handle<Value>* argv = new Handle<Value>[argc];
    for (int i = 0; i < argc; i++) {
      argv[i] = rubyToV8(rbArgv[i]);
    }

    MethodCaller* self = reinterpret_cast<MethodCaller*>(data);
    Handle<Value> ret = node::MakeCallback(Context::GetCurrent()->Global(),
                                           self->block, argc, argv);
    delete argv;

    return v8ToRuby(ret);
  }

  VALUE obj;
  VALUE methodID;
  int argc;
  VALUE* argv;
  // TODO: Should this be persistent?
  Local<Function> block;
};

Handle<Value> RubyObject::CallMethod(const Arguments& args)
{
  HandleScope scope;

  RubyObject *self = node::ObjectWrap::Unwrap<RubyObject>(args.This());
  ID methodID = ID(External::Unwrap(args.Data()));

  cout << "Calling method: " << rb_id2name(methodID) << " with " << args.Length() << " args" << endl;

  VALUE ex;
  VALUE res = SafeRubyCall(MethodCaller(self->m_obj, methodID, args), ex);
  if (ex != Qnil) {
    return scope.Close(ThrowException(rubyExToV8(ex)));
  }

  return scope.Close(rubyToV8(res));
}
