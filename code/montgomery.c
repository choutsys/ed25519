#include "utils.h"
void xADD(const mpz_t XP,    const mpz_t ZP,   // Projective coords of P 
          const mpz_t XQ,    const mpz_t ZQ,   // Projective coords of Q
          const mpz_t Xdiff, const mpz_t Zdiff,// Proj coords of P - Q
                mpz_t Xsum,        mpz_t Zsum, // Proj coords of P + Q
          const mpz_t p)
{
  mpz_t u, v, temp;
  mpz_inits(u, v, temp, NULL);

  // u = (XP - ZP)(XQ +ZQ)
  submodp(u, XP, ZP, p);
  addmodp(temp, XQ, ZQ, p);
  mulmodp(u, u, temp, p);

  // v = (XP + ZP)(XQ - ZQ)
  addmodp(v, XP, ZP, p);
  submodp(temp, XQ, ZQ, p);
  mulmodp(v, v, temp, p);

  // Xsum = Zdiff(u + v)^2
  addmodp(Xsum, u, v, p);
  mpz_powm_ui(Xsum, Xsum, 2, p);
  mulmodp(Xsum, Xsum, Zdiff, p);

  // Zsum = Xdiff(u - v)^2
  submodp(Zsum, u, v, p);
  mpz_powm_ui(Zsum, Zsum, 2, p);
  mulmodp(Zsum, Zsum, Xdiff, p);

  mpz_clears(u, v, temp, NULL);
}


void xDBL(const mpz_t XP, const mpz_t ZP,  // Projective coords of P
                mpz_t X2P,      mpz_t Z2P, // Projective coords of [2]P
          const mpz_t p,  const mpz_t a24) // a24 = (A + 2)/4
{
  mpz_t q, r, s;
  mpz_inits(q, r, s, NULL);

  // q = (XP + ZP)^2
  addmodp(q, XP, ZP, p);
  mpz_powm_ui(q, q, 2, p);
  
  // r = (XP - ZP)^2
  submodp(r, XP, ZP, p);
  mpz_powm_ui(r, r, 2, p);

  // s = q - r
  submodp(s, q, r, p);
  
  // X2P = qr
  mulmodp(X2P, q, r, p);

  // Z2P = S*(R + a24*S)
  mulmodp(Z2P, a24, s, p);
  addmodp(Z2P, Z2P, r, p);
  mulmodp(Z2P, Z2P, s, p);

  mpz_clears(q, r, s, NULL);
}

// swaps (X1 : Z1) and (X2 : Z2) depending on the value of swap
static void cswap(const int swap, mpz_t X1, mpz_t Z1, mpz_t X2, mpz_t Z2)
{
  mpz_t mask, dummy;
  size_t bits, tmp;
  
  bits = mpz_sizeinbase(X1, 2);
  bits = ((tmp = mpz_sizeinbase(Z1, 2)) > bits) ? tmp : bits;
  bits = ((tmp = mpz_sizeinbase(X2, 2)) > bits) ? tmp : bits;
  bits = ((tmp = mpz_sizeinbase(Z2, 2)) > bits) ? tmp : bits;

  mpz_init_set_ui(mask, 1);
  mpz_mul_2exp(mask, mask, bits); // 100......00 with bits trailing zeros 
  mpz_sub_ui(mask, mask, swap);

  // conditional swap of X1 and X2
  mpz_init_set(dummy, X1);
  mpz_xor(dummy, dummy, X2);
  mpz_and(dummy, dummy, mask);
  mpz_xor(X1, X1, dummy);
  mpz_xor(X2, X2, dummy);
 
  // conditional swap of Z1 and Z2
  mpz_set(dummy, Z1);
  mpz_xor(dummy, dummy, Z2);
  mpz_and(dummy, dummy, mask);
  mpz_xor(Z1, Z1, dummy);
  mpz_xor(Z2, Z2, dummy);
 
  mpz_clears(mask, dummy, NULL);
}

static
void recover(const mpz_t XP, const mpz_t YP, // ZP =  1
             mpz_t XQ, mpz_t YQ, mpz_t ZQ,   // recover YQ
             const mpz_t Xsum, const mpz_t Zsum, // (X(P + Q) : Z(P + Q))
             const mpz_t p, const mpz_t twoa, const mpz_t twob)
{
  mpz_t v1, v2, v3, v4;

  mpz_inits(v1, v2, v3, v4, NULL);

  mulmodp(v1, XP, ZQ, p);
  addmodp(v2, XQ, v1, p);
  submodp(v3, XQ, v1, p);
  mpz_powm_ui(v3, v3, 2, p);
  mulmodp(v3, v3, Xsum, p);
  mulmodp(v1, twoa, ZQ, p);
  addmodp(v2, v2, v1, p);
  mulmodp(v4, XP, XQ, p);
  addmodp(v4, v4, ZQ, p);
  mulmodp(v2, v2, v4, p);
  mulmodp(v1, v1, ZQ, p);
  submodp(v2, v2, v1, p);
  mulmodp(v2, v2, Zsum, p);
  submodp(YQ, v2, v3, p);
  mulmodp(v1, twob, YP, p);
  mulmodp(v1, v1, ZQ, p);
  mulmodp(v1, v1, Zsum, p);
  mulmodp(XQ, v1, XQ, p);
  mulmodp(ZQ, v1, ZQ, p);

  mpz_invert(ZQ, ZQ, p);
  mulmodp(XQ, XQ, ZQ, p);
  mulmodp(YQ, YQ, ZQ, p);
  mpz_set_ui(ZQ, 1);


}


