#include "Ruby.h"
#include "RubyObject.h"
#include "common.h"
#include <cstring>

#include <iostream>
using namespace std;

using namespace v8;

Persistent<Function> Ruby::s_getCtor;
VALUE Ruby::BLOCK_WRAPPER_CLASS;

void Ruby::Init(Handle<Object> module)
{
  static char* argv[] = { (char*)"norby", (char*)"-e", (char*)"" };

  RUBY_INIT_STACK;
  ruby_init();
  ruby_options(3, argv);
  
  BLOCK_WRAPPER_CLASS = rb_define_class("BlockWrapper", rb_cObject);
  
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
  NODE_SET_METHOD(bindings, "eval", Eval);
  NODE_SET_METHOD(bindings, "getMethod", GetMethod);
  // TODO: Maybe we should load the constants here and place them in an object?
  NODE_SET_METHOD(bindings, "getConstant", GetConstant);
  
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

struct ConstGetter
{
  ConstGetter(Handle<Value> nv) : nameVal(nv) {}
  VALUE operator()() const
  {
    NanScope();

    Local<String> constName = nameVal->ToString();
    String::Utf8Value constStr(constName);
    
    ID id;
    VALUE mod;
    const char* split = std::strstr(*constStr, "::");
    if (split) {
      id = rb_intern(split + 2);
      mod = rb_const_get(rb_cObject, rb_intern2(*constStr, split - *constStr));
    }
    else {
      id = rb_intern2(*constStr, constStr.length());
      mod = rb_cObject;
    }

    return rb_const_get(mod, id);
  }

  Handle<Value> nameVal;
};

// TODO: Should/could we combine this with RubyObject::GetClass?
NAN_METHOD(Ruby::GetClass)
{
  NanScope();

  VALUE klass;
  SAFE_RUBY_CALL(klass, ConstGetter(args[0]));
  
  if (TYPE(klass) != T_CLASS) {
    std::string msg(*String::Utf8Value(args[0]));
    msg.append(" is not a class");
    // TODO: TypeError?
    NanThrowError(msg.c_str());
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

struct ClassDefiner
{
  ClassDefiner(Handle<Value> nv, VALUE s) : nameVal(nv), super(s) {}
  VALUE operator()() const
  {
    NanScope();
    
    Local<String> className = nameVal->ToString();
    return rb_define_class(*String::Utf8Value(className), super);
  }
  
  Handle<Value> nameVal;
  VALUE super;
};

NAN_METHOD(Ruby::DefineClass)
{
  NanScope();

  Local<String> name = args[0]->ToString();
  log("Inherit called for " << *String::Utf8Value(name) << endl);

  VALUE super;
  SAFE_RUBY_CALL(super, ConstGetter(args[1]));
  VALUE klass;
  SAFE_RUBY_CALL(klass, ClassDefiner(args[0], super));
  
  NanReturnValue(RubyObject::GetClass(klass));
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
  VALUE res;
  SAFE_RUBY_CALL(res, EvalCaller(*String::Utf8Value(str)));

  NanReturnValue(rubyToV8(res));
}

// TODO: Can this be combined with RubyObject::CallMethod? Maybe rename?
NAN_METHOD(CallMethod)
{
  NanScope();
  NanReturnValue(CallRubyFromV8(rb_cObject, args));
}

// TODO: Should this throw immediately if the function doesnt exist?
NAN_METHOD(Ruby::GetMethod)
{
  NanScope();

  Local<String> name = args[0]->ToString();
  ID methodID = rb_intern(*String::Utf8Value(name));
  Local<Function> func =
    NanNew<FunctionTemplate>(CallMethod, EXTERNAL_WRAP((void*)methodID))->GetFunction();
  func->SetName(name);

  NanReturnValue(func);
}

NAN_METHOD(Ruby::GetConstant)
{
  NanScope();

  VALUE constant;
  SAFE_RUBY_CALL(constant, ConstGetter(args[0]));
  
  // TODO: Should we allow getting classes this way? Maybe throw an exception?
  NanReturnValue(rubyToV8(constant));
}
