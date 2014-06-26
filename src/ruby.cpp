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
    Handle<Value> v8Args[] = {};
    // TODO: Convert args and ret val
    node::MakeCallback(*owner, prop.As<Function>(), 0, v8Args);
    return Qnil;
  }
  else
    return rb_call_super(argc, argv);
}

Handle<Value> Inherits(const Arguments& args)
{
  HandleScope scope;

  Local<Function> cons = args[0].As<Function>();
  Local<String> name = cons->GetName()->ToString();
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

Handle<Value> GCStart(const Arguments& args)
{
  HandleScope scope;
  rb_gc_start();

  return scope.Close(Undefined());
}

Handle<Value> TestExternal(const Arguments& args)
{
  HandleScope scope;
  
  ID id = rb_intern("String");
  VALUE klass = rb_const_get(rb_cObject, id);
  
  return scope.Close(External::Wrap((void*)klass));
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
               
  exports->Set(String::NewSymbol("getClass"),
               FunctionTemplate::New(GetClass)->GetFunction());

  exports->Set(String::NewSymbol("gcStart"),
               FunctionTemplate::New(GCStart)->GetFunction());

  exports->Set(String::NewSymbol("require"),
               FunctionTemplate::New(Require)->GetFunction());

  exports->Set(String::NewSymbol("_rubyInherits"),
               FunctionTemplate::New(Inherits)->GetFunction());
               
  exports->Set(String::NewSymbol("testExternal"),
               FunctionTemplate::New(TestExternal)->GetFunction());
}

NODE_MODULE(ruby_bridge, Init)