void ladder(const uint8_t *m, const size_t bits, // m = scalar  
            const mpz_t x, const mpz_t y, 
            mpz_t x_out, mpz_t y_out,
            const mpz_t p, const mpz_t A, const mpz_t B)
{
  mpz_t twoa, twob, a24, inv4, tmpX0, tmpZ0, tmpX1, tmpZ1;
  mpz_t X0, Y0, Z0, X1, Y1, Z1, XP, YP, ZP;
  int mi, swap = 0;

  mpz_inits(twoa, twob, a24, inv4, tmpX0, tmpZ0, tmpX1, tmpZ1, NULL);
  mpz_inits(X0, Y0, Z0, X1, Y1, Z1, XP, YP, ZP, NULL);

  // a24 = (a + 2)/4 [p], twoa = 2A [p], twob = 2B [p]
  mpz_set_ui(inv4, 4);
  mpz_add_ui(a24, A, 2);
  mpz_invert(inv4, inv4, p);
  mulmodp(a24, a24, inv4, p);
  mpz_mul_2exp(twoa, A, 1);
  mpz_mod(twoa, twoa, p);
  mpz_mul_2exp(twob, B, 1);
  mpz_mod(twob, twob, p);

  // (XP, YP, ZP) = (x, y, 0)
  mpz_set(XP, x);
  mpz_set(YP, y);
  mpz_set_ui(ZP, 1);

  // (X0, Z0) = (1, 0)
  mpz_set_ui(X0, 1);

  // (X1 : Z1) = (XP, ZP)
  mpz_set(X1, XP);
  mpz_set(Z1, ZP);

  // i = bits ; invariant : (X0 : Z0) are the proj coords of [m >> i]P
  for(int i = bits - 1; i >= 0; i--)
  {
    mi = (m[i >> 3] >> (i & 7)) & 1;
    swap ^= mi;
    cswap(swap, X0, Z0, X1, Z1);
    swap = mi;
    
    // There are no tuples in C...
    // (X0 : Z0), (X1 : Z1) = [2](X0 : Z0), (X0 : Z0) + (X1 : Z1) 
    xDBL(X0, Z0, tmpX0, tmpZ0, p, a24);
    xADD(X0, Z0, X1, Z1, XP, ZP, tmpX1, tmpZ1, p);
    mpz_set(X0, tmpX0); mpz_set(Z0, tmpZ0); 
    mpz_set(X1, tmpX1); mpz_set(Z1, tmpZ1);
  }

  cswap(swap, X0, Z0, X1, Z1);

  // recover Y0 and adjust X0 & Z0 s.t. (X0 : Y0 : Z0) = [m](XP : YP : ZP)
  recover(XP, YP, X0, Y0, Z0, X1, Z1, p, twoa, twob);
  
  // (x_out, y_out) = (X0/Z0, Y0/Z0) 
  mpz_invert(x_out, Z0, p);
  mulmodp(x_out, x_out, X0, p);
  mpz_invert(y_out, Z0, p);
  mulmodp(y_out, y_out, Y0, p);
  
  mpz_clears(twoa, twob, a24, inv4, tmpX0, tmpZ0, tmpX1, tmpZ1, NULL);
  mpz_clears(X0, Y0, Z0, X1, Y1, Z1, XP, YP, ZP, NULL);
}


/*
int main(void)
{
  mpz_t p, A, B; 
  mpz_t x, y, X, Y;
  uint8_t m[32] = {0};

  mpz_inits(p, A, B, x, y, X, Y, NULL);
  mpz_set_ui(p, 1);
  mpz_mul_2exp(p, p, 255);
  mpz_sub_ui(p, p, 19);

  mpz_set_ui(A, 486662);
  mpz_set_ui(B, 1);

  mpz_set_ui(x, 9);
  mpz_set_str(y, "14781619447589544791020593568409986887264606134616475288964881837755586237401", 10);
  
  m[0] = 2;
  ladder(m, 255, x, y, X, Y, p, A, B);

  gmp_printf("%Zd %Zd\n", X, Y);

  return 0;
}*/
