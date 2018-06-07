#include "ipc.h"
#include "browser_window.h"
#include <node.h>
#include <v8.h>
#include <node_internals.h>

namespace api {

	using v8::FunctionCallbackInfo;
	using v8::Isolate;
	using v8::Local;
	using v8::Object;
	using v8::String;
	using v8::Value;

	void Method(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, "world #######"));
	}

	void Initialize(Local<Object> exports, v8::Local<v8::Value> unused, v8::Local<v8::Context> context, void* priv) {
		NODE_SET_METHOD(exports, "hello", Method);
	}

}

NODE_MODULE_CONTEXT_AWARE(ipc, api::Initialize)



