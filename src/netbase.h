// Copyright (c) 2009-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_NETBASE_H
#define BITCOIN_NETBASE_H

#if defined(HAVE_CONFIG_H)
#include <config/bitcoin-config.h>
#endif

#include <compat.h>
#include <netaddress.h>
#include <serialize.h>
#include <util/sock.h>

#include <functional>
#include <memory>
#include <stdint.h>
#include <string>
#include <type_traits>
#include <vector>

extern int nConnectTimeout;
extern bool fNameLookup;

//! -timeout default
static const int DEFAULT_CONNECT_TIMEOUT = 5000;
//! -dns default
static const int DEFAULT_NAME_LOOKUP = true;

enum class ConnectionDirection {
    None = 0,
    In = (1U << 0),
    Out = (1U << 1),
    Both = (In | Out),
};
static inline ConnectionDirection& operator|=(ConnectionDirection& a, ConnectionDirection b) {
    using underlying = typename std::underlying_type<ConnectionDirection>::type;
    a = ConnectionDirection(underlying(a) | underlying(b));
    return a;
}
static inline bool operator&(ConnectionDirection a, ConnectionDirection b) {
    using underlying = typename std::underlying_type<ConnectionDirection>::type;
    return (underlying(a) & underlying(b));
}

class proxyType
{
public:
    proxyType(): randomize_credentials(false) {}
    explicit proxyType(const CService &_proxy, bool _randomize_credentials=false): proxy(_proxy), randomize_credentials(_randomize_credentials) {}

    bool IsValid() const { return proxy.IsValid(); }

    CService proxy;
    bool randomize_credentials;
};

/** Credentials for proxy authentication */
struct ProxyCredentials
{
    std::string username;
    std::string password;
};

/**
 * Wrapper for getaddrinfo(3). Do not use directly: call Lookup/LookupHost/LookupNumeric/LookupSubNet.
 */
std::vector<CNetAddr> WrappedGetAddrInfo(const std::string& name, bool allow_lookup);

enum Network ParseNetwork(const std::string& net);
std::string GetNetworkName(enum Network net);
/** Return a vector of publicly routable Network names; optionally append NET_UNROUTABLE. */
std::vector<std::string> GetNetworkNames(bool append_unroutable = false);
bool SetProxy(enum Network net, const proxyType &addrProxy);
bool GetProxy(enum Network net, proxyType &proxyInfoOut);
bool IsProxy(const CNetAddr &addr);
bool SetNameProxy(const proxyType &addrProxy);
bool HaveNameProxy();
bool GetNameProxy(proxyType &nameProxyOut);

using DNSLookupFn = std::function<std::vector<CNetAddr>(const std::string&, bool)>;
extern DNSLookupFn g_dns_lookup;

bool LookupHost(const std::string& name, std::vector<CNetAddr>& vIP, unsigned int nMaxSolutions, bool fAllowLookup, DNSLookupFn dns_lookup_function = g_dns_lookup);
bool LookupHost(const std::string& name, CNetAddr& addr, bool fAllowLookup, DNSLookupFn dns_lookup_function = g_dns_lookup);
bool Lookup(const std::string& name, CService& addr, int portDefault, bool fAllowLookup, DNSLookupFn dns_lookup_function = g_dns_lookup);
bool Lookup(const std::string& name, std::vector<CService>& vAddr, int portDefault, bool fAllowLookup, unsigned int nMaxSolutions, DNSLookupFn dns_lookup_function = g_dns_lookup);
CService LookupNumeric(const std::string& name, int portDefault = 0, DNSLookupFn dns_lookup_function = g_dns_lookup);
bool LookupSubNet(const std::string& strSubnet, CSubNet& subnet, DNSLookupFn dns_lookup_function = g_dns_lookup);

/**
 * Create a TCP socket in the given address family.
 * @param[in] address_family The socket is created in the same address family as this address.
 * @return pointer to the created Sock object or unique_ptr that owns nothing in case of failure
 */
std::unique_ptr<Sock> CreateSockTCP(const CService& address_family);

/**
 * Socket factory. Defaults to `CreateSockTCP()`, but can be overridden by unit tests.
 */
extern std::function<std::unique_ptr<Sock>(const CService&)> CreateSock;

bool ConnectSocketDirectly(const CService &addrConnect, const SOCKET& hSocketRet, int nTimeout, bool manual_connection);
bool ConnectThroughProxy(const proxyType& proxy, const std::string& strDest, int port, const Sock& sock, int nTimeout, bool& outProxyConnectionFailed);
/** Disable or enable blocking-mode for a socket */
bool SetSocketNonBlocking(const SOCKET& hSocket, bool fNonBlocking);
/** Set the TCP_NODELAY flag on a socket */
bool SetSocketNoDelay(const SOCKET& hSocket);
void InterruptSocks5(bool interrupt);

bool Socks5(const std::string& strDest, int port, const ProxyCredentials* auth, const Sock& socket);

#endif // BITCOIN_NETBASE_H
