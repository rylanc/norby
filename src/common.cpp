#include "common.h"
#include "RubyObject.h"
#include <node.h>
#include <cassert>
#include <vector>
#include <limits>

#include <iostream>
using namespace std;

using namespace v8;

const int32_t MIN_INT32 = std::numeric_limits<int32_t>::min();
const int32_t MAX_INT32 = std::numeric_limits<int32_t>::max();

Handle<Value> rubyToV8(VALUE val)
{
  HandleScope scope;

  log("Converting " << RSTRING_PTR(rb_funcall2(val, rb_intern("to_s"), 0, NULL)) << " to v8" << endl);

  int type = TYPE(val);
  switch (type) {
  case T_NONE:
  case T_NIL: // TODO: Is this right?
    return Undefined();
  case T_FLOAT:
    return scope.Close(Number::New(RFLOAT_VALUE(val)));
  case T_STRING:
    return scope.Close(String::New(RSTRING_PTR(val), RSTRING_LEN(val)));
  case T_ARRAY: {
    int len = RARRAY_LEN(val);
    Local<Array> array = Array::New(len);
    for (int i = 0; i < len; i++) {
      array->Set(i, rubyToV8(rb_ary_entry(val, i)));
   }

    return scope.Close(array);
  }
  case T_FIXNUM: {
    long longVal = FIX2LONG(val);
    if (longVal >= MIN_INT32 && longVal <= MAX_INT32) {
      return scope.Close(Integer::New(longVal));
    }
    else
      return scope.Close(Number::New(longVal));
  }
  case T_BIGNUM:
    return scope.Close(Number::New(rb_num2long(val)));
  case T_TRUE:
    return scope.Close(True());
  case T_FALSE:
    return scope.Close(False());
  default:
    cerr << "Unknown ruby type: " << rb_obj_classname(val) << endl;
    return Undefined();
  }
}

VALUE v8ToRuby(Handle<Value> val)
{
  HandleScope scope;

  log("Converting " << *String::Utf8Value(val->ToDetailString()) << " to ruby" << endl);

  if (val->IsUndefined())
    return Qnil; // TODO: Is this right?
  else if (val->IsNull())
    return Qnil; // TODO: Is this right?
  else if (val->IsTrue())
    return Qtrue;
  else if (val->IsFalse())
    return Qfalse;
  else if (val->IsString()) {
    // TODO: Is there any way to Write the string directly into the rb string?
    String::Utf8Value str(val);
    return rb_str_new(*str, str.length());
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
    Local<Value> wrappedVal = val.As<Object>()->Get(String::New("_rubyObj"));
    if (wrappedVal->IsObject()) {
      Local<Object> wrappedObj = wrappedVal.As<Object>();
      Local<Value> hidden =
        wrappedObj->GetHiddenValue(String::New(RubyObject::RUBY_OBJECT_TAG));
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
  HandleScope scope;

  assert(rb_obj_is_kind_of(ex, rb_eException) == Qtrue);

  VALUE msg = rb_funcall(ex, rb_intern("message"), 0);
  Local<String> msgStr = String::New(RSTRING_PTR(msg), RSTRING_LEN(msg));

  VALUE klass = rb_class_of(ex);
  if (klass == rb_eArgError ||
      klass == rb_eLoadError)
    return scope.Close(Exception::Error(msgStr));
  else if (klass == rb_eNameError)
    return scope.Close(Exception::ReferenceError(msgStr));
  else if (klass == rb_eTypeError)
    return scope.Close(Exception::TypeError(msgStr));
  else if (klass == rb_eSyntaxError)
    return scope.Close(Exception::SyntaxError(msgStr));
  else {
    cerr << "Unknown ruby exception: " << rb_obj_classname(ex) << endl;
    return scope.Close(Exception::Error(msgStr));
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
  HandleScope scope;
  
  std::vector<Handle<Value> > v8Args(argc);
  for (int i = 0; i < argc; i++) {
    v8Args[i] = rubyToV8(argv[i]);
  }
  
  Handle<Value> ret = node::MakeCallback(recv, callback, argc, &v8Args[0]);
  
  return v8ToRuby(ret);
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
  HandleScope scope;
  
  Local<Array> propNames = obj->GetPropertyNames();
  for (uint32_t i = 0; i < propNames->Length(); i++) {
    Local<Value> key = propNames->Get(i);
    
    cout << *String::Utf8Value(key) << endl;
  }
#endif
}

void DumpV8Args(const Arguments& args)
{
#ifdef _DEBUG
  HandleScope scope;
  
  for (int i = 0; i < args.Length(); i++) {
    cout << i << ": " << *String::Utf8Value(args[i]) << endl;
  }
#endif
}
