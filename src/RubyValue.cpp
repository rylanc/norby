#include "RubyValue.h"
#include "SafeMethodCaller.h"

using namespace v8;

VALUE RubyValue::s_wrappedClass;
ID RubyValue::s_wrapperID;
VALUE RubyValue::s_globalHash;
Persistent<Function> RubyValue::s_constructor;

VALUE Block::s_wrapperClass;

void RubyValue::Init()
{
  s_globalHash = rb_hash_new();
  rb_funcall2(s_globalHash, rb_intern("compare_by_identity"), 0, NULL);
  rb_gc_register_address(&s_globalHash);
  
  s_wrappedClass = rb_define_class("WrappedRubyValue", rb_cData);
  s_wrapperID = rb_intern("@_wrappedObject");
  Block::s_wrapperClass = rb_define_class("BlockWrapper", rb_cData);
  
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
