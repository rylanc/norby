#include "Ruby.h"
#include "RubyValue.h"
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
  NODE_SET_METHOD(exports, "rubyBoolToJS", RubyBoolToJS);
  NODE_SET_METHOD(exports, "rubyFixnumToJS", RubyFixnumToJS);
  NODE_SET_METHOD(exports, "rubyFloatToJS", RubyFloatToJS);
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
  
  // TODO: We should handle encoding here
  Handle<String> str = args[0].As<String>();
  VALUE rbStr = rb_str_new(NULL, str->Utf8Length());
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

  NanReturnValue(NanNew<String>(RSTRING_PTR(rbStr), RSTRING_LEN(rbStr)));
}

NAN_METHOD(Ruby::RubyBoolToJS)
{
  NanScope();
  
  VALUE val = *node::ObjectWrap::Unwrap<RubyValue>(args[0].As<Object>());

  if (RTEST(val))
    NanReturnValue(NanTrue());
  else
    NanReturnValue(NanFalse());
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
