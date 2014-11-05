Encryption
==========

SSL capable proxy

Simple multiclient secure rpoxy - the asiest way to turn your unsecure channel into secure one. 
Applicable for protocols that do not replicate IP layer - does not incapsulate source/desctination IP. Poxy
does not inspect packets.

Example of usage:
```
 <ssl-tunnel> <s>sourceIP <s>destinationIP 
 where:
          <ssl-tunnel> - one of the two available implementations (see below)
          <s>sourceIP - ip:port to listen for incoming connection 
                            (e.g. s0.0.0.0:443 as for secure or 0.0.0.0:80 for NONsecure)
          <s>destinationIP - same for descination ip:port
```
          
Two implementation

* Python (python/ssl-tunnel.py). Easy to extend. No external dependency. Works on any platform with Python 2.7. Uses one connection-one-thread approach. Not sutable for >1000 connections
* Native (native/bin/tunnel). Platform - POSIX Linux, tested with Ubuntu 14 LTS. Worker pool oriented (only one worker is implemented as for now).
  * External dependency:
    * Log4cpp
    * openssl
    
  * How to build:
``` 
      cd native
      make
      (optional) mate test 
```
*Instalation
  * Use install script to generate requred self signed certificates and keys. 
    
    
