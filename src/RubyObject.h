#include <node.h>
#include <v8.h>
#include <ruby.h>
#include <map>

class RubyObject : public node::ObjectWrap
{
 public:
  static const char* RUBY_OBJECT_TAG;
 
  static v8::Local<v8::Function> GetClass(VALUE klass);
  VALUE GetObject() { return m_obj; }

 private:
  RubyObject(VALUE obj);
  ~RubyObject();

  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Handle<v8::Value> CallMethod(const v8::Arguments& args);

  VALUE m_obj;
  typedef std::map<ID, v8::Persistent<v8::FunctionTemplate> > TplMap;
  static TplMap s_functionTemplates;
};
