#ifndef NORBY_SAFE_METHOD_CALLER_H_
#define NORBY_SAFE_METHOD_CALLER_H_

#include "RubyValue.h"
#include <vector>

struct Block
{
  Block(v8::Handle<v8::Function> f)
  {
    NanAssignPersistent(func, f);
    dataObj = Data_Wrap_Struct(s_wrapperClass, NULL, Free, this);
  }
    
  static VALUE Func(VALUE, VALUE data, int argc, const VALUE* rbArgv)
  {
    Block* block;
    Data_Get_Struct(data, Block, block);
  
    v8::Local<v8::Function> fn = NanNew<v8::Function>(block->func);
    // TODO: Should we store args.This() and call it as the receiver?
    std::vector<v8::Handle<v8::Value> > v8Args(argc);
    for (int i = 0; i < argc; i++) {
      v8Args[i] = RubyValue::New(rbArgv[i]);
    }
    
    // Since we can (for the most part) assume that there will always be an
    // existing v8 stack at this point, we don't have to call
    // node::MakeCallback. This allows us to propagate any exceptions up the v8
    // call stack, instead of just exiting the process. This will cause issues
    // if we're somehow called directly from native code (e.g. directly from
    // the libuv event loop)
    v8::Handle<v8::Value> res =
      fn->Call(NanGetCurrentContext()->Global(), argc, &v8Args[0]);
      
    if (res.IsEmpty())
      return Qnil;
      
    assert(res->IsObject());
    return RubyValue::Unwrap(res.As<v8::Object>());
  }
    
  static void Free(void* data)
  {
    Block* block = static_cast<Block*>(data);
    NanDisposePersistent(block->func);
    delete block;
  }
  
  static VALUE s_wrapperClass;
    
  v8::Persistent<v8::Function> func;
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
  
  SafeMethodCaller(_NAN_METHOD_ARGS_TYPE args,
                   v8::Local<v8::Function> blockFunc) :
    rubyArgs(args.Length()-2), ex(Qnil), block(new Block(blockFunc))
  {
    FillArgs(args);
  }
  
  inline
  void FillArgs(_NAN_METHOD_ARGS_TYPE args)
  {
    recv = RubyValue::Unwrap(args.This());
    
    assert(args[0]->IsObject());
    methodID = SYM2ID(RubyValue::Unwrap(args[0].As<v8::Object>()));
  
    for (size_t i = 0; i < rubyArgs.size(); i++) {
      assert(args[i+1]->IsObject());
      rubyArgs[i] = RubyValue::Unwrap(args[i+1].As<v8::Object>());
    }
  }
  
  inline
  v8::Handle<v8::Value> Call()
  {
    VALUE res = rb_rescue2(RUBY_METHOD_FUNC(SafeCB), VALUE(this),
                           RUBY_METHOD_FUNC(RescueCB), VALUE(&ex),
                           rb_eException, NULL);
    if (ex != Qnil) {
      v8::Local<v8::Object> errObj = NanNew<v8::Object>();
      errObj->Set(NanNew<v8::String>("error"), RubyValue::New(ex));
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

#endif // NORBY_SAFE_METHOD_CALLER_H_
