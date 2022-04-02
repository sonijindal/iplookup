IPLookup is a tool to look up for the ASN and the block of network a given ip belongs to.
It checks an input IP against a database of network block to ASN mappings.
This mapping can be set as an environment variable. If the variable is not set,
the file is downloaded form https://lg01.infra.ring.nlnog.net/table.txt


Build:
```
g++ --std=c++1z IPLookup.cpp -o iplookup -lcurl
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

```<ip>``` is an IPv4 or IPv6 address
It runs the initial db building and first lookup of ths IP. The program also waits for input of more IPs to lookup.

Example run:

  ```
(base) sonika:IPProject$ ./iplookup 10.0.0.9
================ IP LOOKUP TOOL ===============
CONFIG_FILE_PATH set to table.txt
******* Test Start *********
Printing result:
Network: 10.0.0.0/8 ASN:  62538
******* Test End *********

******* Test Start *********
Enter IP for lookup:2804:6cac:2000:19:10:1:1:1
Printing result:
Network: 2804:6cac::/32 ASN:  270484
******* Test End *********

******* Test Start *********
Enter IP for lookup:2001:0db8:3c4d:0015:0000:0000:1a2f:1a2b
IP not found
******* Test End *********

******* Test Start *********
Enter IP for lookup:10.20.30.40
Printing result:
Network: 10.0.0.0/8 ASN:  62538
******* Test End *********

******* Test Start *********
Enter IP for lookup:^C
  ```
