#include "common.h"
#include "Ruby.h"
#include "RubyObject.h"
#include <node.h>
#include <cassert>
#include <limits>

#include <iostream>
using namespace std;

using namespace v8;

const int32_t MIN_INT32 = std::numeric_limits<int32_t>::min();
const int32_t MAX_INT32 = std::numeric_limits<int32_t>::max();

Handle<Value> rubyToV8(VALUE val)
{
  NanEscapableScope();

  log("Converting " << RSTRING_PTR(rb_funcall2(val, rb_intern("to_s"), 0, NULL)) << " to v8" << endl);

  int type = TYPE(val);
  switch (type) {
  case T_NONE:
  case T_NIL: // TODO: Is this right?
     return NanEscapeScope(NanUndefined());
  case T_FLOAT:
     return NanEscapeScope(NanNew<Number>(RFLOAT_VALUE(val)));
  case T_STRING:
     return NanEscapeScope(NanNew<String>(RSTRING_PTR(val), RSTRING_LEN(val)));
  case T_ARRAY: {
    int len = RARRAY_LEN(val);
    Local<Array> array = NanNew<Array>(len);
    for (int i = 0; i < len; i++) {
      array->Set(i, rubyToV8(rb_ary_entry(val, i)));
    }

    return NanEscapeScope(array);
  }
  case T_FIXNUM: {
    long longVal = FIX2LONG(val);
    if (longVal >= MIN_INT32 && longVal <= MAX_INT32) {
      return NanEscapeScope(NanNew<Integer>(longVal));
    }
    else
      return NanEscapeScope(NanNew<Number>(longVal));
  }
  case T_BIGNUM:
    return NanEscapeScope(NanNew<Number>(rb_num2long(val)));
  case T_TRUE:
    return NanEscapeScope(NanTrue());
  case T_FALSE:
    return NanEscapeScope(NanFalse());
  case T_OBJECT: 
  case T_DATA: {
    Local<Object> owner = RubyObject::RubyUnwrap(val);
    if (owner.IsEmpty()) {
      VALUE klass = rb_class_of(val);
      Local<Function> rubyClass = RubyObject::GetClass(klass);
      
      Local<Function> ctor = Ruby::GetCtor(rubyClass);
      Handle<Value> argv[] = { External::New((void*)val) };
      return NanEscapeScope(ctor->NewInstance(1, argv));
    }
    else
      return NanEscapeScope(owner);
  }
  default:
    cerr << "Unknown ruby type(" << type << "): " << rb_obj_classname(val) << endl;
    return NanEscapeScope(NanUndefined());
  }
}

VALUE v8ToRuby(Handle<Value> val)
{
  NanScope();

  log("Converting " << *String::Utf8Value(val) << " to ruby" << endl);

  if (val->IsUndefined())
    return Qnil; // TODO: Is this right?
  else if (val->IsNull())
    return Qnil; // TODO: Is this right?
  else if (val->IsTrue())
    return Qtrue;
  else if (val->IsFalse())
    return Qfalse;
  else if (val->IsString()) {
    // TODO: We should handle encoding here
    Handle<String> str = val.As<String>();
    VALUE rbStr = rb_str_new(NULL, str->Utf8Length());
    str->WriteUtf8(RSTRING_PTR(rbStr));
    return rbStr;
  }
  else if (val->IsArray()) {
    Handle<Array> v8Arr = val.As<Array>();
    VALUE rbArr = rb_ary_new2(v8Arr->Length());
    for (uint32_t i = 0; i < v8Arr->Length(); i++) {
      rb_ary_store(rbArr, i, v8ToRuby(v8Arr->Get(i)));
    }

    return rbArr;
  }
  else if (val->IsInt32())
    return INT2NUM(val->Int32Value());
  else if (val->IsUint32())
    return UINT2NUM(val->Uint32Value());
  else if (val->IsNumber())
    return rb_float_new(val->NumberValue());
  else if (val->IsObject()) {
    // TODO: Is this the best way to do this? Should we add the hidden prop to
    // the owner object?
    Local<Value> wrappedVal = val.As<Object>()->Get(NanNew<String>("_rubyObj"));
    if (wrappedVal->IsObject()) {
      Local<Object> wrappedObj = wrappedVal.As<Object>();
      Local<Value> hidden =
        wrappedObj->GetHiddenValue(NanNew<String>(RubyObject::RUBY_OBJECT_TAG));
      if (!hidden.IsEmpty() && hidden->IsTrue()) {
        RubyObject* rubyObj = node::ObjectWrap::Unwrap<RubyObject>(wrappedObj);
        return rubyObj->GetObject();
      }
    }
  }
  
  // TODO: Should we wrap objects here?
  String::Utf8Value str(val->ToDetailString());
  cerr << "Unknown v8 type: " << *str << endl;
  return Qnil;
}

