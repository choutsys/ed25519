#include "utils.h"
#include <openssl/sha.h>
#include "edwards.h"
#include "ed25519.h"
 
#define KEY_BYTES       32
#define SCALAR_BYTES    32
#define FIELD_BYTES     32
#define ENCODING_BYTES  32
#define BPX       "15112221349535400772501151409588531511454012693041857206046113283949847762202"
#define BPY       "46316835694926478169428394003475163141307993866256225615783033603165251855960"
#define D         "37095705934669439343138083508754565189542113879843219016388785533085940283555"
#define LSTR      "7237005577332262213973186563042994240857116359379907606001950938285454250989"
#define P         "57896044618658097711785492504343953926634992332820282019728792003956564819949" 

static void encode_point(mpz_t x, mpz_t y, uint8_t enc[32])
{
  leint2buffer(y, enc, 31);
  enc[31] |= mpz_tstbit(x, 0) << 7;
}

static int decode_point(const uint8_t point[32], mpz_t x, mpz_t y)
{
  mpz_t p, pp3o8, twopm1o4, pm1o4, u, mu, v, d, vx2;
  int sqrt, sign;
  mpz_inits(p, pp3o8, twopm1o4, pm1o4, u, mu, v, d, vx2, NULL);
  mpz_set_str(p, P, 10);
  mpz_add_ui(pp3o8, p, 3);
  mpz_fdiv_q_2exp(pp3o8, pp3o8, 3);
  mpz_sub_ui(pm1o4, p, 1);
  mpz_fdiv_q_2exp(pm1o4, pm1o4, 2);
  mpz_set_ui(twopm1o4, 2);
  mpz_powm(twopm1o4, twopm1o4, pm1o4, p);
  mpz_set_str(d, D, 10);
  sign = point[31] >> 7;

  buffer2leint(point, y, 32);
  mpz_clrbit(y, 255);
  if(mpz_cmp(y, p) >= 0)
    return 0;

  mpz_powm_ui(u, y, 2, p);
  mulmodp(v, u, d, p);
  mpz_sub_ui(u, u, 1);
  mpz_neg(mu, u);
  mpz_add_ui(v, v, 1);
  
  mpz_invert(x, v, p);
  mulmodp(x, x, u, p);
  mpz_powm(x, x, pp3o8, p);

  mpz_powm_ui(vx2, x, 2, p);
  mulmodp(vx2, vx2, v, p);

  sqrt = (mpz_congruent_p(vx2, u, p) != 0) 
          + 2*(mpz_congruent_p(vx2, mu, p) != 0);
  switch(sqrt)
  {
    case 0:
    return 0;
    case 2:
    mulmodp(x, x, twopm1o4, p);
    break;
  }


  if(mpz_cmp_ui(x, 0) == 0 && sign == 1)
    return 0;

  if(mpz_tstbit(x, 0) != sign)
    mpz_sub(x, p, x);

  return 1;
}

static void iterate_basepoint(uint8_t scalar[32], 
                              uint8_t point[32])
{
  mpz_t bpx, bpy, a, d, x, y, p;

  mpz_inits(bpx, bpy, a, d, x, y, p, NULL);

  mpz_set_str(bpx, BPX, 10);
  mpz_set_str(bpy, BPY, 10);
  mpz_set_si(a, -1);
  mpz_set_str(d, D, 10);

  mpz_set_str(p, P, 10);

  scalar_mult(scalar, 256, bpx, bpy, x, y, p, a, d);
  encode_point(x, y, point);  
}
void generate_key(uint8_t prvkey[32], uint8_t pubkey[32])
{
  uint8_t xy[64], x[32];

  // generate and hash the private key
  get_randomness(prvkey, 32);
  SHA512(prvkey, 32, xy);

  // prune x 
  xy[0] &= 0xf8;
  xy[31] &= 0x7f;
  xy[31] |= 0x40; 
  // Perform [x]B 
  memcpy(x, xy, 32);
  iterate_basepoint(x, pubkey);
}

void sign(FILE *message, 
          uint8_t prvkey[32], uint8_t pubkey[32], 
          uint8_t signature[64])
{
  uint8_t sprefix[64], digest[64];
  uint8_t buffer[1337], rbuf[32];
  SHA512_CTX ctx1, ctx2;
  int bytesRead;
  mpz_t r, s, k, S, L;

  mpz_inits(r, s, k, S, L, NULL);
  mpz_set_str(L, LSTR, 10);
  
  SHA512(prvkey, 32, sprefix);
  sprefix[0] &= 0xf8;
  sprefix[31] &= 0x7f;
  sprefix[31] |= 0x40; 

 
  SHA512_Init(&ctx1);
  SHA512_Update(&ctx1, &sprefix[32], 32);
  while((bytesRead = fread(buffer, 1, 1337, message)) > 0)
    SHA512_Update(&ctx1, buffer, bytesRead);
  SHA512_Final(digest, &ctx1);

  buffer2leint(digest, r, 64);
  mpz_mod(r, r, L);
  leint2buffer(r, rbuf, 32);
  iterate_basepoint(rbuf, signature);

  rewind(message);

  SHA512_Init(&ctx2);
  SHA512_Update(&ctx2, signature, 32);
  SHA512_Update(&ctx2, pubkey, 32);
  while((bytesRead = fread(buffer, 1, 1337, message)) > 0)
    SHA512_Update(&ctx2, buffer, bytesRead);
  SHA512_Final(digest, &ctx2);
 
  buffer2leint(digest, k, 64);
  buffer2leint(sprefix, s, 32);
  mpz_mod(k, k, L);
  mulmodp(S, k, s, L);
  addmodp(S, S, r, L);

  leint2buffer(S, &signature[32], 32);
  mpz_clears(r, s, k, S, L, NULL);
}

