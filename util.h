
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <curl/curl.h>
#include <arpa/inet.h>
#include <sstream>

using namespace std;

enum ADDR_TYPE {
    IPV4_ADDR,
    IPV6_ADDR,
    INVALID
};

void PrintResult(vector<pair<string, string> > result);
enum ADDR_TYPE GetAddrType(string ip);
string GetFileName();
bool DownloadFile(string url, string fileName);
