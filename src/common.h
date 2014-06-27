#include <v8.h>
#include <ruby.h>

#ifdef _DEBUG
#define log(x) cout << x
#else
#define log(x)
#endif

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

VALUE CallV8FromRuby(const v8::Handle<v8::Object> recv,
                     const v8::Handle<v8::Function> callback,
                     int argc, const VALUE* argv);

// For debugging
void DumpRubyArgs(int argc, VALUE* argv);
void DumpV8Props(v8::Handle<v8::Object> obj);
void DumpV8Args(const v8::Arguments& args);
