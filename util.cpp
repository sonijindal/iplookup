#include "util.h"

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
        cout << "CONFIG_FILE_PATH is not set, so download" << endl;
        fileName = "download.txt";
        if (DownloadFile(url, fileName) == false)
        {
            cerr << "Unable to download from the URL" << endl;
            return "";
        }
    }
    else
    {
        cout << "CONFIG_FILE_PATH is set to " << filePath << endl;
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