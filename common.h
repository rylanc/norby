#include <v8.h>
#include <ruby.h>

v8::Handle<v8::Value> rubyToV8(VALUE val);
VALUE v8ToRuby(v8::Handle<v8::Value> val);
v8::Handle<v8::Value> rubyExToV8(VALUE ex);

VALUE RescueCB(VALUE data, VALUE ex);

template<class T>
struct SafeCallWrapper
{
  static VALUE Func(VALUE data)
  {
    const T* t = reinterpret_cast<T*>(data);

    return (*t)();
  }
};

template<class F>
inline VALUE SafeRubyCall(const F& f, VALUE &ex)
{
  ex = Qnil;
  return rb_rescue2(RUBY_METHOD_FUNC(SafeCallWrapper<F>::Func), VALUE(&f),
                    RUBY_METHOD_FUNC(RescueCB), VALUE(&ex), rb_eException, NULL);
}
