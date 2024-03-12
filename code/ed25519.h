#ifndef ED25519_H
#define ED25519_H

void generate_key(uint8_t prvkey[32], uint8_t pubkey[32]);

void sign(FILE *message, 
          uint8_t prvkey[32], uint8_t pubkey[32], 
          uint8_t signature[64]);
 
int verify(uint8_t signature[64], FILE *message, uint8_t pubkey[32]);

#endif
