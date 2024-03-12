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

Doing a constant time scalar multiplication on a Montgomery curve is easier, hence the conversion. This multiplication is done similarly to what is suggested in that document.


Although the step of the scalar multiplication per se is immune to cache attacks and timing attacks, that scalar is derived from a modular reduction and is involved in multiplication, and these computations are done via GMP. 

The most concerning and probably secret-dependant computation is the modular reduction of a 512-bit hash (interpreted as little-endian integer) to a 256-bit number.

Hardening this signature scheme would require to implement something like Barrett reduction in lieu of this step.

The multiplication involving the secret scalar could be then done via a handmade arithmetic library that performs addition and multiplication on 26-bit limbs : https://loup-vaillant.fr/tutorials/poly1305-design
