#include "browser_window.h"

#include <stdio.h>
#include <node.h>
#include <v8.h>
#include <node_internals.h>
#include <node_object_wrap.h>

#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/views/cef_display.h"
#include "include/wrapper/cef_helpers.h"

namespace api {

	using v8::Context;
	using v8::Function;
	using v8::FunctionCallbackInfo;
	using v8::FunctionTemplate;
	using v8::Isolate;
	using v8::Local;
	using v8::Number;
	using v8::Object;
	using v8::Persistent;
	using v8::String;
	using v8::Value;

	//TODO outsource helpers or use lib instead 
	std::string toStdString(v8::Local<v8::String> str) {
		v8::String::Utf8Value param1(str->ToString());
		return std::string(*param1);
	}

	std::string getString(const FunctionCallbackInfo<Value>& args, int index) {
		Isolate* isolate = args.GetIsolate();
		v8::MaybeLocal<v8::String> str= args[index]->ToString(isolate->GetCurrentContext());
		return toStdString(str.ToLocalChecked());
	}

	double getDouble(const FunctionCallbackInfo<Value>& args, int index) {
		Isolate* isolate = args.GetIsolate();
		v8::Maybe<double> mWidth = args[index]->NumberValue(isolate->GetCurrentContext());
		return mWidth.ToChecked();
	}

	int getInt(const FunctionCallbackInfo<Value>& args, int index) {
		Isolate* isolate = args.GetIsolate();
		v8::Maybe<double> mWidth = args[index]->NumberValue(isolate->GetCurrentContext());
		return (int)(mWidth.ToChecked()); // TODO use round?
	}
	//end helpers

	Persistent<Function> BrowserWindow::constructor;

	BrowserWindow::BrowserWindow(std::string url, std::string title, int width, int height) {
		// SimpleHandler implements browser-level callbacks.
		//CefRefPtr<SimpleHandler> handler(new SimpleHandler(use_views));

		// Specify CEF browser settings here.
		CefBrowserSettings browser_settings;

		// Information used when creating the native window.
		CefWindowInfo window_info;

#if defined(OS_WIN)
		// On Windows we need to specify certain flags that will be passed to
		// CreateWindowEx().
		window_info.SetAsPopup(NULL, title);
#endif

		//must be set after setaspopup
		window_info.width = width;
		window_info.height = height;

		// Create the first browser window.
		// CefBrowserHost::CreateBrowser(window_info, handler, url, browser_settings, NULL);
		CefBrowserHost::CreateBrowser(window_info, NULL, url, browser_settings, NULL);

	}

	BrowserWindow::~BrowserWindow() {
	}

	void BrowserWindow::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();

		// Prepare constructor template
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
		tpl->SetClassName(String::NewFromUtf8(isolate, "BrowserWindow"));
		tpl->InstanceTemplate()->SetInternalFieldCount(1);

		// Prototype
		NODE_SET_PROTOTYPE_METHOD(tpl, "loadFile", LoadFile);

		constructor.Reset(isolate, tpl->GetFunction());
		exports->Set(String::NewFromUtf8(isolate, "BrowserWindow"), tpl->GetFunction());
	}

	void BrowserWindow::New(const FunctionCallbackInfo<Value>& args) {
		// Isolate* isolate = args.GetIsolate();

		printf("browser window constructor called %s \n", args.IsConstructCall() ? "true" : "false");

		if (args.IsConstructCall()) {

			//TODO handle invalid args
			std::string url = getString(args, 0);
			std::string title = getString(args, 1);
			double width = getInt(args, 2);
			double height = getInt(args, 3);

			BrowserWindow* obj = new BrowserWindow(url, title, width, height);
			obj->Wrap(args.This());
			args.GetReturnValue().Set(args.This());
		}
	}

	void BrowserWindow::LoadFile(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();

		printf("load file called \n");

		BrowserWindow* obj = ObjectWrap::Unwrap<BrowserWindow>(args.Holder());
		obj->value_ += 1;

		args.GetReturnValue().Set(Number::New(isolate, obj->value_));
	}
/*
	BrowserWindow::BrowserWindow(v8::Isolate* isolate, v8::Local<v8::Object> wrapper){
		Init(isolate, wrapper);
	}

	BrowserWindow::~BrowserWindow() {
		//api_web_contents_->DestroyWebContents(true / async /);
	}

	void BrowserWindow::Init(v8::Isolate* isolate, v8::Local<v8::Object> wrapper) {
	}


	void InitAll(Local<Object> exports) {
		//MyObject::Init(exports);
	}
*/
}
NODE_MODULE_CONTEXT_AWARE(BrowserWindow, api::BrowserWindow::Init)


#define ELECTRON_BUILTIN_MODULES(V)          \
  V(BrowserWindow) \
  //V(ipc)

// This is used to load built-in modules. Instead of using
// __attribute__((constructor)), we call the _register_<modname>
// function for each built-in modules explicitly. This is only
// forward declaration. The definitions are in each module's
// implementation when calling the NODE_BUILTIN_MODULE_CONTEXT_AWARE.
#define V(modname) void _register_##modname();
ELECTRON_BUILTIN_MODULES(V)
#undef V

void RegisterBuiltinCustomModules() {
#define V(modname) _register_##modname();
	ELECTRON_BUILTIN_MODULES(V)
#undef V
}
