#include <node.h>
#include <nan.h>
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

  static NAN_METHOD(New);
  static NAN_METHOD(CallMethod);

  VALUE m_obj;
  typedef std::map<ID, v8::Persistent<v8::FunctionTemplate> > TplMap;
  //typedef std::map<ID, v8::CopyablePersistentTraits<v8::FunctionTemplate>::CopyablePersistent> TplMap;
  static TplMap s_functionTemplates;
};
