#include <node.h>
#include <v8.h>
#include <ruby.h>
#include "RubyObject.h"
#include "common.h"

#include <iostream>
using namespace std;

using namespace v8;

struct ClassGetter
{
  ClassGetter(Handle<Value> nv) : nameVal(nv) {}
  VALUE operator()() const
  {
    HandleScope scope;

    Local<String> className = nameVal->ToString();
    ID id = rb_intern(*String::Utf8Value(className));
    return rb_const_get(rb_cObject, id);
  }

  Handle<Value> nameVal;
};

// TODO: Should/could we combine this with RubyObject::GetClass?
Handle<Value> GetClass(const Arguments& args) {
  HandleScope scope;

  VALUE ex;
  VALUE klass = SafeRubyCall(ClassGetter(args[0]), ex);
  if (ex != Qnil) {
    return scope.Close(ThrowException(rubyExToV8(ex)));
  }
  
  return scope.Close(RubyObject::GetClass(klass));
}

struct RequireCaller
{
  RequireCaller(const char* n) : name(n) {}
  VALUE operator()() const
  {
    return rb_require(name);
  }

  const char* name;
};

Handle<Value> Require(const Arguments& args)
{
  HandleScope scope;

  Local<String> name = args[0]->ToString();
  VALUE ex;
  VALUE res = SafeRubyCall(RequireCaller(*String::Utf8Value(name)), ex);
  if (ex != Qnil) {
    return scope.Close(ThrowException(rubyExToV8(ex)));
  }

  return scope.Close(rubyToV8(res));
}

// TODO: Is this the best signature?
VALUE MethodMissing(int argc, VALUE* argv, VALUE self)
{
  assert(argc > 0);
  
  HandleScope scope;
  
  Persistent<Object>* owner;
  Data_Get_Struct(self, Persistent<Object>, owner);
  
  VALUE rbName = rb_id2str(SYM2ID(argv[0]));
  
  log("MethodMissing called for " << RSTRING_PTR(rbName) << endl);
  
  Local<String> v8Name = String::NewSymbol(RSTRING_PTR(rbName), RSTRING_LEN(rbName));
  Local<Value> prop = (*owner)->Get(v8Name);
  log(RSTRING_PTR(rbName) << " is: " << *String::Utf8Value(prop) << endl);
  if (prop->IsFunction()) {
    return CallV8FromRuby(*owner, prop.As<Function>(), argc-1, argv+1);
  }
  else
    return rb_call_super(argc, argv);
}

Handle<Value> DefineClass(const Arguments& args)
{
  HandleScope scope;

  Local<String> name = args[0]->ToString();
  log("Inherit called for " << *String::Utf8Value(name) << endl);

  VALUE ex;
  VALUE super = SafeRubyCall(ClassGetter(args[1]), ex);
  if (ex != Qnil) {
    return scope.Close(ThrowException(rubyExToV8(ex)));
  }

  VALUE klass = rb_define_class(*String::Utf8Value(name), super);
  Local<Function> ctor = RubyObject::GetClass(klass);
  
  // TODO: Implement responds_to? method
  rb_define_method(klass, "method_missing", RUBY_METHOD_FUNC(MethodMissing), -1);
  
  return scope.Close(ctor);
}

struct EvalCaller
{
  EvalCaller(const char* s) : str(s) {}
  VALUE operator()() const
  {
    return rb_eval_string(str);
  }

  const char* str;
};

Handle<Value> Eval(const Arguments& args)
{
  HandleScope scope;

  Local<String> str = args[0]->ToString();
  VALUE ex;
  VALUE res = SafeRubyCall(EvalCaller(*String::Utf8Value(str)), ex);
  if (ex != Qnil) {
    return scope.Close(ThrowException(rubyExToV8(ex)));
  }

  return scope.Close(rubyToV8(res));
}

// TODO: Can this be combined with RubyObject::CallMethod? Maybe rename?
Handle<Value> CallMethod(const Arguments& args)
{
  HandleScope scope;
  return scope.Close(CallRubyFromV8(rb_cObject, args));
}

// TODO: Should this throw immediately if the function doesnt exist?
Handle<Value> GetFunction(const Arguments& args)
{
  HandleScope scope;

  Local<String> name = args[0]->ToString();
  ID methodID = rb_intern(*String::Utf8Value(name));
  Local<Function> func =
    FunctionTemplate::New(CallMethod, External::Wrap((void*)methodID))->GetFunction();
  func->SetName(name);

  return scope.Close(func);
}

Handle<Value> GCStart(const Arguments& args)
{
  HandleScope scope;
  rb_gc_start();

  return scope.Close(Undefined());
}

void CleanupRuby(void*)
{
  log("Cleaning up!" << endl);
  ruby_cleanup(0);
}

void Init(Handle<Object> exports) {
  int argc = 0;
  char** argv = NULL;

  // TODO: Do we need to call this?
  ruby_sysinit(&argc, &argv);
  RUBY_INIT_STACK;
  ruby_init();
  ruby_init_loadpath();

  node::AtExit(CleanupRuby);
               
  exports->Set(String::NewSymbol("_getClass"),
               FunctionTemplate::New(GetClass)->GetFunction());

  exports->Set(String::NewSymbol("_gcStart"),
               FunctionTemplate::New(GCStart)->GetFunction());
               
  exports->Set(String::NewSymbol("_defineClass"),
               FunctionTemplate::New(DefineClass)->GetFunction());

  exports->Set(String::NewSymbol("require"),
               FunctionTemplate::New(Require)->GetFunction());
               
  exports->Set(String::NewSymbol("eval"),
               FunctionTemplate::New(Eval)->GetFunction());
  
  // TODO: Right name?             
  exports->Set(String::NewSymbol("getFunction"),
               FunctionTemplate::New(GetFunction)->GetFunction());
}

NODE_MODULE(ruby_bridge, Init)
