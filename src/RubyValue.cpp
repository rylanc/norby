#include "RubyValue.h"
#include <vector>

using namespace v8;

VALUE RubyValue::s_wrappedClass;
ID RubyValue::s_wrapperID;
VALUE RubyValue::s_blockWrapperClass;
VALUE RubyValue::s_globalHash;
Persistent<Function> RubyValue::s_constructor;

void RubyValue::Init()
{
  s_globalHash = rb_hash_new();
  rb_funcall2(s_globalHash, rb_intern("compare_by_identity"), 0, NULL);
  rb_gc_register_address(&s_globalHash);
  
  s_wrappedClass = rb_define_class("WrappedRubyValue", rb_cData);
  s_wrapperID = rb_intern("@_wrappedObject");
  s_blockWrapperClass = rb_define_class("BlockWrapper", rb_cData);
  
  Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>();
  tpl->SetClassName(NanNew<String>("RubyValue"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  
  NODE_SET_PROTOTYPE_METHOD(tpl, "callMethod", CallMethod);
  NODE_SET_PROTOTYPE_METHOD(tpl, "callMethodWithBlock", CallMethodWithBlock);
  NODE_SET_PROTOTYPE_METHOD(tpl, "setOwner", SetOwner);
  NODE_SET_PROTOTYPE_METHOD(tpl, "getOwner", GetOwner);
  NODE_SET_PROTOTYPE_METHOD(tpl, "getType", GetType);
    
  NanAssignPersistent(s_constructor, tpl->GetFunction());
}

Local<Object> RubyValue::New(VALUE rbObj)
{
  NanEscapableScope();
  
  RubyValue* self = new RubyValue(rbObj);
  Local<Object> v8Obj = NanNew<Function>(s_constructor)->NewInstance();
  self->Wrap(v8Obj);
  
  return NanEscapeScope(v8Obj);
}

inline
RubyValue::RubyValue(VALUE obj) :
  m_obj(obj), m_owner(NULL)
{
  if (!SPECIAL_CONST_P(m_obj)) {
    long count = FIX2LONG(rb_hash_lookup2(s_globalHash, m_obj, LONG2FIX(0)));
    rb_hash_aset(s_globalHash, m_obj, LONG2FIX(++count));
  }
}

RubyValue::~RubyValue()
{
  if (!SPECIAL_CONST_P(m_obj)) {
    long count = FIX2LONG(rb_hash_lookup2(s_globalHash, m_obj, LONG2FIX(0)));
    assert(count > 0);
    if (count > 1)
      rb_hash_aset(s_globalHash, m_obj, LONG2FIX(--count));
    else
      rb_hash_delete(s_globalHash, m_obj);
  }
}

// TODO: Should this be a SafeMethodCaller inner class?
struct Block
{
  Block(Handle<Function> f)
  {
    NanAssignPersistent(func, f);
    dataObj = Data_Wrap_Struct(RubyValue::s_blockWrapperClass,
                               NULL, Free, this);
  }
    
  static VALUE Func(VALUE, VALUE data, int argc, const VALUE* rbArgv)
  {
    Block* block;
    Data_Get_Struct(data, Block, block);
  
    Local<Function> fn = NanNew<Function>(block->func);
    // TODO: Should we store args.This() and call it as the receiver?
    std::vector<Handle<Value> > v8Args(argc);
    for (int i = 0; i < argc; i++) {
      v8Args[i] = RubyValue::New(rbArgv[i]);
    }
    
    Handle<Value> res = NanMakeCallback(NanGetCurrentContext()->Global(), fn,
                                        argc, &v8Args[0]);
    assert(res->IsObject());                                    
    return RubyValue::Unwrap(res.As<Object>());
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
  inline
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
  
  inline
  void FillArgs(_NAN_METHOD_ARGS_TYPE args)
  {
    recv = RubyValue::Unwrap(args.This());
    
    assert(args[0]->IsObject());
    methodID = SYM2ID(RubyValue::Unwrap(args[0].As<Object>()));
  
    for (size_t i = 0; i < rubyArgs.size(); i++) {
      assert(args[i+1]->IsObject());
      rubyArgs[i] = RubyValue::Unwrap(args[i+1].As<Object>());
    }
  }
  
  inline
  Handle<Value> Call()
  {
    // TODO: HandleScope?
    VALUE res = rb_rescue2(RUBY_METHOD_FUNC(SafeCB), VALUE(this),
                           RUBY_METHOD_FUNC(RescueCB), VALUE(&ex),
                           rb_eException, NULL);
    if (ex != Qnil) {
      Local<Object> errObj = NanNew<Object>();
      errObj->Set(NanNew<String>("error"), RubyValue::New(ex));
      return errObj;
    }
      
    return RubyValue::New(res);
  }
  
  inline
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

NAN_METHOD(RubyValue::CallMethod)
{
  NanScope();

  SafeMethodCaller caller(args);
  NanReturnValue(caller.Call());
}

// TODO: Does doing it this way really make it safer / less C++ code?
NAN_METHOD(RubyValue::CallMethodWithBlock)
{
  NanScope();
    
  assert(args[args.Length()-1]->IsFunction());
  Local<Function> blockFunc = args[args.Length()-1].As<Function>();
  
  SafeMethodCaller caller(args, blockFunc);
  NanReturnValue(caller.Call());
}

template<typename T, typename P>
void RubyValue::OwnerWeakCB(const _NanWeakCallbackData<T, P>& data)
{
  RubyValue* self = data.GetParameter();
  rb_ivar_set(self->m_obj, s_wrapperID, Qnil);
}

NAN_METHOD(RubyValue::SetOwner)
{
  NanScope();
  
  RubyValue *self = node::ObjectWrap::Unwrap<RubyValue>(args.This());
  assert(args[0]->IsObject());
  
  if (rb_obj_frozen_p(self->m_obj) == Qfalse) {
    VALUE wrappedObj = Data_Wrap_Struct(s_wrappedClass, NULL, NULL, self);
    rb_ivar_set(self->m_obj, s_wrapperID, wrappedObj);
  
    self->m_owner = &NanMakeWeakPersistent(args[0].As<Object>(), self,
                                           OwnerWeakCB)->persistent;
    self->m_owner->MarkIndependent();
  }
  
  NanReturnUndefined();
}

// TODO: Can/should we move any of this (and Set) to JS?
NAN_METHOD(RubyValue::GetOwner)
{
  NanScope();
  
  RubyValue *self = node::ObjectWrap::Unwrap<RubyValue>(args.This());
  VALUE wrappedObj = rb_attr_get(self->m_obj, s_wrapperID);
  if (wrappedObj == Qnil)
    NanReturnUndefined();
  else {
    RubyValue* obj;
    Data_Get_Struct(wrappedObj, RubyValue, obj);
    NanReturnValue(NanNew<v8::Object>(*obj->m_owner));
  }
}

NAN_METHOD(RubyValue::GetType)
{
  NanScope();
  NanReturnValue(NanNew<Integer>(TYPE(RubyValue::Unwrap(args.This()))));
}
