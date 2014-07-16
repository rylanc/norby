#include "Ruby.h"
#include "RubyObject.h"
#include "RubyModule.h"
#include "common.h"
#include <cstring>
#include <string>

using namespace v8;

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
  LOG("Cleaning up!");
  RubyObject::Cleanup();
  ruby_cleanup(0);
}

NAN_METHOD(Ruby::New)
{
  NanScope();
  
  assert(args[0]->IsObject());
  Local<Object> ctors = args[0].As<Object>();
  
  Local<Function> createCtor =
    ctors->Get(NanNew<String>("createCtor")).As<Function>();
  assert(createCtor->IsFunction());
  RubyModule::SetCreateCtor(createCtor);

  Local<Object> bindings = NanNew<Object>();
  NODE_SET_METHOD(bindings, "_getClass", GetClass);
  NODE_SET_METHOD(bindings, "_defineClass", DefineClass);
  NODE_SET_METHOD(bindings, "getMethod", GetMethod);
  // TODO: Maybe we should load the constants here and place them in an object?
  NODE_SET_METHOD(bindings, "getConstant", GetConstant);

  NanReturnValue(bindings);
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

// TODO: Do we still need this and .getConstant?
NAN_METHOD(Ruby::GetClass)
{
  NanScope();

  VALUE klass;
  SAFE_RUBY_CALL(klass, ConstGetter(args[0]));

  // TODO: rb_is_class_id?
  if (TYPE(klass) != T_CLASS) {
    std::string msg(*String::Utf8Value(args[0]));
    msg.append(" is not a class");
    NanThrowTypeError(msg.c_str());
    NanReturnUndefined();
  }

  NanReturnValue(RubyModule::ToV8(klass));
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

  LOG("Inherit called for " << *String::Utf8Value(args[0]));

  VALUE super;
  SAFE_RUBY_CALL(super, ConstGetter(args[1]));
  VALUE klass;
  SAFE_RUBY_CALL(klass, ClassDefiner(args[0], super));

  NanReturnValue(RubyModule::ToV8(klass));
}

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
