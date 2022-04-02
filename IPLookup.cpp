// IPLookup tool

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <curl/curl.h>
#include <arpa/inet.h>
#include <sstream>

#include "util.h"

using namespace std;

class IPLookup
{
    map<string, string> cidrToAsn;

    const int IPV4_SECTION_COUNT = 4;
    const int IPV4_SECTION_SIZE = 8;
    const int IPV6_SECTION_COUNT = 8;
    const int IPV6_SECTION_SIZE = 16;

    public:
    // Build network address from the input ipv4 address by masking some bits
    // as passed in the argument
    // (10.20.30.41, 31) returns 10.20.30.40/31
    string ApplyMaskToIp(string ip, int maskBits)
    {
        vector<uint8_t> ipv4Octets;
        string result;
        stringstream ss(ip);
        char delim = '.';
        string token;

        while (getline(ss, token, delim))
        {
            ipv4Octets.push_back(stoi(token));
        }

        uint8_t octetsMasked = maskBits / IPV4_SECTION_SIZE;
        int i = 0;
        for (; i < octetsMasked; ++i)
        {
            if (i != 0)
            {
                result += ".";
            }
            result += to_string(ipv4Octets[i]);
        }

        uint8_t remainingMaskBits = (maskBits % IPV4_SECTION_SIZE);
        if (remainingMaskBits > 0)
        {
            if (i != 0)
            {
                result += ".";
            }
            uint8_t mask = ~((1 << (IPV4_SECTION_SIZE - remainingMaskBits)) - 1);
            
            uint8_t remainingMask = ipv4Octets[i] & mask;
            result += to_string(remainingMask);
            i++;
        }
        while (i < IPV4_SECTION_COUNT)
        {
            result += ".0";
            ++i;
        }
        result += "/" + to_string(maskBits);

        return result;
        
    }

    // Build network address from the input ipv6 address by masking some bits
    // as passed in the argument
    // (2804:6cac:2000:19:10:1:1:1, 112) returns 2804:6cac:2000:19:10:1:1::/112
    string ApplyMaskToIpv6(string ip, int maskBits)
    {
        vector<uint16_t> ipv4Octets;
        string result;
        stringstream ss(ip);
        char delim = ':';
        string token;

        while (getline(ss, token, delim))
        {
            ipv4Octets.push_back(stoi(token, 0, 16));
        }

        uint16_t octetsMasked = maskBits / IPV6_SECTION_SIZE;
        int i = 0;
        for (; i < octetsMasked; ++i)
        {
            stringstream str;
            if (i != 0)
            {
                result += ":";
            }
            str << hex << ipv4Octets[i];
            result += str.str();
        }

        uint16_t remainingMaskBits = (maskBits % IPV6_SECTION_SIZE);
        if (remainingMaskBits > 0)
        {
            if (i != 0)
            {
                result += ":";
            }
            uint16_t mask = ~((1 << (IPV6_SECTION_SIZE - remainingMaskBits)) - 1);
            
            uint16_t remainingMask = ipv4Octets[i] & mask;
            stringstream str;
            str << hex << remainingMask;
            result += str.str();
            i++;
        }
        if (i < IPV6_SECTION_COUNT)
        {
            result += "::";
            ++i;
        }
        result += "/" + to_string(maskBits);

        return result;
        
    }

    // Build a map of all the networks we have to the respective ASNs
    bool BuildMap(string fileName)
    {
        ifstream file(fileName);
        if (!file.is_open())
        {
            cerr << "Invalid file passed: " << fileName << endl;
            return false;
        }

        string line;
        while (getline(file, line))
        {
            auto pos = line.find(" ");
            if (pos != string::npos)
            {
                auto first = line.substr(0, pos);
                auto second = line.substr(pos);
                cidrToAsn[first] = second;
            }
        }

        file.close();
        return true;
    }

    // Find all the networks where this IP belong
    vector< pair<string, string> > FindNetworkAndAsn(string ip)
    {
        vector<pair<string, string> > result;

        ADDR_TYPE type = GetAddrType(ip);
        if (type == IPV4_ADDR)
        {
            // Ipv4
            for (int i = 32; i > 0; --i)
            {
                // Build network from the IP and mast bits
                string network = ApplyMaskToIp(ip, i);
                if (cidrToAsn.find(network) != cidrToAsn.end())
                {
                    auto res = make_pair(network, cidrToAsn[network]);
                    result.push_back(res);
                }
            }
        }
        else if (type == IPV6_ADDR)
        {
            // Ipv6
            for (int i = 128; i > 0; --i)
            {
                // Build network from the IP and mast bits
                string network = ApplyMaskToIpv6(ip, i);
                if (cidrToAsn.find(network) != cidrToAsn.end())
                {
                    auto res = make_pair(network, cidrToAsn[network]);
                    result.push_back(res);
                }
            }
        }
        else
        {
            cout << "IP <" << ip << "> is invalid" << endl;
        }

        return result;
    }
};

void ProcessInputs(IPLookup lookup)
{
    // Scan for more IPs
    vector<pair<string, string> > result;
    string ip;
    cout << endl;
    cout << "******* Test Start *********" << endl;
    cout << "Enter IP for lookup:";
    while (cin >> ip)
    {
        result = lookup.FindNetworkAndAsn(ip);
        PrintResult(result);
        cout << "******* Test End *********" << endl;
        cout << endl;
        cout << "******* Test Start *********" << endl;
        cout << "Enter IP for lookup:";
    }
}

int main(int argc, char* argv[])
{
    // Validate input
    cout << "================ IP LOOKUP TOOL ===============" << endl;
    cout << "Usage: ./iplookup <optional ip>" << endl;
    cout << "If no ip is passed during invokation, user can pass input on stdin" << endl;
    cout << "CONFIG_FILE_PATH can be set to a file with network to ASN mapping" << endl;
    cout << "===============================================" << endl;
    cout << endl;

    string ip = "";
    if (argc == 2)
    {
        ip = argv[1];
    }
    IPLookup lookup;

    // Get DB File Name
    string fileName = GetFileName();
    if (fileName == "")
    {
        cerr << "Invalid file!" << endl;
        return -1;
    }
    
    // Build DB
    if (lookup.BuildMap(fileName) == false)
    {
        cerr << "Building DB failed!" << endl;
        return -1;
    }

    // Process single input
    // Lookup for input IP
    if (ip != "")
    {
        vector<pair<string, string> > result = lookup.FindNetworkAndAsn(ip);
        if (result.size() == 0)
        {
            cerr << "No result found, check the input!" << endl;
            return -1;
        }
        PrintResult(result);
    }
    else
    {
        // Work on multiple inputs from user 
        ProcessInputs(lookup);
    }

    return 0;
}
