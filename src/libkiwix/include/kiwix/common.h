
#ifndef _KIWIX_COMMON_H_
#define _KIWIX_COMMON_H_

#include <zim/zim.h>

#ifdef __GNUC__
#define DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define DEPRECATED __declspec(deprecated)
#else
#praga message("WARNING: You need to implement DEPRECATED for this compiler")
#define DEPRECATED
#endif


namespace kiwix
{

/**
 * `IpMode` is used to [configure](@ref Server::setIpMode()) a `Server` object
 * to listen for incoming connections not on a single IP address but on all
 * addresses of the specified family.
 */
enum class IpMode
{
  /**
   * Listen on all IPv4 addresses.
   */
  IPV4,

  /**
   * Listen on all IPv6 addresses.
   */
  IPV6,

  /**
   * Listen on all available addresses.
   */
  ALL,

  /**
   * `IpMode::AUTO` (which is the default) must be used when an explicit
   * (non-empty) IP address for listening is provided via
   * `Server::setAddress()`. If no such address is enforced, then
   * `IpMode::AUTO` is equivalent to `IpMode::ALL`.
   */
  AUTO
};

typedef zim::size_type size_type;
typedef zim::offset_type offset_type;

} // namespace kiwix

#endif //_KIWIX_COMMON_H_
