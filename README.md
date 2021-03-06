IPLookup is a tool to look up for the ASN and the block of network a given ip belongs to.
It checks an input IP (ipv4 or ipv6) against a database of network block to ASN mappings.
This mapping can be set as an environment variable. If the variable is not set,
the file is downloaded form https://lg01.infra.ring.nlnog.net/table.txt

Processing flow:
1. Find the file, either local if env variable (CONFIG_FILE_PATH) is set or download the file using curl.
2. Read the lines in the file and split each line by space. Insert them into a map of network to asn mapping. Exampple entry in map: Key: 8.8.8.0/24 Value: 15169
3. For the input IP, build all the networks which this IP can belong to by masking 32bits to 0 bits (128 to 0 for IPv6). Example: 
```
10.20.30.41/32
10.20.30.40/31
10.20.30.40/30
10.20.30.40/29
10.20.30.32/28
10.20.30.32/27
11.20.30.0/26
...
```

4. Look for each of these networks in the map.
5. For each entry found in the map, add the network and the ASN into the result vector.
6. Results will be in the sorted order from more specific to less specific.
```
(base) sonika:iplookup$ ./iplookup 10.20.30.41
================ IP LOOKUP TOOL ===============
Usage: ./iplookup <optional ip>
If no ip is passed during invokation, user can pass input on stdin
CONFIG_FILE_PATH can be set to a file with network to ASN mapping
CONFIG_FILE_PATH set to table.txt
Printing result:
Network: 10.0.0.0/8 ASN:  62538
```
Notes:
1. This has been tested on Mac and Linux.
2. libcurl is required to be installed beforehand.
3. IPv6 address int the db table should be in compressed form. Eg: 2001:0db8:3c4d:0015:1a2f:1a2b:0000:0000 can be abbreviated as 2001:db8:3c4d:15:1a2f:1a2b::
4. Input IPv6 address should be in the non-abbreviated form. Some variants of ipv6 address doesn't work at the moment.

Build:
```
g++ --std=c++1z IPLookup.cpp util.cpp -o iplookup -lcurl
```

Usage:

Optional env variable representing a file which contains entries of the form "NetworkBlock ASN"

```
export CCONFIG_FILE_PATH=<filepath>
```

Run:
```
./iplookup <ip>
```

```<ip>``` is an IPv4 or IPv6 address. This parameter is optional.
If ip is not passed during the invocation, the user is asked for the input on stdin.

Setup:
```
(base) sonika:iplookup$ sudo apt-get install libcurl
(base) sonika:iplookup$ g++ --std=c++1z IPLookup.cpp util.cpp -o iplookup -lcurl
(base) sonika:iplookup$ export CCONFIG_FILE_PATH=table.txt
```

IPv4:
```
(base) sonika:iplookup$ ./iplookup
================ IP LOOKUP TOOL ===============
Usage: ./iplookup <optional ip>
If no ip is passed during invokation, user can pass input on stdin
CONFIG_FILE_PATH can be set to a file with network to ASN mapping
CONFIG_FILE_PATH set to table.txt

******* Test Start *********
Enter IP for lookup:8.8.8.8
Printing result:
Network: 8.8.8.0/24 ASN:  15169
Network: 8.0.0.0/12 ASN:  3356
Network: 8.0.0.0/9 ASN:  3356
******* Test End *********

******* Test Start *********
Enter IP for lookup:8.9.9
IP not found
******* Test End *********

******* Test Start *********
Enter IP for lookup:^C
(base) sonika:iplookup$ ./iplookup 8.8.8.8
================ IP LOOKUP TOOL ===============
Usage: ./iplookup <optional ip>
If no ip is passed during invokation, user can pass input on stdin
CONFIG_FILE_PATH can be set to a file with network to ASN mapping
CONFIG_FILE_PATH set to table.txt
Printing result:
Network: 8.8.8.0/24 ASN:  15169
Network: 8.0.0.0/12 ASN:  3356
Network: 8.0.0.0/9 ASN:  3356
(base) sonika:iplookup$ 
```

IPv6:
```
(base) sonika:iplookup$ ./iplookup 2001:dc7:cd00:0000:0010:2b00:08a0:0000
================ IP LOOKUP TOOL ===============
Usage: ./iplookup <optional ip>
If no ip is passed during invokation, user can pass input on stdin
CONFIG_FILE_PATH can be set to a file with network to ASN mapping
===============================================

CONFIG_FILE_PATH is set to table.txt
File:table.txt
Printing result:
Network: 2001:dc7:cd00::/48 ASN:  24151
Network: 2001:dc7::/32 ASN:  24151
(base) sonika:iplookup$ 
```
