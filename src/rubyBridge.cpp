#include <node.h>
#include "Ruby.h"

using namespace v8;

void Init(Handle<Object> exports, Handle<Object> module) {
  Ruby::Init(module);
}

NODE_MODULE(norby, Init)
