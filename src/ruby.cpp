#include <node.h>
#include <nan.h>
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
    NanScope();

    Local<String> className = nameVal->ToString();
    ID id = rb_intern(*String::Utf8Value(className));
    return rb_const_get(rb_cObject, id);
  }

  Handle<Value> nameVal;
};

// TODO: Should/could we combine this with RubyObject::GetClass?
NAN_METHOD(GetClass)
{
  NanScope();

  VALUE ex;
  VALUE klass = SafeRubyCall(ClassGetter(args[0]), ex);
  if (ex != Qnil) {
    NanThrowError(rubyExToV8(ex));
    NanReturnUndefined();
  }
  
  NanReturnValue(RubyObject::GetClass(klass));
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

NAN_METHOD(Require)
{
  NanScope();

  Local<String> name = args[0]->ToString();
  VALUE ex;
  VALUE res = SafeRubyCall(RequireCaller(*String::Utf8Value(name)), ex);
  if (ex != Qnil) {
    NanThrowError(rubyExToV8(ex));
    NanReturnUndefined();
  }

  NanReturnValue(rubyToV8(res));
}

// TODO: Is this the best signature?
VALUE MethodMissing(int argc, VALUE* argv, VALUE self)
{
  assert(argc > 0);
  
  NanScope();
  
  Persistent<Object>* persistOwner;
  Data_Get_Struct(self, Persistent<Object>, persistOwner);
  Local<Object> owner = NanNew<Object>(*persistOwner);
  
  VALUE rbName = rb_id2str(SYM2ID(argv[0]));
  
  log("MethodMissing called for " << RSTRING_PTR(rbName) << endl);
  
  Local<String> v8Name = NanNew<String>(RSTRING_PTR(rbName), RSTRING_LEN(rbName));
  Local<Value> prop = owner->Get(v8Name);
  log(RSTRING_PTR(rbName) << " is: " << *String::Utf8Value(prop) << endl);
  if (prop->IsFunction()) {
    return CallV8FromRuby(owner, prop.As<Function>(), argc-1, argv+1);
  }
  else
    return rb_call_super(argc, argv);
}

NAN_METHOD(DefineClass)
{
  NanScope();

  Local<String> name = args[0]->ToString();
  log("Inherit called for " << *String::Utf8Value(name) << endl);

  VALUE ex;
  VALUE super = SafeRubyCall(ClassGetter(args[1]), ex);
  if (ex != Qnil) {
    NanThrowError(rubyExToV8(ex));
    NanReturnUndefined();
  }

  VALUE klass = rb_define_class(*String::Utf8Value(name), super);
  Local<Function> ctor = RubyObject::GetClass(klass);
  
  // TODO: Implement responds_to? method
  rb_define_method(klass, "method_missing", RUBY_METHOD_FUNC(MethodMissing), -1);
  
  NanReturnValue(ctor);
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

NAN_METHOD(Eval)
{
  NanScope();

  Local<String> str = args[0]->ToString();
  // TODO: This pattern is pretty common. Can we move it to a function/macro?
  VALUE ex;
  VALUE res = SafeRubyCall(EvalCaller(*String::Utf8Value(str)), ex);
  if (ex != Qnil) {
    NanThrowError(rubyExToV8(ex));
    NanReturnUndefined();
  }

  NanReturnValue(rubyToV8(res));
}

// TODO: Can this be combined with RubyObject::CallMethod? Maybe rename?
NAN_METHOD(CallMethod)
{
  NanScope();
  NanReturnValue(CallRubyFromV8(rb_cObject, args));
}

// TODO: Should this throw immediately if the function doesnt exist?
NAN_METHOD(GetFunction)
{
  NanScope();

  Local<String> name = args[0]->ToString();
  ID methodID = rb_intern(*String::Utf8Value(name));
  Local<Function> func =
    NanNew<FunctionTemplate>(CallMethod, EXTERNAL_WRAP((void*)methodID))->GetFunction();
  func->SetName(name);

  NanReturnValue(func);
}

NAN_METHOD(GCStart)
{
  NanScope();
  rb_gc_start();

  NanReturnUndefined();
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
               
  exports->Set(NanNew<String>("_getClass"),
               NanNew<FunctionTemplate>(GetClass)->GetFunction());

  exports->Set(NanNew<String>("_gcStart"),
               NanNew<FunctionTemplate>(GCStart)->GetFunction());
               
  exports->Set(NanNew<String>("_defineClass"),
               NanNew<FunctionTemplate>(DefineClass)->GetFunction());

  exports->Set(NanNew<String>("require"),
               NanNew<FunctionTemplate>(Require)->GetFunction());
               
  exports->Set(NanNew<String>("eval"),
               NanNew<FunctionTemplate>(Eval)->GetFunction());
  
  // TODO: Right name?             
  exports->Set(NanNew<String>("getFunction"),
               NanNew<FunctionTemplate>(GetFunction)->GetFunction());
}

NODE_MODULE(ruby_bridge, Init)
