

#include "aria2.h"
#include "xmlrpc.h"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include "tools.h"
#include "tools/pathTools.h"
#include "tools/stringTools.h"
#include "tools/otherTools.h"
#include "downloader.h" // For AriaError

#ifdef _WIN32
# define ARIA2_CMD "aria2c.exe"
#else
# define ARIA2_CMD "aria2c"
# include <unistd.h>
#endif


#define LOG_ARIA_ERROR() \
  { \
    std::cerr << "ERROR: aria2 RPC request failed. (" << res << ")." << std::endl; \
    std::cerr << (curlErrorBuffer[0] ? curlErrorBuffer : curl_easy_strerror(res)) << std::endl; \
  }

namespace kiwix {

namespace {

void pauseAnyActiveDownloads(const std::string& ariaSessionFilePath)
{
    std::ifstream inputFile(ariaSessionFilePath);
    if ( !inputFile )
        return;

    std::ostringstream ss;
    std::string line;
    while ( std::getline(inputFile, line) ) {
        if ( !startsWith(line, " pause=") ) {
            ss << line << "\n";
        }
        if ( !line.empty() && line[0] != ' ' && line[0] != '#' ) {
            ss << " pause=true\n";
        }
    }

    std::ofstream outputFile(ariaSessionFilePath);
    outputFile << ss.str();
}

} // unnamed namespace

Aria2::Aria2(std::string sessionFileDir):
  mp_aria(nullptr),
  m_port(42042),
  m_secret(getNewRpcSecret())
{
  std::vector<const char*> callCmd;

  std::string rpc_port = "--rpc-listen-port=" + to_string(m_port);
  std::string session_file = appendToDirectory(sessionFileDir, "kiwix.session");
  pauseAnyActiveDownloads(session_file);
  std::string session = "--save-session=" + session_file;
  std::string inputFile = "--input-file=" + session_file;
//  std::string log_dir = "--log=\"" + logDir + "\"";
#ifdef _WIN32
  int pid = GetCurrentProcessId();
#else
  pid_t pid = getpid();
#endif
  std::string stop_with_pid = "--stop-with-process=" + to_string(pid);
  std::string rpc_secret = "--rpc-secret=" + m_secret;
  m_secret = "token:"+m_secret;

  std::string aria2cmd = appendToDirectory(
    removeLastPathElement(getExecutablePath(true)),
    ARIA2_CMD);
  if (fileExists(aria2cmd)) {
    // A local aria2c exe exists (packaged with kiwix-desktop), use it.
    callCmd.push_back(aria2cmd.c_str());
  } else {
    // Try to use a potential installed aria2c.
    callCmd.push_back(ARIA2_CMD);
  }
  callCmd.push_back("--follow-metalink=mem");
  callCmd.push_back("--enable-rpc");
  callCmd.push_back(rpc_secret.c_str());
  callCmd.push_back(rpc_port.c_str());
  if (fileReadable(session_file)) {
    callCmd.push_back(inputFile.c_str());
  }
  callCmd.push_back(session.c_str());
//  callCmd.push_back(log_dir.c_str());
  callCmd.push_back("--auto-save-interval=10");
  callCmd.push_back(stop_with_pid.c_str());
  callCmd.push_back("--allow-overwrite=true");
  callCmd.push_back("--dht-entry-point=router.bittorrent.com:6881");
  callCmd.push_back("--dht-entry-point6=router.bittorrent.com:6881");
  callCmd.push_back("--quiet=true");
  callCmd.push_back("--bt-enable-lpd=true");
  callCmd.push_back("--always-resume=true");
  callCmd.push_back("--max-concurrent-downloads=42");
  callCmd.push_back("--rpc-max-request-size=6M");
  callCmd.push_back("--file-allocation=none");
  std::string launchCmd;
  for (auto &cmd : callCmd) {
      launchCmd.append(cmd).append(" ");
  }
  mp_aria = Subprocess::run(callCmd);

  CURL* p_curl = curl_easy_init();
  char curlErrorBuffer[CURL_ERROR_SIZE];

  curl_easy_setopt(p_curl, CURLOPT_URL, "http://localhost/rpc");
  curl_easy_setopt(p_curl, CURLOPT_PORT, m_port);
  curl_easy_setopt(p_curl, CURLOPT_POST, 1L);
  curl_easy_setopt(p_curl, CURLOPT_ERRORBUFFER, curlErrorBuffer);
  curl_easy_setopt(p_curl, CURLOPT_TIMEOUT_MS, 100);

  typedef std::chrono::duration<double> Seconds;

  const double MAX_WAITING_TIME_SECONDS = 1;
  const auto t0 = std::chrono::steady_clock::now();
  bool maxWaitingTimeWasExceeded = false;

  CURLcode res = CURLE_OK;
  while ( !maxWaitingTimeWasExceeded ) {
    sleep(10);
    curlErrorBuffer[0] = 0;
    res = curl_easy_perform(p_curl);
    if (res == CURLE_OK) {
      break;
    }

    const auto dt = std::chrono::steady_clock::now() - t0;
    const double elapsedTime = std::chrono::duration_cast<Seconds>(dt).count();
    maxWaitingTimeWasExceeded = elapsedTime > MAX_WAITING_TIME_SECONDS;
  }
  curl_easy_cleanup(p_curl);
  if ( maxWaitingTimeWasExceeded ) {
    LOG_ARIA_ERROR();
    throw std::runtime_error("Cannot connect to aria2c rpc. Aria2c launch cmd : " + launchCmd);
  }
}

void Aria2::close()
{
  saveSession();
  shutdown();
}

size_t write_callback_to_iss(char* ptr, size_t size, size_t nmemb, void* userdata)
{
  auto outStream = static_cast<std::stringstream*>(userdata);
  outStream->write(ptr, nmemb);
  return nmemb;
}

std::string Aria2::doRequest(const MethodCall& methodCall)
{
  auto requestContent = methodCall.toString();
  std::stringstream outStream;
  CURLcode res;
  long response_code;
  char curlErrorBuffer[CURL_ERROR_SIZE];
  CURL* p_curl = curl_easy_init();
  curl_easy_setopt(p_curl, CURLOPT_URL, "http://localhost/rpc");
  curl_easy_setopt(p_curl, CURLOPT_PORT, m_port);
  curl_easy_setopt(p_curl, CURLOPT_POST, 1L);
  curl_easy_setopt(p_curl, CURLOPT_ERRORBUFFER, curlErrorBuffer);
  curl_easy_setopt(p_curl, CURLOPT_POSTFIELDSIZE, requestContent.size());
  curl_easy_setopt(p_curl, CURLOPT_POSTFIELDS, requestContent.c_str());
  curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, &write_callback_to_iss);
  curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, &outStream);
  curlErrorBuffer[0] = 0;
  res = curl_easy_perform(p_curl);
  if (res != CURLE_OK) {
    LOG_ARIA_ERROR();
    curl_easy_cleanup(p_curl);
    throw std::runtime_error("Cannot perform request");
  }
  curl_easy_getinfo(p_curl, CURLINFO_RESPONSE_CODE, &response_code);
  curl_easy_cleanup(p_curl);

  auto responseContent = outStream.str();
  if (response_code != 200) {
    std::cerr << "ERROR: Invalid return code (" << response_code << ") from aria :" << std::endl;
    std::cerr << responseContent << std::endl;
    throw std::runtime_error("Invalid return code from aria");
  }

  MethodResponse response(responseContent);
  if (response.isFault()) {
    throw AriaError(response.getFault().getFaultString());
  }
  return responseContent;
}

