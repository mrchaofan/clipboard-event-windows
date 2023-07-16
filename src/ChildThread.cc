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

    ChildThread::ChildThread(const ChildThreadNewOptions &options) : arg(options.arg), RunInChildThread(options.RunInChildThread), GracefullyShutdown(options.GracefullyShutdown),
                                                                     RunInMainThread(options.RunInMainThread)
    {
        uv_async_init(uv_default_loop(), &this->async, UVRunInMainThread);
        this->async.data = this;
    }

    ChildThread *ChildThread::New(const ChildThreadNewOptions &options)
    {
        ChildThread *childThread = new ChildThread(options);
        int er = uv_thread_create(&childThread->tid, childThread->RunInChildThread, childThread->arg);
        if (er)
        {
            delete childThread;
            return nullptr;
        }
        return childThread;
    }

    void ChildThread::WakeupMain()
    {
        uv_async_send(&this->async);
    }

    void ChildThread::RequestGracefullyShotdown()
    {
        this->GracefullyShutdown(&this->tid, this->arg);
        uv_thread_join(&this->tid);
        uv_close((uv_handle_t *)&this->async, UVCloseCallback);
    }

    ChildThread::~ChildThread()
    {
        // 由于uv_close需要的uv_handle_t载调用栈的周期内依然被liuv使用，所以这里不能遵守RAII
    }
}
