#include "Ruby.h"
#include "RubyValue.h"
#include <ruby/encoding.h>
#include <limits>

using namespace v8;

const int32_t MIN_INT32 = std::numeric_limits<int32_t>::min();
const int32_t MAX_INT32 = std::numeric_limits<int32_t>::max();

void Ruby::Init(Handle<Object> exports)
{
  static char* argv[] = { (char*)"norby", (char*)"-e", (char*)"" };

  RUBY_INIT_STACK;
  ruby_init();
  ruby_options(3, argv);

  node::AtExit(Cleanup);

  RubyValue::Init();
  
  exports->Set(NanNew<String>("Object"), RubyValue::New(rb_cObject));
  exports->Set(NanNew<String>("RubyValue"), RubyValue::GetRubyValueCtor());

  NODE_SET_METHOD(exports, "getSymbol", GetSymbol);
  
  // TODO: Should these be RubyValue members?
  NODE_SET_METHOD(exports, "jsStrToRuby", JsStrToRuby);
  NODE_SET_METHOD(exports, "jsNumToRuby", JsNumToRuby);
  
  NODE_SET_METHOD(exports, "rubyStrToJS", RubyStrToJS);
  NODE_SET_METHOD(exports, "rubyFixnumToJS", RubyFixnumToJS);
  NODE_SET_METHOD(exports, "rubyFloatToJS", RubyFloatToJS);
  
  Local<Object> types = NanNew<Object>();
  exports->Set(NanNew<String>("types"), types);
  types->Set(NanNew<String>("NONE"), NanNew<Integer>(T_NONE));
  types->Set(NanNew<String>("NIL"), NanNew<Integer>(T_NIL));
  types->Set(NanNew<String>("OBJECT"), NanNew<Integer>(T_OBJECT));
  types->Set(NanNew<String>("CLASS"), NanNew<Integer>(T_CLASS));
  types->Set(NanNew<String>("ICLASS"), NanNew<Integer>(T_ICLASS));
  types->Set(NanNew<String>("MODULE"), NanNew<Integer>(T_MODULE));
  types->Set(NanNew<String>("FLOAT"), NanNew<Integer>(T_FLOAT));
  types->Set(NanNew<String>("STRING"), NanNew<Integer>(T_STRING));
  types->Set(NanNew<String>("REGEXP"), NanNew<Integer>(T_REGEXP));
  types->Set(NanNew<String>("ARRAY"), NanNew<Integer>(T_ARRAY));
  types->Set(NanNew<String>("HASH"), NanNew<Integer>(T_HASH));
  types->Set(NanNew<String>("STRUCT"), NanNew<Integer>(T_STRUCT));
  types->Set(NanNew<String>("BIGNUM"), NanNew<Integer>(T_BIGNUM));
  types->Set(NanNew<String>("FILE"), NanNew<Integer>(T_FILE));
  types->Set(NanNew<String>("FIXNUM"), NanNew<Integer>(T_FIXNUM));
  types->Set(NanNew<String>("TRUE"), NanNew<Integer>(T_TRUE));
  types->Set(NanNew<String>("FALSE"), NanNew<Integer>(T_FALSE));
  types->Set(NanNew<String>("DATA"), NanNew<Integer>(T_DATA));
  types->Set(NanNew<String>("MATCH"), NanNew<Integer>(T_MATCH));
  types->Set(NanNew<String>("SYMBOL"), NanNew<Integer>(T_SYMBOL));
  types->Set(NanNew<String>("RATIONAL"), NanNew<Integer>(T_RATIONAL));
  types->Set(NanNew<String>("COMPLEX"), NanNew<Integer>(T_COMPLEX));
}

void Ruby::Cleanup(void*)
{
  ruby_cleanup(0);
}

NAN_METHOD(Ruby::GetSymbol)
{
  NanScope();
  
  VALUE sym = ID2SYM(rb_intern(*String::Utf8Value(args[0])));

  NanReturnValue(RubyValue::New(sym));
}

NAN_METHOD(Ruby::JsStrToRuby)
{
  NanScope();
  
  Handle<String> str = args[0].As<String>();
  VALUE rbStr = rb_enc_str_new(NULL, str->Utf8Length(), rb_utf8_encoding());
  str->WriteUtf8(RSTRING_PTR(rbStr));

  NanReturnValue(RubyValue::New(rbStr));
}

NAN_METHOD(Ruby::JsNumToRuby)
{
  NanScope();
  
  Handle<Value> v8Num = args[0];
  VALUE rbNum = Qnil;
  if (v8Num->IsInt32())
    rbNum = INT2NUM(v8Num->Int32Value());
  else if (v8Num->IsUint32())
    rbNum = UINT2NUM(v8Num->Uint32Value());
  else if (v8Num->IsNumber())
    rbNum = rb_float_new(v8Num->NumberValue());

  NanReturnValue(RubyValue::New(rbNum));
}

NAN_METHOD(Ruby::RubyStrToJS)
{
  NanScope();

  VALUE rbStr = *node::ObjectWrap::Unwrap<RubyValue>(args[0].As<Object>());
  int encIdx = rb_enc_get_index(rbStr);
  if (encIdx != rb_usascii_encindex() && encIdx != rb_utf8_encindex() &&
      encIdx != rb_ascii8bit_encindex()) {
    rbStr = rb_str_conv_enc(rbStr, rb_enc_from_index(encIdx),
                            rb_utf8_encoding());
  }

  NanReturnValue(NanNew<String>(RSTRING_PTR(rbStr), RSTRING_LEN(rbStr)));
}

NAN_METHOD(Ruby::RubyFixnumToJS)
{
  NanScope();
  
  VALUE val = *node::ObjectWrap::Unwrap<RubyValue>(args[0].As<Object>());
  
  long longVal = FIX2LONG(val);
  if (longVal >= MIN_INT32 && longVal <= MAX_INT32) {
    NanReturnValue(NanNew<Integer>(longVal));
  }
  else
    NanReturnValue(NanNew<Number>(longVal));
}

NAN_METHOD(Ruby::RubyFloatToJS)
{
  NanScope();
  
  VALUE val = *node::ObjectWrap::Unwrap<RubyValue>(args[0].As<Object>());
  
  NanReturnValue(NanNew<Number>(RFLOAT_VALUE(val)));
}

NODE_MODULE(norby, Ruby::Init)
