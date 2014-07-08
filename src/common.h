#ifndef NORBY_COMMON_H_
#define NORBY_COMMON_H_

#include <nan.h>
#include <v8.h>
#include <ruby.h>

#ifdef _DEBUG
#include <iostream>
#define log(x) std::cout << x << std::endl
#else
#define log(x)
#endif

v8::Handle<v8::Value> rubyToV8(VALUE val);
VALUE v8ToRuby(v8::Handle<v8::Value> val);
v8::Handle<v8::Value> rubyExToV8(VALUE ex);

VALUE RescueCB(VALUE data, VALUE ex);

template<class F>
struct SafeCallWrapper
{
  static VALUE Func(VALUE data)
  {
    const F* f = reinterpret_cast<F*>(data);

    return (*f)();
  }
};

template<class F>
inline VALUE SafeRubyCall(const F& f, VALUE &ex)
{
  ex = Qnil;
  return rb_rescue2(RUBY_METHOD_FUNC(SafeCallWrapper<F>::Func), VALUE(&f),
                    RUBY_METHOD_FUNC(RescueCB), VALUE(&ex), rb_eException, NULL);
}

#define SAFE_RUBY_CALL(x, y) \
{ \
  VALUE ex; \
  x = SafeRubyCall(y, ex); \
  if (ex != Qnil) { \
    NanThrowError(rubyExToV8(ex)); \
    NanReturnUndefined(); \
  } \
}

VALUE CallV8FromRuby(const v8::Handle<v8::Object> recv,
                     const v8::Handle<v8::Function> callback,
                     int argc, const VALUE* argv);
v8::Handle<v8::Value> CallRubyFromV8(VALUE recv, _NAN_METHOD_ARGS_TYPE args);

#if (NODE_MODULE_VERSION > 0x000B)
#define EXTERNAL_WRAP(x) v8::External::New(v8::Isolate::GetCurrent(), x)
#define EXTERNAL_NEW EXTERNAL_WRAP
#define EXTERNAL_UNWRAP(x) x.As<External>()->Value()
#else
#define EXTERNAL_WRAP(x) v8::External::Wrap(x)
#define EXTERNAL_NEW(x) v8::External::New(x)
#define EXTERNAL_UNWRAP(x) v8::External::Unwrap(x)
#endif

// For debugging
void DumpRubyArgs(int argc, VALUE* argv);
void DumpV8Props(v8::Handle<v8::Object> obj);
void DumpV8Args(_NAN_METHOD_ARGS_TYPE args);

#endif // NORBY_COMMON_H_
