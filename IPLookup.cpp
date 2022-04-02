#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <curl/curl.h>
#include <arpa/inet.h>
#include <sstream>

using namespace std;

class IPLookup
{
    map<string, string> cidrToAsn;
    enum ADDR_TYPE {
        IPV4_ADDR,
        IPV6_ADDR,
        INVALID
    };

    const int IPV4_SECTION_COUNT = 4;
    const int IPV4_SECTION_SIZE = 8;
    const int IPV6_SECTION_COUNT = 8;
    const int IPV6_SECTION_SIZE = 16;

    public:
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

    enum ADDR_TYPE GetAddrType(string ip)
    {
        struct in6_addr inaddrv6;
        struct in_addr inaddrv4;
        if (inet_pton(AF_INET, ip.c_str(), &inaddrv4) == 1)
        {
            return IPV4_ADDR;
        }
        else if (inet_pton(AF_INET6, ip.c_str(), &inaddrv6) == 1)
        {
            return IPV6_ADDR;
        }
        return INVALID;
    }

    string GetFileName()
    {
        string fileName;

        char* filePath = std::getenv("CONFIG_FILE_PATH");
        string url = "https://lg01.infra.ring.nlnog.net/table.txt";

        if(filePath == NULL)
        {
            cout << "CONFIG_FILE_PATH not set" << endl;
            fileName = "download.txt";
            if (DownloadFile(url, fileName) == false)
            {
                cerr << "Unable to download from the URL" << endl;
                return "";
            }
        }
        else
        {
            cout << "CONFIG_FILE_PATH set to " << filePath << endl;
            fileName = filePath;
        }
        return fileName;
    }

    bool DownloadFile(string url, string fileName)
    {
        CURL *curl_handle;
        FILE *file;
        curl_global_init(CURL_GLOBAL_ALL);
 
        curl_handle = curl_easy_init();
        
        curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
        
        curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
        
        file = fopen(fileName.c_str(), "wb");
        if (file) {
            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, file);
        
            curl_easy_perform(curl_handle);
        
            fclose(file);
        }
 
        curl_easy_cleanup(curl_handle);
        curl_global_cleanup();
    
        return true;
    }

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

    vector< pair<string, string> > FindNetworkAndAsn(string ip)
    {
        vector<pair<string, string> > result;

        // Build network from the IP
        ADDR_TYPE type = GetAddrType(ip);
        if (type == IPV4_ADDR)
        {
            // Ipv4
            for (int i = 32; i > 0; --i)
            {
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
                string network = ApplyMaskToIpv6(ip, i);
                if (cidrToAsn.find(network) != cidrToAsn.end())
                {
                    auto res = make_pair(network, cidrToAsn[network]);
                    result.push_back(res);
                }
            }
        }

        return result;
    }
};

void PrintResult(vector<pair<string, string> > result)
{
    if (result.size() == 0)
    {
        cout << "IP not found" << endl;
    }
    else
    {
        cout << "Printing result:" << endl;
        for (int i = 0; i < result.size(); ++i)
        {
            cout << "Network: " << result[i].first << " ASN: " << result[i].second << endl;
        }
    }
}

int main(int argc, char* argv[])
{
    // Validate input
    if (argc != 2)
    {
        cerr << "Usage: ./iplookup <ip>" << endl;
        return -1;
    }
    string ip = argv[1];
    cout << "================ IP LOOKUP TOOL ===============" << endl;
    IPLookup lookup;

    // Get DB File Name
    string fileName = lookup.GetFileName();
    if (fileName == "")
    {
        return -1;
    }
    // Build DB
    lookup.BuildMap(fileName);

    cout << "******* Test Start *********" << endl;
    // Lookup for input IP
    vector<pair<string, string> > result = lookup.FindNetworkAndAsn(ip);
    PrintResult(result);

    // Scan for more IPs
    cout << "******* Test End *********" << endl;
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

    return 0;
}
