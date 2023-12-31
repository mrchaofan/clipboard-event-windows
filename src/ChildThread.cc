#include "ChildThread.h"
#include "uv.h"

namespace Chaofan
{

    void ChildThread::UVRunInMainThread(uv_async_t *async)
    {
        ChildThread *childThread = (ChildThread *)async->data;
        childThread->RunInMainThread(childThread->arg);
    }

    void ChildThread::UVCloseCallback(uv_handle_t *async)
    {
        ChildThread *childThread = (ChildThread *)async->data;
        delete childThread;
    }

    ChildThread::ChildThread(const ChildThreadNewOptions &options) : arg(options.arg), RunInChildThread(options.RunInChildThread),
                                                                     RunInMainThread(options.RunInMainThread), Terminate(options.Terminate)
    {
    }

    ChildThread *ChildThread::New(const ChildThreadNewOptions &options)
    {
        ChildThread *childThread = new ChildThread(options);
        int er = uv_async_init(uv_default_loop(), &childThread->async, UVRunInMainThread);
        if (er)
        {
            delete childThread;
            return nullptr;
        }
        childThread->async.data = childThread;
        er = uv_thread_create(&childThread->tid, childThread->RunInChildThread, childThread->arg);
        if (er)
        {
            uv_close((uv_handle_t *)&childThread->async, UVCloseCallback);
            return nullptr;
        }
        return childThread;
    }

    void ChildThread::DispatchEventLoop()
    {
        uv_async_send(&this->async);
    }

    void ChildThread::RequestTerminate()
    {
        this->Terminate(&this->tid, this->arg);
    }

    void ChildThread::WaitForTerminate(ChildThread *childThread)
    {
        uv_thread_join(&childThread->tid);
        uv_close((uv_handle_t *)&childThread->async, UVCloseCallback);
    }

    ChildThread::~ChildThread()
    {
    }
}
