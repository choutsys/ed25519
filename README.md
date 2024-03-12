# Ed25519 signature scheme with constant time scalar multiplication


## Build the programs

Navigate to this directory and run

```
$ make
```

## Run the programs

### Generate a random key pair 

```
$ ./keygen [prefix]
```

generates two keyfiles, one that corresponds to the private key with the `.sk` extension, and and that corresponds to the associated public key with the `.pk` extension. Both have the specified prefix in their name, and are exacty 32 bytes. 


### Sign a file with a private key

```
$ ./sign [prefix] [data] [sigfile]
```

where `prefix` is the common part of filenames (before the expected extensions `.pk` and `.sk`) of the keypair used to sign the `data` file.

These keyfiles should be exactly 32 bytes. 

The program has an undefined behavior if the public key and the private key don't match : the signature will probably be wrong in this case. 

Exactly 64 bytes are written to `sigfile`, that correspond to the signature of the file under `prefix.sk`

### Verify a signature

```
$ ./verify [pubkeyfile] [data] [sigfile]
```

prints `ACCEPT` or `REJECT` depending on whether `sigfile` contains the signature of the `data` file produced by the private key associated to the provided public key.

The program won't run if sigfile doesn't contain exactly 64 bytes, or if `pubkeyfile` doesn't contain exactly 32 bytes. 


## Constant time scalar multiplication

These programs were implemented by following the RFC 8032 specification on Ed25519 signature scheme. 
The implementation the RFC suggests doesn't do scalar multiplication on elliptic curves points in constant-time. 

For this specific computation, points on Edwards curve with parameters $p$, $a$, and $d$ are converted to points on the birationally equivalent Montgomery curve with parameters $p$, $A$ and $B$. 
Formulae were taken from https://inria.hal.science/hal-01483768/document

Doing a constant time scalar multiplication on a Montgomery curve is easier, hence the conversion. 

This constant-time scalar multiplication isn't satisfying because the secret scalars go through computations made by GMP. The most concerning and probably secret-dependant computation is the modular reduction of a 512-bit hash (interpreted as little-endian integer) to a (nearly) 256-bit number, that will serve a secret scalar.

Even though I had already implemented a basic low-level library to manipulate 26-bit limbs in order to perform modular multiplication in $\mathbb{F}_p$ where $p$ isn't far from a power of two, I am compelled to notice that the numbers manipulated had the right amount of limbs in the first place, beore being reduced or multiplied back then (assignment 2).


The difficulty here is to reduce a number that is supposed to have twice the number of 26-bit limbs (if we were to do the same as in Assignment 2) modulo an integer that isn't that close to a power of two. That was too much for my limbic system.

I stumbled upon Barrett reduction, the conditions here are met for $n$ (where $n$ is the distance to the nearest power of two of the size of the curve group) to apply that reduction but I couldn't implement it by the Assignment deadline. 


The other extensions suggested (multiexponentiation, and batch verifications) weren't implemented for the same reason. 
