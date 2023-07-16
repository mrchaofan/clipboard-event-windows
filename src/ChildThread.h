#ifndef CHILD_THREAD_H
#define CHILD_THREAD_H

#include <uv.h>

namespace Chaofan
{
    struct ChildThreadNewOptions
    {
        void *arg;
        void (*RunInChildThread)(void *);
        void (*RunInMainThread)(void *);
        void (*GracefullyShutdown)(uv_thread_t *, void *);
    };
    class ChildThread : private ChildThreadNewOptions
    {
    private:
        static void UVRunInMainThread(uv_async_t *);
        static void UVCloseCallback(uv_handle_t *);
        uv_thread_t tid;
        uv_async_t async;
        void *arg;
        void (*RunInChildThread)(void *);
        void (*GracefullyShutdown)(uv_thread_t *, void *);
        void (*RunInMainThread)(void *);
        ChildThread(const ChildThreadNewOptions &);
        ChildThread(ChildThread &) = delete;
        ChildThread(ChildThread &&) = delete;
        void operator=(ChildThread&) = delete;
        void operator=(ChildThread&&) = delete;

    public:
        static ChildThread *New(const ChildThreadNewOptions &);
        ~ChildThread();
        void WakeupMain();
        void RequestGracefullyShotdown();
    };
}
#endif