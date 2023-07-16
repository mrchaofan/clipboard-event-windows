#include <node.h>
#include <v8.h>
#include <uv.h>
#include "ChildThread.h"

namespace Chaofan
{
    namespace Api
    {
        using namespace v8;
        static Persistent<Function> callback;
        static ChildThread *childThread{nullptr};
        static bool clipboardHasUpdate = false;
        static bool requestShutdown = false;
        static uv_mutex_t mutex;
        static void RunInChildThread(void *)
        {
        }
        static void RunInMainThread(void *)
        {
        }
        static void GracefullyShutdown(uv_thread_t *, void *)
        {
        }

        static void SetClipboardListener(const v8::FunctionCallbackInfo<v8::Value> &args)
        {
            Isolate *isolate = args.GetIsolate();
            if (!callback.IsEmpty())
            {
                args.GetReturnValue().Set(Boolean::New(isolate, false));
                return;
            }
            if (args.Length() == 0)
            {
                callback.Reset();
                if (childThread != nullptr)
                {
                    delete childThread;
                    childThread = nullptr;
                }
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
            v8::Local<v8::Object> exports,
            v8::Local<v8::Value> module,
            v8::Local<v8::Context> context)
        {
            uv_mutex_init(&mutex);
        }
    }
}

NODE_MODULE_CONTEXT_AWARE(clipboard_update_event, Chaofan::Api::INIT);