void add_points(mpz_t xp, mpz_t yp, 
                mpz_t xq, mpz_t yq, 
                mpz_t x_out, mpz_t y_out, 
                mpz_t d, mpz_t p)
{
  mpz_t s, ss, m, mm, x, y;

  mpz_inits(s, ss, m, mm, x, y, NULL);

  mulmodp(s, xp, xq, p);
  mulmodp(ss, yp, yq, p);
  mulmodp(m, xp, yq, p);
  mulmodp(mm, yp, xq, p);
  addmodp(m, m, mm, p);

  mulmodp(x, s, ss, p);
  mulmodp(x, x, d, p);
  mpz_set_ui(y, 1);
  submodp(y, y, x, p);
  mpz_add_ui(x, x, 1);
  mpz_invert(x, x, p);
  mpz_invert(y, y, p);
  addmodp(s, s, ss, p);
  mulmodp(x, x, m, p);
  mulmodp(y, y, s, p);
  mpz_set(y_out, y);
  mpz_set(x_out, x);
}

int verify(uint8_t signature[64], FILE *message, uint8_t pubkey[32])
{
  mpz_t Rx, Ry, S, Ax, Ay, L, k, lhsx, lhsy, rhsx, rhsy, bpx, bpy, d, a, p;
  uint8_t firsthalf[32], secondhalf[32], buffer[1337], digest[64];
  uint8_t kbuf[32];
  int bytesRead;
  SHA512_CTX ctx;

  memcpy(firsthalf, signature, 32);
  memcpy(secondhalf, &signature[32], 32);
  mpz_inits(Rx, Ry, S, Ax, Ay, L, k, lhsx, lhsy, rhsx, rhsy, bpx, bpy, d, a, p, NULL);
  mpz_set_str(L, LSTR, 10);
  mpz_set_str(d, D, 10);
  mpz_set_si(a, -1);
  mpz_set_str(bpx, BPX, 10);
  mpz_set_str(bpy, BPY, 10);
  mpz_set_str(p, P, 10);

  buffer2leint(secondhalf, S, 32);
  if(!decode_point(firsthalf, Rx, Ry) || 
     !decode_point(pubkey, Ax, Ay)    ||
      mpz_cmp(S, L) >= 0)
    return 0;

  SHA512_Init(&ctx);
  SHA512_Update(&ctx, firsthalf, 32);
  SHA512_Update(&ctx, pubkey, 32);
  while((bytesRead = fread(buffer, 1, 1337, message)) > 0)
    SHA512_Update(&ctx, buffer, bytesRead);
  SHA512_Final(digest, &ctx);
 
  buffer2leint(digest, k, 64);
  mpz_mod(k, k, L);
  leint2buffer(k, kbuf, 32);
  scalar_mult(secondhalf, 256, bpx, bpy, lhsx, lhsy, p, a, d);
  scalar_mult(kbuf, 256, Ax, Ay, rhsx, rhsy, p, a, d);
  add_points(Rx, Ry, rhsx, rhsy, rhsx, rhsy, d, p);
  if(mpz_cmp(rhsx, lhsx) != 0 || mpz_cmp(rhsy, lhsy) != 0)
    return 0;

  return 1;
}
/*
int main(void)
{
  FILE *message = fopen("vide", "rb");
  uint8_t prvkey[32], pubkey[32], signature[64];

  hexstring2buffer("f5e5767cf153319517630f226876b86c"
   "8160cc583bc013744c6bf255f5cc0ee5", prvkey, 32);
  hexstring2buffer("278117fc144c72340f67d0f2316e8386"
   "ceffbf2b2428c9c51fef7c597f1d426e", pubkey, 32);
  hexstring2buffer("0aab4c900501b3e24d7cdf4663326a3a"
   "87df5e4843b2cbdb67cbf6e460fec350"
   "aa5371b1508f9f4528ecea23c436d94b"
   "5e8fcd4f681e30a6ac00a9704a188a03"
 , signature, 64);

//sign(message, prvkey, pubkey, signature);

 //buffer2hexstream(signature, stdout, 64);
 if(verify(signature, message, pubkey))
    puts("good");
  

  printf("\n");

} */
