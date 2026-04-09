

#include "subprocess.h"

#ifdef _WIN32
# include "subprocess_windows.h"
#else
# include "subprocess_unix.h"
#endif

Subprocess::Subprocess(std::unique_ptr<SubprocessImpl> impl, commandLine_t& commandLine) :
  mp_impl(std::move(impl))
{
  mp_impl->run(commandLine);
}

Subprocess::~Subprocess()
{
  mp_impl->kill();
}

std::unique_ptr<Subprocess> Subprocess::run(commandLine_t& commandLine)
{
#ifdef _WIN32
  auto impl = std::unique_ptr<SubprocessImpl>(new WinImpl);
#else
  auto impl = std::unique_ptr<UnixImpl>(new UnixImpl);
#endif
  return std::unique_ptr<Subprocess>(new Subprocess(std::move(impl), commandLine));
}

bool Subprocess::isRunning()
{
  return mp_impl->isRunning();
}

bool Subprocess::kill()
{
  return mp_impl->kill();
}
