#ifndef KIWIX_SUBPROCESS_WINDOWS_H_
#define KIWIX_SUBPROCESS_WINDOWS_H_

#include "subprocess.h"

#include <windows.h>
#include <synchapi.h>

class WinImpl : public SubprocessImpl
{
  private:
    int m_pid;
    bool m_running;
    HANDLE m_subprocessHandle;
    HANDLE  m_waitingThreadHandle;
    CRITICAL_SECTION m_criticalSection;

  public:
    WinImpl();
    virtual ~WinImpl();

    void run(commandLine_t& commandLine);
    bool kill();
    bool isRunning();

    static DWORD WINAPI waitForPID(void* self);
};

#endif //KIWIX_SUBPROCESS_WINDOWS_H_
