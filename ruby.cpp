#include <node.h>
#include <v8.h>
#include <ruby.h>
#include "RubyObject.h"
#include "common.h"

#include <iostream>
using namespace std;

using namespace v8;

inline
VALUE GetClass(Handle<Value> nameVal)
{
  HandleScope scope;

  Local<String> className = nameVal->ToString();

  ID id = rb_intern(*String::Utf8Value(className));
  return rb_const_get(rb_cObject, id);
}

Handle<Value> NewInstance(const Arguments& args) {
  HandleScope scope;

  if (args.Length() == 0) {
    ThrowException(Exception::Error(String::New("no class name specified")));
    return scope.Close(Undefined());
  }

  VALUE klass = GetClass(args[0]);

  int argc = args.Length() - 1;
  VALUE* argv = NULL;
  if (argc > 0) {
    // TODO: Is there any smart ptr we can use here?
    argv = new VALUE[argc];
    for (int i = 1; i < args.Length(); i++) {
      argv[i - 1] = v8ToRuby(args[i]);
    }
  }

  Local<Value> rubyObj = RubyObject::New(klass, argc, argv);
  delete [] argv;

  return scope.Close(rubyObj);
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

VALUE CallMe(VALUE, VALUE data)
{
  HandleScope scope;

  Persistent<Function>* func = reinterpret_cast<Persistent<Function>*>(data);
  Handle<Value> argv[] = {};
  node::MakeCallback(Context::GetCurrent()->Global(), *func, 0, argv);

  return Qnil;
}

Handle<Value> Inherits(const Arguments& args)
{
  HandleScope scope;

  Local<Function> cons = args[0].As<Function>();
  Local<String> name = cons->GetName()->ToString();
  cout << "Inherit called for " << *String::Utf8Value(name) << endl;

  VALUE super = GetClass(args[1]);

  VALUE klass = rb_define_class(*String::Utf8Value(name), super);

  Local<Object> proto = cons->Get(String::NewSymbol("prototype")).As<Object>();
  Local<Array> props = proto->GetPropertyNames();
  for (uint32_t i = 0; i < props->Length(); i++) {
    Local<Value> key = props->Get(i);
    cout << *String::Utf8Value(key->ToString()) << " ? ";
    Local<Value> prop = proto->Get(key);
    cout << prop->IsFunction() << endl;
    if (prop->IsFunction()) {
      String::Utf8Value funcName(key);
      Persistent<Function> *func = new Persistent<Function>(prop.As<Function>());
      //Handle<Value> argv[] = {};
      //node::MakeCallback(Context::GetCurrent()->Global(), *func, 0, argv);
      VALUE proc = rb_proc_new(RUBY_METHOD_FUNC(CallMe), VALUE(func));
      rb_funcall(klass, rb_intern("define_method"), 2, rb_str_new(*funcName, funcName.length()), proc);
    }
  }

  return scope.Close(RubyObject::New(klass, 0, NULL));
}

Handle<Value> GCStart(const Arguments& args)
{
  HandleScope scope;
  rb_gc_start();

  return scope.Close(Undefined());
}

void CleanupRuby(void*)
{
  cout << "Cleaning up!" << endl;
  ruby_cleanup(0);
}

void Init(Handle<Object> exports) {
  char* argv[] = { "node" };
  int argc = 1;
  char** argv2 = argv;

  // TODO: Do we need to call this?
  ruby_sysinit(&argc, &argv2);
  RUBY_INIT_STACK;
  ruby_init();
  ruby_init_loadpath();

  node::AtExit(CleanupRuby);

  exports->Set(String::NewSymbol("newInstance"),
               FunctionTemplate::New(NewInstance)->GetFunction());

  exports->Set(String::NewSymbol("gcStart"),
               FunctionTemplate::New(GCStart)->GetFunction());

  exports->Set(String::NewSymbol("require"),
               FunctionTemplate::New(Require)->GetFunction());

  exports->Set(String::NewSymbol("inherits"),
               FunctionTemplate::New(Inherits)->GetFunction());
}

NODE_MODULE(ruby, Init)
