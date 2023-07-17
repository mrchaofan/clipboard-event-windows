#ifndef CHILD_THREAD_H
#define CHILD_THREAD_H

#include <uv.h>
#define CHILD_THREAD_OPTIONS \
void *arg {nullptr};\
void (*RunInChildThread)(void *) {nullptr};\
void (*RunInMainThread)(void *) {nullptr};\
void (*GracefullyShutdown)(uv_thread_t *, void *) {nullptr};
namespace Chaofan
{
    struct ChildThreadNewOptions
    {
        CHILD_THREAD_OPTIONS
    };
    class ChildThread : private ChildThreadNewOptions
    {
    private:
        static void UVRunInMainThread(uv_async_t *);
        static void UVCloseCallback(uv_handle_t *);
        CHILD_THREAD_OPTIONS
        uv_thread_t tid;
        uv_async_t async;
        ChildThread(const ChildThreadNewOptions &);
        ChildThread(ChildThread &) = delete;
        ChildThread(ChildThread &&) = delete;
        void operator=(ChildThread&) = delete;
        void operator=(ChildThread&&) = delete;

    public:
        static ChildThread *New(const ChildThreadNewOptions &);
        static void WaitForShutdown(ChildThread *);
        ~ChildThread();
        void WakeupMain();
        void RequestGracefullyShotdown();
    };
}
#endif