
#ifndef KIWIX_SUBPROCESS_H_
#define KIWIX_SUBPROCESS_H_

#include <string>
#include <memory>
#include <vector>

typedef std::vector<const char *> commandLine_t;

class SubprocessImpl
{
  public:
    virtual void run(commandLine_t& commandLine) = 0;
    virtual bool kill() = 0;
    virtual bool isRunning() = 0;
    virtual ~SubprocessImpl() = default;
};

class Subprocess
{
  private:
    // Impl depends of the system (window, unix, ...)
    std::unique_ptr<SubprocessImpl> mp_impl;
    Subprocess(std::unique_ptr<SubprocessImpl> impl, commandLine_t& commandLine);

  public:
    static std::unique_ptr<Subprocess> run(commandLine_t& commandLine);
    ~Subprocess();

    bool isRunning();
    bool kill();
};


#endif // KIWIX_SUBPROCESS_H_
