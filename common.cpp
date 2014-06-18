#include "common.h"
#include <cassert>

#include <iostream>
using namespace std;

using namespace v8;

Handle<Value> rubyToV8(VALUE val)
{
  HandleScope scope;

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
  case T_FIXNUM:
    return scope.Close(Integer::New(NUM2INT(val)));
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

  cout << "Converting " << *String::Utf8Value(val->ToDetailString()) << " to ruby" << endl;

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
  else {
    String::Utf8Value str(val->ToDetailString());
    cerr << "Unknown v8 type: " << *str << endl;
    return Qnil;
  }
}

Handle<Value> rubyExToV8(VALUE ex)
{
  HandleScope scope;

  assert(rb_obj_is_kind_of(ex, rb_eException) == Qtrue);

  VALUE msg = rb_funcall(ex, rb_intern("message"), 0);
  Local<String> msgStr = String::New(RSTRING_PTR(msg), RSTRING_LEN(msg));

  VALUE klass = rb_class_of(ex);
  if (klass == rb_eArgError)
    return scope.Close(Exception::Error(msgStr));
  else if (klass == rb_eTypeError)
    return scope.Close(Exception::TypeError(msgStr));
  else {
    cerr << "Unknown exception: " << rb_obj_classname(ex) << endl;
    return scope.Close(Exception::Error(msgStr));
  }
}

VALUE RescueCB(VALUE data, VALUE ex)
{
  VALUE *storedEx = reinterpret_cast<VALUE*>(data);
  *storedEx = ex;

  return Qnil;
}
