#include "utils.h"
#include "montgomery.h"

void edparam2montparam(const mpz_t a, const mpz_t d,
                       mpz_t A, mpz_t B, const mpz_t p)
{
  mpz_t tmp;
  mpz_init(tmp);

  // A = 2(a + d)/(a - d) [p]
  submodp(A, a, d, p);
  mpz_invert(A, A, p);
  mpz_set(B, A);
  addmodp(tmp, a, d, p);
  mpz_mul_2exp(tmp, tmp, 1);
  mulmodp(A, A, tmp, p);

  // B = 4/(a - d) [p]
  mpz_mul_2exp(B, B, 2);
  mpz_mod(B, B, p);
  
  mpz_clear(tmp);
}


void ed2mont(const mpz_t edx, const mpz_t edy, 
             mpz_t montx, mpz_t monty, const mpz_t p)
{
  mpz_t tmp;
  
  mpz_init(tmp);

  // montx = (edx + 1)/(edx - 1) [p]
  mpz_set_ui(tmp, 1);
  submodp(montx, tmp, edy, p);
  mpz_invert(montx, montx, p);
  addmodp(tmp, tmp, edy, p);
  mulmodp(montx, montx, tmp, p);
  
  // monty = montx/edx [p]
  mpz_invert(monty, edx, p);
  mulmodp(monty, monty, montx, p);

  mpz_clear(tmp);
}

void mont2ed(const mpz_t montx, const mpz_t monty,
             mpz_t edx, mpz_t edy, const mpz_t p)
{
  mpz_t tmp;

  mpz_init(tmp);
  
  // edx = montx/monty [p]
  mpz_invert(edx, monty, p);
  mulmodp(edx, montx, edx, p);
  
  // edy = (montx - 1)/(montx + 1) [p]
  mpz_set_ui(tmp, 1);
  addmodp(edy, montx, tmp, p);
  mpz_invert(edy, edy, p);
  submodp(tmp, montx, tmp, p);
  mulmodp(edy, edy, tmp, p);

  mpz_clear(tmp);
}

void scalar_mult(uint8_t *m, const size_t bits,
                 const mpz_t x, const mpz_t y,
                 mpz_t x_out, mpz_t y_out,
                 const mpz_t p, const mpz_t a, const mpz_t d)
{
  mpz_t montx, monty, montx_out, monty_out;
  mpz_t A, B; 
  
  mpz_inits(montx, monty, montx_out, monty_out, NULL);
  mpz_inits(A, B, NULL);

  edparam2montparam(a, d, A, B, p);

  ed2mont(x, y, montx, monty, p);
  ladder(m, bits, montx, monty, montx_out, monty_out, p, A, B);
  mont2ed(montx_out, monty_out, x_out, y_out, p);
}

/*
int main(void)
{
  mpz_t p, a, d; 
  mpz_t x, y, X, Y;
  uint8_t m[32] = {0};

  mpz_inits(p, a, d, x, y, X, Y, NULL);
  mpz_set_ui(p, 1);
  mpz_mul_2exp(p, p, 255);
  mpz_sub_ui(p, p, 19);

  mpz_set_si(a, -1);
  mpz_set_str(d, "37095705934669439343138083508754565189542113879843219016388785533085940283555", 10);

  mpz_set_str(x, "15112221349535400772501151409588531511454012693041857206046113283949847762202", 10);
  mpz_set_str(y, "46316835694926478169428394003475163141307993866256225615783033603165251855960", 10);
  
  hexstring2buffer("12581e70a192aeb9ac1411b36d11fc06393db55998190491c063807a6b4d730d", m, 32);
  scalar_mult(m, 255, x, y, X, Y, p, a, d);

  return 0;
}*/
