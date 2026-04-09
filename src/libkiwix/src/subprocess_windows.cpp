

#include "subprocess_windows.h"

#include <windows.h>
#include <winbase.h>
#include <shlwapi.h>
#include <iostream>
#include <sstream>

WinImpl::WinImpl():
  m_pid(0),
  m_running(false),
  m_subprocessHandle(INVALID_HANDLE_VALUE),
  m_waitingThreadHandle(INVALID_HANDLE_VALUE)
{
  InitializeCriticalSection(&m_criticalSection);
}

WinImpl::~WinImpl()
{
  kill();
  WaitForSingleObject(m_waitingThreadHandle, INFINITE);
  CloseHandle(m_subprocessHandle);
  DeleteCriticalSection(&m_criticalSection);
}

DWORD WINAPI WinImpl::waitForPID(void* _self)
{
  WinImpl* self = static_cast<WinImpl*>(_self);
  WaitForSingleObject(self->m_subprocessHandle, INFINITE);

  EnterCriticalSection(&self->m_criticalSection);
  self->m_running = false;
  LeaveCriticalSection(&self->m_criticalSection);

  return 0;
}

std::unique_ptr<wchar_t[]> toWideChar(const std::string& value, size_t min_size = 0)
{
  size_t size = MultiByteToWideChar(CP_UTF8, 0,
                value.c_str(), -1, nullptr, 0);
  auto wdata = std::unique_ptr<wchar_t[]>(new wchar_t[size>min_size?size:min_size]);
  auto ret = MultiByteToWideChar(CP_UTF8, 0,
                value.c_str(), -1, wdata.get(), size);
  if (0 == ret) {
    std::ostringstream oss;
    oss << "Cannot convert to wchar : " << GetLastError();
    throw std::runtime_error(oss.str());
  }
  if (size < min_size) {
    memset(wdata.get() + size, 0, min_size-size);
  }
  return wdata;
}


void WinImpl::run(commandLine_t& commandLine)
{
  STARTUPINFOW startInfo = {0};
  PROCESS_INFORMATION procInfo;
  startInfo.cb = sizeof(startInfo);
  std::wostringstream oss;
  for(auto& item: commandLine) {
    auto witem = toWideChar(item, MAX_PATH);
    PathQuoteSpacesW(witem.get());
    oss << witem.get() << " ";
  }
  auto wCommandLine = oss.str();
  if (CreateProcessW(
    NULL,
    const_cast<wchar_t*>(wCommandLine.c_str()),
    NULL,
    NULL,
    false,
    CREATE_NO_WINDOW,
    NULL,
    NULL,
    &startInfo,
    &procInfo))
  {
    m_pid = procInfo.dwProcessId;
    m_subprocessHandle = procInfo.hProcess;
    CloseHandle(procInfo.hThread);
    m_running = true;
    m_waitingThreadHandle = CreateThread(NULL, 0, &waitForPID, this, 0, NULL);
  }
}

bool WinImpl::kill()
{
  return TerminateProcess(m_subprocessHandle, 0);
}

bool WinImpl::isRunning()
{
  EnterCriticalSection(&m_criticalSection);
  bool ret = m_running;
  LeaveCriticalSection(&m_criticalSection);
  return ret;
}
