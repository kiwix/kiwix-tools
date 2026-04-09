#ifndef KIWIXLIB_KIWIX_SERVE_H_
#define KIWIXLIB_KIWIX_SERVE_H_

#include <memory>
#include <string>

class Subprocess;
namespace kiwix {

class KiwixServe
{
  public:
    KiwixServe(const std::string& libraryPath, int port = 8181);
    ~KiwixServe();

    void run();
    void shutDown();
    bool isRunning();
    int getPort() { return m_port; }
    int setPort(int port);

  private:
  std::unique_ptr<Subprocess> mp_kiwixServe;
  int m_port;
  std::string m_libraryPath;
};

}; //end namespace kiwix

#endif // KIWIXLIB_KIWIX_SERVE_H_
