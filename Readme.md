#NOTE: This implementation is tested with Ubuntu 16-04 version.

# EC-ElGamal

This repository contains a C implementation of the additive homomorphic elliptic curve based EL-Gamal cryptographic scheme and a corresponding Java JNI wrapper. The elliptic curve operations of OpenSSL are used for the implementation.
Ciphertexts can be added toghether such that the decrypted result corresponds to the sum of the plaintexts (i.e. p1 + p2 = Dec(Enc(p1) ++ Enc(p2)))) similar to the Paillier-cryptosystem.
This crypto system is implemented on top of the base provided by https://github.com/lubux/ecelgamal.

## Content 
The *native* folder contains the C/C++ implementation of the DataRing.

### Prerequisites 
Make sure you have the following installed:
 * [CMake](https://cmake.org/)
 * [OpenSSL](http://www.openssl.org/source/)
 * [Maven](https://maven.apache.org/)
 * Java JDK 1.8
 * Boost C++ Libraries (http://www.boost.org)

### C Library
The C library contains two versions of EC-Elgamal, a basic version and a Chinese Remainder Thereom (CRT) based optimized version, as introduced by [Pilatus](http://www.vs.inf.ethz.ch/publ/papers/mshafagh_SenSys17_Pilatus.pdf). The library builds with cmake. To compile the library and run the testing.cpp: 

```
cd native
cmake .
make
./out/ecelgamal
```

## Security
This library is for academic purposes, gives no security guarantees and may contain implementation vulnerabilities.