Handle<Value> rubyExToV8(VALUE ex)
{
  NanEscapableScope();

  assert(rb_obj_is_kind_of(ex, rb_eException) == Qtrue);

  VALUE msg = rb_funcall(ex, rb_intern("message"), 0);
  Local<String> msgStr = NanNew<String>(RSTRING_PTR(msg), RSTRING_LEN(msg));

  // TODO: Do these error constructions work in all node versions?
  VALUE klass = rb_class_of(ex);
  if (klass == rb_eArgError ||
      klass == rb_eLoadError)
    return NanEscapeScope(Exception::Error(msgStr));
  else if (klass == rb_eNameError ||
           klass == rb_eNoMethodError)
    return NanEscapeScope(Exception::ReferenceError(msgStr));
  else if (klass == rb_eTypeError)
    return NanEscapeScope(Exception::TypeError(msgStr));
  else if (klass == rb_eSyntaxError)
    return NanEscapeScope(Exception::SyntaxError(msgStr));
  else {
    cerr << "Unknown ruby exception: " << rb_obj_classname(ex) << endl;
    return NanEscapeScope(Exception::Error(msgStr));
  }
}

VALUE RescueCB(VALUE data, VALUE ex)
{
  VALUE *storedEx = reinterpret_cast<VALUE*>(data);
  *storedEx = ex;

  return Qnil;
}

VALUE CallV8FromRuby(const Handle<Object> recv,
                     const Handle<Function> callback,
                     int argc, const VALUE* argv)
{
  NanScope();
  
  std::vector<Handle<Value> > v8Args(argc);
  for (int i = 0; i < argc; i++) {
    v8Args[i] = rubyToV8(argv[i]);
  }
  
  Handle<Value> ret = NanMakeCallback(recv, callback, argc, &v8Args[0]);
  
  return v8ToRuby(ret);
}

struct MethodCaller
{
  inline
  MethodCaller(VALUE o, _NAN_METHOD_ARGS_TYPE args):
    obj(o), methodID(ID(EXTERNAL_UNWRAP(args.Data()))), rubyArgs(args.Length())
  {
    // TODO: Is this right? Is there a way to determine if a block is expected?
    if (args.Length() > 0 && args[args.Length()-1]->IsFunction()) {
      block = args[args.Length()-1].As<Function>();
      rubyArgs.resize(args.Length()-1);
    }
    
    for (size_t i = 0; i < rubyArgs.size(); i++) {
      rubyArgs[i] = v8ToRuby(args[i]);
    }
  }
  
  VALUE operator()() const
  {
    log("Calling method: " << rb_obj_classname(obj) << "." <<
        rb_id2name(methodID) << " with " << rubyArgs.size() << " args");
    
    if (block.IsEmpty()) {
      log(endl);
    
      return rb_funcall2(obj, methodID, rubyArgs.size(), (VALUE*)&rubyArgs[0]);
    }
    else {
      log(" and a block" << endl);
      
      // TODO: Probably not available in Ruby < 1.9
      return rb_block_call(obj, methodID, rubyArgs.size(), (VALUE*)&rubyArgs[0],
                           RUBY_METHOD_FUNC(BlockFunc), (VALUE)this);
    }
  }
  
  static VALUE BlockFunc(VALUE, VALUE data, int argc, const VALUE* rbArgv)
  {
    MethodCaller* self = reinterpret_cast<MethodCaller*>(data);
  
    return CallV8FromRuby(NanGetCurrentContext()->Global(), self->block,
                          argc, rbArgv);
  }

  VALUE obj;
  VALUE methodID;
  std::vector<VALUE> rubyArgs;
  // TODO: Should this be persistent?
  Local<Function> block;
};

Handle<Value> CallRubyFromV8(VALUE recv, _NAN_METHOD_ARGS_TYPE args)
{
  NanEscapableScope();

  VALUE ex;
  VALUE res = SafeRubyCall(MethodCaller(recv, args), ex);
  if (ex != Qnil) {
    NanThrowError(rubyExToV8(ex));
    return NanEscapeScope(NanUndefined());
  }

  return NanEscapeScope(rubyToV8(res));
}

void DumpRubyArgs(int argc, VALUE* argv)
{
#ifdef _DEBUG
  for (int i = 0; i < argc; i++) {
    VALUE str = rb_funcall2(argv[i], rb_intern("to_s"), 0, NULL);
    cout << i << ": " << StringValueCStr(str) << endl;
  }
#endif
}

void DumpV8Props(Handle<Object> obj)
{
#ifdef _DEBUG
  NanScope();
  
  Local<Array> propNames = obj->GetPropertyNames();
  for (uint32_t i = 0; i < propNames->Length(); i++) {
    Local<Value> key = propNames->Get(i);
    
    cout << *String::Utf8Value(key) << endl;
  }
#endif
}

void DumpV8Args(_NAN_METHOD_ARGS_TYPE args)
{
#ifdef _DEBUG
  NanScope();
  
  for (int i = 0; i < args.Length(); i++) {
    cout << i << ": " << *String::Utf8Value(args[i]) << endl;
  }
#endif
}
