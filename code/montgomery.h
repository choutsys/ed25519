#ifndef MONTGOMERY_H
#define MONTGOMERY_H

void xADD(const mpz_t XP,    const mpz_t ZP,   // Projective coords of P 
          const mpz_t XQ,    const mpz_t ZQ,   // Projective coords of Q
          const mpz_t Xdiff, const mpz_t Zdiff,// Proj coords of P - Q
                mpz_t Xsum,        mpz_t Zsum, // Proj coords of P + Q
          const mpz_t p);                      // Field parameter

void xDBL(const mpz_t XP, const mpz_t ZP,  // Projective coords of P
                mpz_t X2P,      mpz_t Z2P, // Projective coords of [2]P
          const mpz_t p,  const mpz_t a24);// a24 = (A+2)/4 in F_p

void ladder(const uint8_t *m, const size_t bits, // m = scalar  
            const mpz_t x, const mpz_t y, 
            mpz_t x_out, mpz_t y_out,
            const mpz_t p, const mpz_t A, const mpz_t B);


#endif // #ifndef MONTGOMERY_H
