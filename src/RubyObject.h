#include <node.h>
#include <v8.h>
#include <ruby.h>
#include <vector>

class RubyObject : public node::ObjectWrap
{
 public:
  static v8::Local<v8::Value> New(VALUE klass, int argc, VALUE* argv);

 private:
  RubyObject(VALUE obj);
  ~RubyObject();

  static v8::Handle<v8::Value> CallMethod(const v8::Arguments& args);

  VALUE m_obj;
  std::vector<ID> m_instanceMethods;
};
