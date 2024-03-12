#ifndef EDWARDS_H
#define EDWARDS_H


void scalar_mult(uint8_t *m, const size_t bits,
                 const mpz_t x, const mpz_t y,
                 mpz_t x_out, mpz_t y_out,
                 const mpz_t p, const mpz_t a, const mpz_t d);

#endif
