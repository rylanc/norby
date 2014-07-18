#ifndef NORBY_RUBY_H_
#define NORBY_RUBY_H_

#include <node.h>
#include <nan.h>
#include <ruby.h>

class Ruby : public node::ObjectWrap
{
 public:
  static void Init(v8::Handle<v8::Object> module);
  static void Cleanup(void*);
 
 private:

  static NAN_METHOD(GetSymbol);
  
  static NAN_METHOD(V8StrToRuby);
  static NAN_METHOD(V8NumToRuby);
  
  static NAN_METHOD(RubyStrToV8);
  static NAN_METHOD(RubyBoolToV8);
  static NAN_METHOD(RubyNumToV8);
  
  // Don't instantiate
  Ruby();
  ~Ruby();
};

#endif // NORBY_RUBY_H_
