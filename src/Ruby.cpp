#include "Ruby.h"
#include "RubyObject.h"
//#include "common.h"
#include <cstring>
#include <string>

using namespace v8;

void Ruby::Init(Handle<Object> exports)
{
  static char* argv[] = { (char*)"norby", (char*)"-e", (char*)"" };

  RUBY_INIT_STACK;
  ruby_init();
  ruby_options(3, argv);

  node::AtExit(Cleanup);

  RubyObject::Init();
  
  exports->Set(NanNew<String>("Object"), RubyObject::New(rb_cObject));
  exports->Set(NanNew<String>("RubyValue"), RubyObject::GetRubyValueCtor());

  NODE_SET_METHOD(exports, "getSymbol", GetSymbol);
  
  NODE_SET_METHOD(exports, "v8StrToRuby", V8StrToRuby);
  NODE_SET_METHOD(exports, "v8NumToRuby", V8NumToRuby);
  
  NODE_SET_METHOD(exports, "rubyStrToV8", RubyStrToV8);
  // TODO: Do we need this?
  NODE_SET_METHOD(exports, "rubyBoolToV8", RubyBoolToV8);
  NODE_SET_METHOD(exports, "rubyNumToV8", RubyNumToV8);
}

void Ruby::Cleanup(void*)
{
  ruby_cleanup(0);
}

NAN_METHOD(Ruby::GetSymbol)
{
  NanScope();
  
  VALUE sym = ID2SYM(rb_intern(*String::Utf8Value(args[0])));

  NanReturnValue(RubyObject::New(sym));
}

NAN_METHOD(Ruby::V8StrToRuby)
{
  NanScope();
  
  // TODO: We should handle encoding here
  Handle<String> str = args[0].As<String>();
  VALUE rbStr = rb_str_new(NULL, str->Utf8Length());
  str->WriteUtf8(RSTRING_PTR(rbStr));

  NanReturnValue(RubyObject::New(rbStr));
}

NAN_METHOD(Ruby::V8NumToRuby)
{
  NanScope();
  
  Handle<Value> v8Num = args[0];
  VALUE rbNum;
  if (v8Num->IsInt32())
    rbNum = INT2NUM(v8Num->Int32Value());
  else if (v8Num->IsUint32())
    rbNum = UINT2NUM(v8Num->Uint32Value());
  else if (v8Num->IsNumber())
    rbNum = rb_float_new(v8Num->NumberValue());

  NanReturnValue(RubyObject::New(rbNum));
}

NAN_METHOD(Ruby::RubyStrToV8)
{
  NanScope();
  
  VALUE rbStr = *node::ObjectWrap::Unwrap<RubyObject>(args[0].As<Object>());

  NanReturnValue(NanNew<String>(RSTRING_PTR(rbStr), RSTRING_LEN(rbStr)));
}

NAN_METHOD(Ruby::RubyBoolToV8)
{
  NanScope();
  
  VALUE val = *node::ObjectWrap::Unwrap<RubyObject>(args[0].As<Object>());

  if (RTEST(val))
    NanReturnValue(NanTrue());
  else
    NanReturnValue(NanFalse());
}

NAN_METHOD(Ruby::RubyNumToV8)
{
  NanScope();

  VALUE val = *node::ObjectWrap::Unwrap<RubyObject>(args[0].As<Object>());
  
  // TODO: Fix this
  NanReturnValue(NanNew<Number>(rb_num2dbl(val)));
}

NODE_MODULE(norby, Ruby::Init)
