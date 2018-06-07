#pragma once
#ifndef MYOBJECT_H
#define MYOBJECT_H

#include "node.h"
#include <node_object_wrap.h>
#include "v8.h"

namespace api {
	class BrowserWindow : public node::ObjectWrap
	{

	public:
		static void Init(v8::Local<v8::Object> exports);

	private:
		//BrowserWindow(v8::Isolate* isolate, v8::Local<v8::Object> wrapper/*, const mate::Dictionary& options/);
		explicit BrowserWindow(std::string url, std::string title = "no title", int width = 800, int height = 600);
		~BrowserWindow();

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void LoadFile(const v8::FunctionCallbackInfo<v8::Value>& args);
		static v8::Persistent<v8::Function> constructor;
		double value_;

		//void Init(v8::Isolate* isolate, v8::Local<v8::Object> wrapper /*, const mate::Dictionary& options/);

	};
} //namespace api

#endif

