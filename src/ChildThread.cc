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
        uv_sem_post(&childThread->sem);
    }

    ChildThread::ChildThread(const ChildThreadNewOptions &options) : arg(options.arg), RunInChildThread(options.RunInChildThread), GracefullyShutdown(options.GracefullyShutdown),
                                                                     RunInMainThread(options.RunInMainThread)
    {
        uv_async_init(uv_default_loop(), &this->async, UVRunInMainThread);
        this->async.data = this;
        uv_sem_init(&this->sem, 0);
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

    void ChildThread::wakeupMain()
    {
        uv_async_send(&this->async);
    }

    ChildThread::~ChildThread()
    {
        this->GracefullyShutdown(&this->tid, this->arg);
        uv_thread_join(&this->tid);
        uv_close((uv_handle_t *)&this->async, UVCloseCallback);
        uv_sem_wait(&this->sem);
        uv_sem_destroy(&this->sem);
    }
}
