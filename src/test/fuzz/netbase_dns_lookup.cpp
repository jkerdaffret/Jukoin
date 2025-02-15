// Copyright (c) 2021 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <netaddress.h>
#include <netbase.h>
#include <test/fuzz/FuzzedDataProvider.h>
#include <test/fuzz/fuzz.h>
#include <test/fuzz/util.h>

#include <cstdint>
#include <string>
#include <vector>

namespace {
FuzzedDataProvider* fuzzed_data_provider_ptr = nullptr;

std::vector<CNetAddr> fuzzed_dns_lookup_function(const std::string& name, bool allow_lookup)
{
    std::vector<CNetAddr> resolved_addresses;
    while (fuzzed_data_provider_ptr->ConsumeBool()) {
        resolved_addresses.push_back(ConsumeNetAddr(*fuzzed_data_provider_ptr));
    }
    return resolved_addresses;
}
} // namespace

FUZZ_TARGET(netbase_dns_lookup)
{
    FuzzedDataProvider fuzzed_data_provider{buffer.data(), buffer.size()};
    fuzzed_data_provider_ptr = &fuzzed_data_provider;
    const std::string name = fuzzed_data_provider.ConsumeRandomLengthString(512);
    const unsigned int max_results = fuzzed_data_provider.ConsumeIntegral<unsigned int>();
    const bool allow_lookup = fuzzed_data_provider.ConsumeBool();
    const int default_port = fuzzed_data_provider.ConsumeIntegral<int>();
    {
        std::vector<CNetAddr> resolved_addresses;
        if (LookupHost(name, resolved_addresses, max_results, allow_lookup, fuzzed_dns_lookup_function)) {
            for (const CNetAddr& resolved_address : resolved_addresses) {
                assert(!resolved_address.IsInternal());
            }
        }
        assert(resolved_addresses.size() <= max_results || max_results == 0);
    }
    {
        CNetAddr resolved_address;
        if (LookupHost(name, resolved_address, allow_lookup, fuzzed_dns_lookup_function)) {
            assert(!resolved_address.IsInternal());
        }
    }
    {
        std::vector<CService> resolved_services;
        if (Lookup(name, resolved_services, default_port, allow_lookup, max_results, fuzzed_dns_lookup_function)) {
            for (const CNetAddr& resolved_service : resolved_services) {
                assert(!resolved_service.IsInternal());
            }
        }
        assert(resolved_services.size() <= max_results || max_results == 0);
    }
    {
        CService resolved_service;
        if (Lookup(name, resolved_service, default_port, allow_lookup, fuzzed_dns_lookup_function)) {
            assert(!resolved_service.IsInternal());
        }
    }
    {
        CService resolved_service = LookupNumeric(name, default_port, fuzzed_dns_lookup_function);
        assert(!resolved_service.IsInternal());
    }
    {
        CSubNet resolved_subnet;
        if (LookupSubNet(name, resolved_subnet, fuzzed_dns_lookup_function)) {
            assert(resolved_subnet.IsValid());
        }
    }
    fuzzed_data_provider_ptr = nullptr;
}
