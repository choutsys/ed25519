#include "utils.h"
#include "ed25519.h"

int main(int argc, char *argv[])
{
  FILE *pub, *sig, *data;
  uint8_t pubkey[32], signature[64];

  if(argc != 4)
  {
    puts("Bad usage. Read README file.");
    exit(EXIT_FAILURE);
  }
  if(!(pub = fopen(argv[1], "rb")))
  {
    puts("Unable to read the public key.");
    exit(EXIT_FAILURE);
  }
  if(!(data = fopen(argv[2], "rb")))
  {
    puts("Unable to read data file.");
    exit(EXIT_FAILURE);
  }
  if(!(sig = fopen(argv[3], "rb")))
  {
    puts("Unable to read signature file.");
    exit(EXIT_FAILURE);
  }
  
  if(fread(pubkey, 1, 32, pub) < 32 || fgetc(pub) != EOF)
  {
    puts("Invalid data in public key file.");
    exit(EXIT_FAILURE);
  }  

  if(fread(signature, 1, 64, sig) < 64 || fgetc(sig) != EOF)
  {
    puts("Invalid data in signature file.");
    exit(EXIT_FAILURE);
  }  

  if(verify(signature, data, pubkey))
    puts("ACCEPT");
  else
    puts("REJECT");

  fclose(pub);
  fclose(data);
  fclose(sig);
  return EXIT_SUCCESS;
}
