

#include "subprocess_unix.h"

#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>

UnixImpl::UnixImpl():
  m_pid(0),
  m_running(false),
  m_shouldQuit(false)
{
}

UnixImpl::~UnixImpl()
{
  kill();
  m_shouldQuit = true;
  m_waitingThread.join();
}

void* UnixImpl::waitForPID(void* _self)
{
  UnixImpl* self = static_cast<UnixImpl*>(_self);
  while (true) {
    if (!waitpid(self->m_pid, NULL, WNOHANG)) {
      break;
    }
    if (self->m_shouldQuit) {
      return nullptr;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  self->m_running = false;

  return self;
}

void UnixImpl::run(commandLine_t& commandLine)
{
  const char* binary = commandLine[0];
  int pid = fork();
  switch(pid) {
    case -1:
      std::cerr << "cannot fork" << std::endl;
      break;
    case 0:
      commandLine.push_back(NULL);
      if (execvp(binary, const_cast<char* const*>(commandLine.data()))) {
        perror("Cannot launch\n");
        _exit(-1);
      }

      break;
    default:
      m_pid = pid;
      m_running = true;
      m_waitingThread = std::thread(waitForPID, this);
      break;
  }
}

bool UnixImpl::kill()
{
  return (::kill(m_pid, SIGKILL) == 0);
}

bool UnixImpl::isRunning()
{
  return m_running;
}
