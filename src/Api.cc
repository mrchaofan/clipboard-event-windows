#include <node.h>
#include <v8.h>
#include <uv.h>
#include <mutex>
#include <iostream>
#include "ChildThread.h"
#include "ClipboardEvent.h"
#define TESTENV false

namespace Chaofan
{
    namespace Api
    {
        using namespace v8;
        static Persistent<Function> callback;
        static ChildThread *childThread{nullptr};
        static bool clipboardHasUpdate = false;
        static bool requestShutdown = false;
        static std::mutex mutex;
        static void WakeupMain()
        {
            std::lock_guard guard{mutex};
            clipboardHasUpdate = true;
            // 先唤醒主线程再释放锁，防止低优先级任务线获得锁，导致主线程任务丢失事件
            childThread->WakeupMain();
        }

        static void RunInChildThread(void *)
        {
            if (TESTENV)
            {
                for (;;)
                {
                    uv_sleep(3000);
                    {
                        std::lock_guard guard{mutex};
                        if (requestShutdown)
                        {
                            break;
                        }
                        clipboardHasUpdate = true;
                        // 先唤醒主线程再释放锁，防止低优先级任务线获得锁，导致主线程任务丢失事件
                        childThread->WakeupMain();
                    }
                }
                return;
            }
            std::cout << GetCurrentThreadId();
            listenClipboardChange(WakeupMain);
        }
        static void RunInMainThread(void *)
        {
            Isolate *isolate{Isolate::GetCurrent()};
            // v8 local句柄回收依赖HandleScope
            HandleScope handleScope(isolate);
            bool hasUpdate = false;
            {
                std::lock_guard guard{mutex};
                if (clipboardHasUpdate)
                {
                    clipboardHasUpdate = false;
                    hasUpdate = true;
                }
            }
            if (hasUpdate && !callback.IsEmpty())
            {
                Local<Function> localCallback{callback.Get(isolate)};
                Local<Context> context{isolate->GetCurrentContext()};
                localCallback->Call(context, context->Global(), 0, nullptr);
            }
        }
        static void GracefullyShutdown(uv_thread_t *tid, void *)
        {
            if (TESTENV)
            {
                std::lock_guard guard{mutex};
                requestShutdown = true;
                return;
            }
            bool suc = PostThreadMessage((DWORD) GetThreadId(*tid), WM_GRACEFUL_EXIT, 0, 0);
            if (!suc) {
                DWORD er = GetLastError();
                std::cout << "PostThreadMessage" << er << "\n";
            }
        }

        static void SetClipboardListener(const v8::FunctionCallbackInfo<v8::Value> &args)
        {
            Isolate *isolate = args.GetIsolate();
            if (args.Length() == 0 || args[0]->IsNullOrUndefined())
            {
                callback.Reset();
                if (childThread != nullptr)
                {
                    childThread->RequestGracefullyShotdown();
                    childThread = nullptr;
                }
                args.GetReturnValue().Set(Boolean::New(isolate, false));
                return;
            }
            if (!callback.IsEmpty())
            {
                args.GetReturnValue().Set(Boolean::New(isolate, false));
                return;
            }

            if (!args[0]->IsFunction())
            {
                isolate->ThrowException(
                    Exception::TypeError(String::NewFromUtf8Literal(isolate, "callback is not a function.")));
                return;
            }
            Local<Function> localCallback = Local<Function>::Cast(args[0]);
            callback.Reset(isolate, localCallback);
            if (childThread == nullptr)
            {
                clipboardHasUpdate = false;
                requestShutdown = false;
                ChildThreadNewOptions option{
                    nullptr,
                    RunInChildThread,
                    RunInMainThread,
                    GracefullyShutdown};
                childThread = ChildThread::New(option);
            }
            args.GetReturnValue().Set(Boolean::New(isolate, true));
        };

        void INIT(
            Local<Object> exports,
            Local<Object> module,
            Local<Context> context)
        {
            NODE_SET_METHOD(module, "exports", SetClipboardListener);
        }
    }
}

NODE_MODULE_CONTEXT_AWARE(clipboard_update_event, Chaofan::Api::INIT);