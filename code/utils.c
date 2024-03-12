#include "utils.h"

int is_hexstring(const char *hexstring, const size_t nbytes)
{
  size_t len;

  len = strlen(hexstring);

  if(len != nbytes*2)
    return 0;

  for(size_t i = 0; i < len; i++)
  {
    if((hexstring[i] < '0') || 
      (hexstring[i] > '9' && hexstring[i] < 'A') ||
      (hexstring[i] > 'F' && hexstring[i] < 'a') ||
      (hexstring[i] > 'f'))
      return 0;
  }

  return 1;
}

void hexstring2buffer(const char *hexstr, uint8_t *buf, 
                      const size_t nbytes)
{
  for(size_t i = 0; i < nbytes; i++)
    sscanf(&hexstr[2*i], "%02hhx", &buf[i]);
}

void buffer2leint(const uint8_t *buffer, mpz_t leint, 
                  const size_t nbytes)
{
  mpz_import(leint, nbytes, -1, 1, 0, 0, buffer);
}

void leint2buffer(const mpz_t leint, uint8_t *buffer, 
                  const size_t nbytes)
{
  memset(buffer, 0, nbytes);
  mpz_export(buffer, NULL, -1, 1, 0, 0, leint);
}

void buffer2hexstream(const uint8_t *buf, FILE *stream, 
                      const size_t nbytes)
{
  for(size_t i = 0; i < nbytes; i++)
    fprintf(stream, "%02hhx", buf[i]);
}

void addmodp(mpz_t rop, const mpz_t op1, const mpz_t op2, 
                    const mpz_t p)
{
  mpz_add(rop, op1, op2);
  mpz_mod(rop, rop, p);
}

void submodp(mpz_t rop, const mpz_t op1, const mpz_t op2, 
                    const mpz_t p)
{
  mpz_sub(rop, op1, op2);
  mpz_mod(rop, rop, p);
}

void mulmodp(mpz_t rop, const mpz_t op1, const mpz_t op2, 
                    const mpz_t p)
{
  mpz_mul(rop, op1, op2);
  mpz_mod(rop, rop, p);
}

void get_randomness(uint8_t *buffer, size_t nbytes)
{
  FILE *randomfile;
  
  randomfile = fopen("/dev/urandom", "rb");
  fread(buffer, 1, nbytes, randomfile);
  fclose(randomfile);
}
