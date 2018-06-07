#include <stdio.h>
#include <iostream>
#include <sstream>

using namespace std;

// NODE related headers
#define NODE_WANT_INTERNALS 1

#include <env.h>
#include <node.h>
#include <uv.h>
#include "libplatform/libplatform.h"
#include "node_internals.h"
#include "v8.h"

using ::v8::Array;
using ::v8::Context;
using ::v8::Local;
using ::v8::Value;

#include "native/bindings/ipc.h"

#include "include/cef_app.h"

// helpers:
#ifdef _WIN32
const std::string PATH_SEPARATOR = "\\";

std::string toString(wchar_t* stri) {
  std::wstring ws(stri);
  std::string str(ws.begin(), ws.end());
  return str;
}

const wchar_t* toWChar(std::string str) {
  std::wstring widestr = std::wstring(str.begin(), str.end());
  const wchar_t* widecstr = widestr.c_str();
  return widecstr;
}

std::string getexepath() {
  wchar_t result[MAX_PATH];
  GetModuleFileName(NULL, result, MAX_PATH);
  return toString(result);
}

std::string getbasedirpath() {
  std::string exe_path = getexepath();
  std::string exe_dir =
      exe_path.substr(0, exe_path.find_last_of(PATH_SEPARATOR));
  return exe_dir + "/../../..";  // bin/Debug/tau.exe || bin/Release/tau.exe
}

void log(std::string str) {
  std::stringstream ss;
  ss << "init path: " << str << "\n";
  OutputDebugStringA(ss.str().c_str());
}
#elif APPLE
#include <mach-o/dyld.h>
const std::string PATH_SEPARATOR = "/";
std::string getexepath() {
  char path[1024];
  uint32_t size = sizeof(path);
  int count = _NSGetExecutablePath(path, &size);
  path[size] = '\0';
  std::string res = std::string(path, size);
  return res;
}
std::string getbasedirpath() {
  std::string exe_path = getexepath();
  std::string exe_dir =
      exe_path.substr(0, exe_path.find_last_of(PATH_SEPARATOR));
  // tau/build/src/Debug/tau.app/Contents/MacOS/../../../../../../src/lib/browser/init.js
  return exe_dir + "/../../../../../..";
}
#endif

char* getCString(std::string str) {
  char* cstr = new char[str.length() + 1];
  strcpy(cstr, str.c_str());  // copy to get "un-const"
  return cstr;
}

using namespace v8;
using namespace node;

void CustomMethod(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();

  double value = args[0]->NumberValue() + args[1]->NumberValue();
  Local<Number> num = Number::New(isolate, value);

  // Set the return value (using the passed in
  // FunctionCallbackInfo<Value>&)
  args.GetReturnValue().Set(num);
}

int node_start() {
  char arg[] = "";

  // argv[1] path is treated relative to cwd which can change so we provide an
  // absolute path  generate relative to binary: tau/bin/Debug|Release/tau[.exe]
  std::string base_path = getbasedirpath();

  printf("exe dir is %s \n", base_path.c_str());
  std::string init_path = base_path + "/src/lib/browser/init.js";

  char* argv[] = {arg, getCString(init_path)};
  int argc = 2;
  // node::Start(argc, argv); -> works but not enough control

  // This needs to run *before* V8::Initialize().  The const_cast is not
  // optional, in case you're wondering.
  int exec_argc;
  const char** exec_argv;

  // modules need to be registered before node::init which is where node
  // registers modules.. ?
  // Explicitly register own builtin modules.
  // RegisterBuiltinCustomModules();
  // registerModules();

  // This needs to run *before* V8::Initialize().
  node::Init(&argc, const_cast<const char**>(argv), &exec_argc, &exec_argv);

  // v8::Platform* platform = v8::platform::CreateDefaultPlatform();
  // V8::InitializePlatform(platform);

  // std::unique_ptr<v8::Platform> platform =
  // v8::platform::NewDefaultPlatform(); V8::InitializePlatform(platform.get());
  node::MultiIsolatePlatform* platform =
      node::GetMainThreadMultiIsolatePlatform();
  // V8::InitializePlatform(platform);

  // MultiIsolatePlatform* platform = CreatePlatform(4, {});
  // V8::InitializePlatform(platform);

  /*
  tracing_agent_.reset(new tracing::Agent(trace_file_pattern));
  NodePlatform* platform = new NodePlatform(v8_thread_pool_size,
  tracing_agent_->GetTracingController()); V8::InitializePlatform(platform);
  */

  // Enable tracing when argv has --trace-events-enabled.
  // v8_platform.StartTracingAgent();
  V8::Initialize();
  // performance::performance_v8_start = PERFORMANCE_NOW();
  // v8_initialized = true;

  // const int exit_code = Start(uv_default_loop(), argc, argv, exec_argc,
  // exec_argv);
  // ==
  auto alloc = node::CreateArrayBufferAllocator();
  std::unique_ptr<ArrayBufferAllocator, decltype(&FreeArrayBufferAllocator)>
      allocator(alloc, &FreeArrayBufferAllocator);
  Isolate* const isolate = NewIsolate(allocator.get());

  if (isolate == nullptr)
    return 12;  // Signal internal error.

  {
    v8::Isolate::Scope isolate_scope(isolate);
    // Create a stack-allocated handle scope.
    v8::HandleScope handle_scope(isolate);

    // Create a new context.
    v8::Local<v8::Context> context = v8::Context::New(isolate);
    // Enter the context for compiling and running the hello world script.
    v8::Context::Scope context_scope(context);

    uv_loop_t* loop = uv_default_loop();
    IsolateData* isolate_data = CreateIsolateData(isolate, loop);

    Environment* env = node::CreateEnvironment(isolate_data, context, argc,
                                               argv, exec_argc, exec_argv);
    // auto env = new Environment(isolate_data, context,
    // platform->GetTracingController());  env->Start(argc, argv, exec_argc,
    // exec_argv, false);

    // customize env here:

    Local<Object> global(context->Global());

    global->Set(v8::String::NewFromUtf8(isolate, "globalConstantTest"),
                String::NewFromUtf8(isolate, "I was set in C++"));

    NODE_SET_METHOD(global, "customMethod", CustomMethod);

	RegisterBuiltinCustomModules();

    //..................

    Local<Object> process(env->process_object());
    // FIXME process is nullptr
    // process->Set(v8::String::NewFromUtf8(isolate, "resourcesPath"),
    // String::NewFromUtf8(isolate, "resources_path_here"));

    LoadEnvironment(env);

    bool more;
    do {
      CefDoMessageLoopWork();

      more = uv_run(loop, UV_RUN_ONCE);

      // platform->DrainBackgroundTasks(isolate); //FIXME probably required /
      // very important but fails assertion

      if (more == false) {
        more = true;
      }

      // platform->DrainBackgroundTasks(isolate); FIXME probably required but
      // fails assertion
      if (more == false) {
        node::EmitBeforeExit(env);

        // Emit `beforeExit` if the loop became alive either after emitting
        // event, or after running some callbacks.
        more = uv_loop_alive(loop);
        if (uv_run(loop, UV_RUN_NOWAIT) != 0)
          more = true;
      }
    } while (more == true);
  }

  // Dispose the isolate and tear down V8.
  isolate->Dispose();

  // v8_platform.StopTracingAgent();
  // v8_initialized = false;
  V8::Dispose();
  v8::V8::ShutdownPlatform();

  return 0;
}