std::string Aria2::addUri(const std::vector<std::string>& uris, const std::vector<std::pair<std::string, std::string>>& options)
{
  MethodCall methodCall("aria2.addUri", m_secret);
  auto uriParams = methodCall.newParamValue().getArray();
  for (auto& uri : uris) {
    uriParams.addValue().set(uri);
  }
  for (auto& option : options) {
    methodCall.newParamValue().getStruct().addMember(option.first).getValue().set(option.second);
  }
  auto ret = doRequest(methodCall);
  MethodResponse response(ret);
  return response.getParamValue(0).getAsS();
}

std::string Aria2::tellStatus(const std::string& gid, const std::vector<std::string>& statusKey)
{
  MethodCall methodCall("aria2.tellStatus", m_secret);
  methodCall.newParamValue().set(gid);
  if (!statusKey.empty()) {
    auto statusArray = methodCall.newParamValue().getArray();
    for (auto& key : statusKey) {
      statusArray.addValue().set(key);
    }
  }
  return doRequest(methodCall);
}

std::string Aria2::getNewRpcSecret()
{
  std::string uuid = gen_uuid("");
  uuid.erase(std::remove(uuid.begin(), uuid.end(), '-'));
  return uuid.substr(0, 9);
}

std::vector<std::string> Aria2::tellActive()
{
  MethodCall methodCall("aria2.tellActive", m_secret);
  auto statusArray = methodCall.newParamValue().getArray();
  statusArray.addValue().set(std::string("gid"));
  auto responseContent = doRequest(methodCall);
  MethodResponse response(responseContent);
  std::vector<std::string> activeGID;
  int index = 0;
  while(true) {
    try {
      auto structNode = response.getParamValue(0).getArray().getValue(index++).getStruct();
      auto gidNode = structNode.getMember("gid");
      activeGID.push_back(gidNode.getValue().getAsS());
    } catch (InvalidRPCNode& e) { break; }
  }
  return activeGID;
}

std::vector<std::string> Aria2::tellWaiting()
{
  MethodCall methodCall("aria2.tellWaiting", m_secret);
  methodCall.newParamValue().set(0);
  methodCall.newParamValue().set(99); // max number of downloads to be returned, don't know how to set this properly assumed that there will not be more than 99 paused downloads.
  auto statusArray = methodCall.newParamValue().getArray();
  statusArray.addValue().set(std::string("gid"));
  auto responseContent = doRequest(methodCall);
  MethodResponse response(responseContent);
  std::vector<std::string> waitingGID;
  int index = 0;
  while(true) {
    try {
      auto structNode = response.getParamValue(0).getArray().getValue(index++).getStruct();
      auto gidNode = structNode.getMember("gid");
      waitingGID.push_back(gidNode.getValue().getAsS());
    } catch (InvalidRPCNode& e) { break; }
  }
  return waitingGID;
}

void Aria2::saveSession()
{
  MethodCall methodCall("aria2.saveSession", m_secret);
  doRequest(methodCall);
  std::cout << "session saved" << std::endl;
}

void Aria2::shutdown()
{
  MethodCall methodCall("aria2.shutdown", m_secret);
  doRequest(methodCall);
}

void Aria2::pause(const std::string& gid)
{
    MethodCall methodCall("aria2.pause", m_secret);
    methodCall.newParamValue().set(gid);
    doRequest(methodCall);
}

void Aria2::unpause(const std::string& gid)
{
    MethodCall methodCall("aria2.unpause", m_secret);
    methodCall.newParamValue().set(gid);
    doRequest(methodCall);
}

void Aria2::remove(const std::string& gid)
{
    MethodCall methodCall("aria2.remove", m_secret);
    methodCall.newParamValue().set(gid);
    doRequest(methodCall);
}

} // end namespace kiwix
