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
  
  static NAN_METHOD(JsStrToRuby);
  static NAN_METHOD(JsNumToRuby);
  
  static NAN_METHOD(RubyStrToJS);
  static NAN_METHOD(RubyBoolToJS);
  static NAN_METHOD(RubyNumToJS);
  
  // Don't instantiate
  Ruby();
  ~Ruby();
};

#endif // NORBY_RUBY_H_
