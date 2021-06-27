

# EC-ElGamal

This repository contains a C implementation of the additive homomorphic elliptic curve based EL-Gamal cryptographic scheme and a corresponding Java JNI wrapper. The elliptic curve operations of OpenSSL are used for the implementation.
Ciphertexts can be added toghether such that the decrypted result corresponds to the sum of the plaintexts (i.e. p1 + p2 = Dec(Enc(p1) ++ Enc(p2)))) similar to the Paillier-cryptosystem.
This crypto system is implemented on top of the base provided by https://github.com/lubux/ecelgamal.

## Content 
The *native* folder contains the C/C++ implementation of the DataRing.

The *data_preProcessing* folder contains jupyter notebooks for processing the Loans dataset.

The *theoretical_analysis* folder contains python notebooks for probabilistic analysis in datasharing system.

The *api_doc* folder contains the API documentation of DataRing. We can display it as a web-page by hosting this folder in a server.

### Prerequisites 
Make sure you have the following installed:
 * [CMake](https://cmake.org/)
 * [OpenSSL](http://www.openssl.org/source/)
 * [Maven](https://maven.apache.org/)
 * Java JDK 1.8
 * Boost C++ Libraries (http://www.boost.org)


## Build and Run
The library builds with cmake. To compile the library and run the testing.cpp: 

```
cd native
cmake .
make
./out/ecelgamal
```

#NOTE: This implementation is tested with Ubuntu 16-04 version. It shoudl work with newer version too.

## Security
This library is for academic purposes, gives no security guarantees and may contain implementation vulnerabilities.
