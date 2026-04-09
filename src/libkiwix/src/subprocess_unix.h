#ifndef KIWIX_SUBPROCESS_UNIX_H_
#define KIWIX_SUBPROCESS_UNIX_H_

#include "subprocess.h"

#include <atomic>
#include <thread>

class UnixImpl : public SubprocessImpl
{
  private:
    int m_pid;
    std::atomic<bool> m_running;
    std::atomic<bool> m_shouldQuit;
    std::thread m_waitingThread;

  public:
    UnixImpl();
    virtual ~UnixImpl();

    void run(commandLine_t& commandLine);
    bool kill();
    bool isRunning();

    static void* waitForPID(void* self);
};

#endif //KIWIX_SUBPROCESS_UNIX_H_
