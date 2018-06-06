// Copyright (c) 2013 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>

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

#undef CHECK
#undef CHECK_EQ
#undef CHECK_NE
#undef CHECK_LE
#undef CHECK_LT
#undef CHECK_GE
#undef CHECK_GT

#include "include/cef_application_mac.h"
#include "include/wrapper/cef_helpers.h"
#include "tests/cefsimple/simple_app.h"
#include "tests/cefsimple/simple_handler.h"

// Receives notifications from the application.
@interface SimpleAppDelegate : NSObject<NSApplicationDelegate>
- (void)createApplication:(id)object;
- (void)tryToTerminateApplication:(NSApplication*)app;
@end

// Provide the CefAppProtocol implementation required by CEF.
@interface SimpleApplication : NSApplication<CefAppProtocol> {
 @private
  BOOL handlingSendEvent_;
}
@end

@implementation SimpleApplication
- (BOOL)isHandlingSendEvent {
  return handlingSendEvent_;
}

- (void)setHandlingSendEvent:(BOOL)handlingSendEvent {
  handlingSendEvent_ = handlingSendEvent;
}

- (void)sendEvent:(NSEvent*)event {
  CefScopedSendingEvent sendingEventScoper;
  [super sendEvent:event];
}

// |-terminate:| is the entry point for orderly "quit" operations in Cocoa. This
// includes the application menu's quit menu item and keyboard equivalent, the
// application's dock icon menu's quit menu item, "quit" (not "force quit") in
// the Activity Monitor, and quits triggered by user logout and system restart
// and shutdown.
//
// The default |-terminate:| implementation ends the process by calling exit(),
// and thus never leaves the main run loop. This is unsuitable for Chromium
// since Chromium depends on leaving the main run loop to perform an orderly
// shutdown. We support the normal |-terminate:| interface by overriding the
// default implementation. Our implementation, which is very specific to the
// needs of Chromium, works by asking the application delegate to terminate
// using its |-tryToTerminateApplication:| method.
//
// |-tryToTerminateApplication:| differs from the standard
// |-applicationShouldTerminate:| in that no special event loop is run in the
// case that immediate termination is not possible (e.g., if dialog boxes
// allowing the user to cancel have to be shown). Instead, this method tries to
// close all browsers by calling CloseBrowser(false) via
// ClientHandler::CloseAllBrowsers. Calling CloseBrowser will result in a call
// to ClientHandler::DoClose and execution of |-performClose:| on the NSWindow.
// DoClose sets a flag that is used to differentiate between new close events
// (e.g., user clicked the window close button) and in-progress close events
// (e.g., user approved the close window dialog). The NSWindowDelegate
// |-windowShouldClose:| method checks this flag and either calls
// CloseBrowser(false) in the case of a new close event or destructs the
// NSWindow in the case of an in-progress close event.
// ClientHandler::OnBeforeClose will be called after the CEF NSView hosted in
// the NSWindow is dealloc'ed.
//
// After the final browser window has closed ClientHandler::OnBeforeClose will
// begin actual tear-down of the application by calling CefQuitMessageLoop.
// This ends the NSApplication event loop and execution then returns to the
// main() function for cleanup before application termination.
//
// The standard |-applicationShouldTerminate:| is not supported, and code paths
// leading to it must be redirected.
- (void)terminate:(id)sender {
  SimpleAppDelegate* delegate =
      static_cast<SimpleAppDelegate*>([NSApp delegate]);
  [delegate tryToTerminateApplication:self];
  // Return, don't exit. The application is responsible for exiting on its own.
}
@end

@implementation SimpleAppDelegate

// Create the application on the UI thread.
- (void)createApplication:(id)object {
  [NSApplication sharedApplication];
  [[NSBundle mainBundle] loadNibNamed:@"MainMenu"
                                owner:NSApp
                      topLevelObjects:nil];

  // Set the delegate for application events.
  [[NSApplication sharedApplication] setDelegate:self];
}

- (void)tryToTerminateApplication:(NSApplication*)app {
  SimpleHandler* handler = SimpleHandler::GetInstance();
  if (handler && !handler->IsClosing())
    handler->CloseAllBrowsers(false);
}

- (NSApplicationTerminateReply)applicationShouldTerminate:
    (NSApplication*)sender {
  return NSTerminateNow;
}
@end


namespace {
    
    using namespace v8;
    using namespace node;

    #include <mach-o/dyld.h>
    const std::string PATH_SEPARATOR = "/";
    std::string getexepath()
    {
        char path[1024];
        uint32_t size = sizeof(path);
        int count = _NSGetExecutablePath(path, &size);
        path[size] = '\0';
        std::string res = std::string(path, size);
        return res;
    }
    std::string getbasedirpath()
    {
        std::string exe_path = getexepath();
        std::string exe_dir = exe_path.substr(0, exe_path.find_last_of(PATH_SEPARATOR));
        //tau/build/src/Debug/tau.app/Contents/MacOS/../../../../../../src/lib/browser/init.js
        return exe_dir + "/../../../../../..";
    }
    
    char* getCString(std::string str) {
        char* cstr = new char[str.length() + 1];
        strcpy(cstr, str.c_str());  // copy to get "un-const"
        return cstr;
    }
    
    int node_start() {
        char arg[] = "";
        
        // argv[1] path is treated relative to cwd which can change so we provide an
        // absolute path  generate relative to binary: tau/bin/Debug|Release/tau[.exe]
        std::string base_path = getbasedirpath();
        
        // printf("exe dir is %s \n", exe_path.c_str());
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
        
        node::MultiIsolatePlatform* platform __unused =
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
            
            // NODE_SET_METHOD(global, "addFast", AddFast);
            
            // api::BrowserWindow win = api::BrowserWindow(isolate, global);
            
            // registerModules();
            
            // RegisterBuiltinCustomModules();
            
            //..................
            
            // Local<Object> process(env->process_object());
            // FIXME process is nullptr
            // process->Set(v8::String::NewFromUtf8(isolate, "resourcesPath"),
            // String::NewFromUtf8(isolate, "resources_path_here"));
            
            LoadEnvironment(env);
            
            bool more;
            do {
                // CefDoMessageLoopWork();
                
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
}  // namespace

// Entry point function for the browser process.
int main(int argc, char* argv[]) {

  // Provide CEF with command-line arguments.
  CefMainArgs main_args(argc, argv);

  // Initialize the AutoRelease pool.
  NSAutoreleasePool* autopool = [[NSAutoreleasePool alloc] init];

  // Initialize the SimpleApplication instance.
  [SimpleApplication sharedApplication];

  // Specify CEF global settings here.
  CefSettings settings;

  // SimpleApp implements application-level callbacks for the browser process.
  // It will create the first browser instance in OnContextInitialized() after
  // CEF has initialized.
  CefRefPtr<SimpleApp> app(new SimpleApp);

  // Initialize CEF for the browser process.
  CefInitialize(main_args, settings, app.get(), NULL);

  // Create the application delegate.
  NSObject* delegate = [[SimpleAppDelegate alloc] init];
  [delegate performSelectorOnMainThread:@selector(createApplication:)
                             withObject:nil
                          waitUntilDone:NO];

  node_start();

  // Run the CEF message loop. This will block until CefQuitMessageLoop() is
  // called.
  CefRunMessageLoop();

  // Shut down CEF.
  CefShutdown();

  // Release the delegate.
  [delegate release];

  // Release the AutoRelease pool.
  [autopool release];

  return 0;
}
