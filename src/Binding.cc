#include <node.h>
#include <v8.h>
#include <uv.h>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include "ChildThread.h"
#ifdef _WIN32
#include "ClipboardEvent.h"
#endif

namespace Chaofan
{
    namespace Binding
    {
        using namespace v8;
        static Persistent<Function> callback;
        static ChildThread *childThread{nullptr};
        static bool clipboardHasUpdate = false;
        static bool requestShutdown = false;
        static std::mutex mutex;
        static std::condition_variable cond;
        static bool messageLoopReady = false;
        static void DispathEventLoop()
        {
            std::lock_guard<std::mutex> guard{mutex};
            clipboardHasUpdate = true;
            // 先唤醒主线程再释放锁，防止低优先级任务线获得锁，导致主线程任务丢失事件
            childThread->DispatchEventLoop();
        }

        static void RunInChildThread(void *)
        {
#ifndef _WIN32
            for (;;)
            {
                uv_sleep(3000);
                {
                    std::lock_guard<std::mutex> guard{mutex};
                    if (requestShutdown)
                    {
                        break;
                    }
                    clipboardHasUpdate = true;
                    // 先唤醒主线程再释放锁，防止低优先级任务线获得锁，导致主线程任务丢失事件
                    childThread->DispatchEventLoop();
                }
            }
#else
            PeekMessageW(NULL, NULL, 0, 0, PM_NOREMOVE);
            std::unique_lock<std::mutex> lk(mutex);
            messageLoopReady = true;
            cond.notify_all();
            lk.unlock();
            listenClipboardChange(DispathEventLoop);
#endif
        }
        static void RunInMainThread(void *)
        {
            Isolate *isolate{Isolate::GetCurrent()};
            // v8 local句柄回收依赖HandleScope
            HandleScope handleScope(isolate);
            bool hasUpdate = false;
            {
                std::lock_guard<std::mutex> guard{mutex};
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
        static void Terminate(uv_thread_t *tid, void *)
        {
#ifndef _WIN32
            std::lock_guard<std::mutex> guard{mutex};
            requestShutdown = true;
#else
            std::unique_lock<std::mutex> lk(mutex);
            cond.wait(lk, [] {
                return messageLoopReady;
            });
            lk.unlock();
            TerminateWin32(tid);
#endif
        }

        static void SetClipboardListener(const v8::FunctionCallbackInfo<v8::Value> &args)
        {
            Isolate *isolate = args.GetIsolate();
            if (args.Length() == 0 || args[0]->IsNullOrUndefined())
            {
                callback.Reset();
                if (childThread != nullptr)
                {
                    childThread->RequestTerminate();
                    ChildThread::WaitForTerminate(childThread);
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
                messageLoopReady = false;
                ChildThreadNewOptions option;
                option.arg = nullptr;
                option.RunInChildThread = RunInChildThread;
                option.RunInMainThread = RunInMainThread;
                option.Terminate = Terminate;
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

NODE_MODULE_CONTEXT_AWARE(clipboard_update_event, Chaofan::Binding::INIT);