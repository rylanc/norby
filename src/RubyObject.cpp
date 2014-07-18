#include "RubyObject.h"
#include <vector>
#include <string>

using namespace v8;

VALUE trueArg = Qtrue;

ID RubyObject::V8_WRAPPER_ID;
VALUE RubyObject::s_wrappedClass;
VALUE RubyObject::BLOCK_WRAPPER_CLASS;

Persistent<Function> RubyObject::s_constructor;

void RubyObject::Init()
{
  s_wrappedClass = rb_define_class("WrappedRubyObject", rb_cObject);
  V8_WRAPPER_ID = rb_intern("@_wrappedObject");
  BLOCK_WRAPPER_CLASS = rb_define_class("BlockWrapper", rb_cObject);
  
  Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>();
  tpl->SetClassName(NanNew<String>("RubyObject"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  
  tpl->PrototypeTemplate()->Set(NanNew<String>("callMethod"),
    NanNew<FunctionTemplate>(CallMethod)->GetFunction());
  tpl->PrototypeTemplate()->Set(NanNew<String>("callMethodWithBlock"),
    NanNew<FunctionTemplate>(CallMethodWithBlock)->GetFunction());
  tpl->PrototypeTemplate()->Set(NanNew<String>("setOwner"),
    NanNew<FunctionTemplate>(SetOwner)->GetFunction());
  tpl->PrototypeTemplate()->Set(NanNew<String>("getOwner"),
    NanNew<FunctionTemplate>(GetOwner)->GetFunction());
    
  NanAssignPersistent(s_constructor, tpl->GetFunction());
}

Local<Object> RubyObject::New(VALUE rbObj)
{
  NanEscapableScope();
  
  RubyObject* self = new RubyObject(rbObj);
  Local<Object> v8Obj = NanNew<Function>(s_constructor)->NewInstance();
  self->Wrap(v8Obj);
  
  return NanEscapeScope(v8Obj);
}

RubyObject::RubyObject(VALUE obj) :
  m_obj(obj)
{
  rb_gc_register_address(&m_obj);
}

RubyObject::~RubyObject()
{
  rb_gc_unregister_address(&m_obj);
}

// TODO: Should this be a SafeMethodCaller inner class?
struct Block
{
  Block(Handle<Function> f)
  {
    NanAssignPersistent(func, f);
    dataObj = Data_Wrap_Struct(RubyObject::BLOCK_WRAPPER_CLASS, NULL, Free, this);
  }
    
  static VALUE Func(VALUE, VALUE data, int argc, const VALUE* rbArgv)
  {
    Block* block;
    Data_Get_Struct(data, Block, block);
  
    Local<Function> fn = NanNew<Function>(block->func);
    // TODO: Should we store args.This() and call it as the receiver?
    std::vector<Handle<Value> > v8Args(argc);
    for (int i = 0; i < argc; i++) {
      v8Args[i] = RubyObject::New(rbArgv[i]);
    }
    
    Handle<Value> res = NanMakeCallback(NanGetCurrentContext()->Global(), fn,
                                        argc, &v8Args[0]);
    assert(res->IsObject());                                    
    return *node::ObjectWrap::Unwrap<RubyObject>(res.As<Object>());
  }
    
  static void Free(void* data)
  {
    Block* block = static_cast<Block*>(data);
    NanDisposePersistent(block->func);
    delete block;
  }
    
  Persistent<Function> func;
  VALUE dataObj;
};

struct SafeMethodCaller
{
  SafeMethodCaller(_NAN_METHOD_ARGS_TYPE args) :
    rubyArgs(args.Length()-1), ex(Qnil), block(NULL)
  {
    FillArgs(args);
  }
  
  SafeMethodCaller(_NAN_METHOD_ARGS_TYPE args, Local<Function> blockFunc) :
    rubyArgs(args.Length()-2), ex(Qnil), block(new Block(blockFunc))
  {
    FillArgs(args);
  }
  
  void FillArgs(_NAN_METHOD_ARGS_TYPE args)
  {
    recv = *node::ObjectWrap::Unwrap<RubyObject>(args.This());
    
    assert(args[0]->IsObject());
    methodID =
      SYM2ID(*node::ObjectWrap::Unwrap<RubyObject>(args[0].As<Object>()));
  
    for (size_t i = 0; i < rubyArgs.size(); i++) {
      assert(args[i+1]->IsObject());
      rubyArgs[i] =
        *node::ObjectWrap::Unwrap<RubyObject>(args[i+1].As<Object>());
    }
  }
  
  Handle<Value> Call()
  {
    // TODO: HandleScope?
    VALUE res = rb_rescue2(RUBY_METHOD_FUNC(SafeCB), VALUE(this),
                           RUBY_METHOD_FUNC(RescueCB), VALUE(&ex), rb_eException, NULL);
    if (ex != Qnil) {
      Local<Object> errObj = NanNew<Object>();
      errObj->Set(NanNew<String>("error"), RubyObject::New(ex));
      return errObj;
    }
    
    return RubyObject::New(res);
  }
  
  static VALUE SafeCB(VALUE data)
  {
    const SafeMethodCaller* self = reinterpret_cast<SafeMethodCaller*>(data);
    
    if (self->block == NULL) {
      return rb_funcall2(self->recv, self->methodID, self->rubyArgs.size(),
                         (VALUE*)&self->rubyArgs[0]);
    }
    else {
      // TODO: Probably not available in Ruby < 1.9
      return rb_block_call(self->recv, self->methodID, self->rubyArgs.size(),
                           (VALUE*)&self->rubyArgs[0],
                           RUBY_METHOD_FUNC(Block::Func), self->block->dataObj);
    }
  }
  
  static VALUE RescueCB(VALUE data, VALUE ex)
  {
    VALUE *storedEx = reinterpret_cast<VALUE*>(data);
    *storedEx = ex;

    return Qnil;
  }

  VALUE recv;
  VALUE methodID;
  std::vector<VALUE> rubyArgs;
  VALUE ex;
  Block* block;
};

NAN_METHOD(RubyObject::CallMethod)
{
  NanScope();

  SafeMethodCaller caller(args);
  NanReturnValue(caller.Call());
}

// TODO: Does doing it this way really make it safer / less C++ code?
NAN_METHOD(RubyObject::CallMethodWithBlock)
{
  NanScope();
    
  assert(args[args.Length()-1]->IsFunction());
  Local<Function> blockFunc = args[args.Length()-1].As<Function>();
  
  SafeMethodCaller caller(args, blockFunc);
  NanReturnValue(caller.Call());
}

// TODO: Should this be a member?
NAN_WEAK_CALLBACK(OwnerWeakCB)
{
  RubyObject* self = data.GetParameter();
  rb_ivar_set(*self, RubyObject::V8_WRAPPER_ID, Qnil);
}

NAN_METHOD(RubyObject::SetOwner)
{
  NanScope();
  
  RubyObject *self = node::ObjectWrap::Unwrap<RubyObject>(args.This());
  assert(args[0]->IsObject());
  
  if (rb_obj_frozen_p(self->m_obj) == Qfalse) {
    VALUE wrappedObj = Data_Wrap_Struct(s_wrappedClass, NULL, NULL, self);
    rb_ivar_set(self->m_obj, V8_WRAPPER_ID, wrappedObj);
  
    self->m_owner = &NanMakeWeakPersistent(args[0].As<Object>(), self, OwnerWeakCB)->persistent;
    self->m_owner->MarkIndependent();
  }
  
  NanReturnUndefined();
}

NAN_METHOD(RubyObject::GetOwner)
{
  NanScope();
  
  RubyObject *self = node::ObjectWrap::Unwrap<RubyObject>(args.This());
  VALUE wrappedObj = rb_attr_get(self->m_obj, V8_WRAPPER_ID);
  if (wrappedObj == Qnil)
    NanReturnUndefined();
  else {
    RubyObject* obj;
    Data_Get_Struct(wrappedObj, RubyObject, obj);
    NanReturnValue(NanNew<v8::Object>(*obj->m_owner));
  }
}
