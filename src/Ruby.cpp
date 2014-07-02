#include "Ruby.h"
#include "RubyObject.h"
#include "common.h"

#include <iostream>
using namespace std;

using namespace v8;

Persistent<Function> Ruby::s_getCtor;

void Ruby::Init(Handle<Object> module)
{
  int argc = 0;
  char** argv = NULL;

  // TODO: Do we need to call this?
  ruby_sysinit(&argc, &argv);
  RUBY_INIT_STACK;
  ruby_init();
  ruby_init_loadpath();
  
  node::AtExit(Cleanup);
  
  module->Set(NanNew<String>("exports"),
              NanNew<FunctionTemplate>(New)->GetFunction());
}

void Ruby::Cleanup(void*)
{
  log("Cleaning up!" << endl);
  RubyObject::Cleanup();
  ruby_cleanup(0);
}

NAN_METHOD(Ruby::New)
{
  NanScope();
  
  assert(args[0]->IsFunction());
  NanAssignPersistent(s_getCtor, args[0].As<Function>());
  
  Local<Object> bindings = NanNew<Object>();
  NODE_SET_METHOD(bindings, "_getClass", GetClass);
  NODE_SET_METHOD(bindings, "_gcStart", GCStart);
  NODE_SET_METHOD(bindings, "_defineClass", DefineClass);
  NODE_SET_METHOD(bindings, "require", Require);
  NODE_SET_METHOD(bindings, "eval", Eval);
  // TODO: Right name?
  NODE_SET_METHOD(bindings, "getFunction", GetFunction);
  
  NanReturnValue(bindings);
}

Local<Function> Ruby::GetCtor(Local<Function> rubyClass)
{
  NanEscapableScope();
  
  Local<Function> getCtor = NanNew<Function>(s_getCtor);
  Handle<Value> argv[] = { rubyClass };
  return NanEscapeScope(NanMakeCallback(NanGetCurrentContext()->Global(),
                                        getCtor, 1, argv).As<Function>());
}

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
NAN_METHOD(Ruby::GetClass)
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

NAN_METHOD(Ruby::GCStart)
{
  NanScope();
  rb_gc_start();

  NanReturnUndefined();
}

NAN_METHOD(Ruby::DefineClass)
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

  // TODO: Can this throw?
  VALUE klass = rb_define_class(*String::Utf8Value(name), super);
  Local<Function> ctor = RubyObject::GetClass(klass, true);
  
  NanReturnValue(ctor);
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

NAN_METHOD(Ruby::Require)
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

struct EvalCaller
{
  EvalCaller(const char* s) : str(s) {}
  VALUE operator()() const
  {
    return rb_eval_string(str);
  }

  const char* str;
};

NAN_METHOD(Ruby::Eval)
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
NAN_METHOD(Ruby::GetFunction)
{
  NanScope();

  Local<String> name = args[0]->ToString();
  ID methodID = rb_intern(*String::Utf8Value(name));
  Local<Function> func =
    NanNew<FunctionTemplate>(CallMethod, EXTERNAL_WRAP((void*)methodID))->GetFunction();
  func->SetName(name);

  NanReturnValue(func);
}
