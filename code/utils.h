#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <gmp.h>

int is_hexstring(const char *hexstring, const size_t nbytes);
void hexstring2buffer(const char *hexstr, uint8_t *buf, 
                      const size_t nbytes);
void buffer2leint(const uint8_t *buffer, mpz_t leint, 
                  const size_t nbytes);
void leint2buffer(const mpz_t leint, uint8_t *buffer, 
                  const size_t nbytes);
void buffer2hexstream(const uint8_t *buf, FILE *stream, 
                     const size_t nbytes);
void addmodp(mpz_t rop, const mpz_t op1, const mpz_t op2, 
                    const mpz_t p);
void submodp(mpz_t rop, const mpz_t op1, const mpz_t op2, 
                    const mpz_t p);
void mulmodp(mpz_t rop, const mpz_t op1, const mpz_t op2, 
                    const mpz_t p);
void get_randomness(uint8_t *buffer, size_t nbytes);

#endif 
