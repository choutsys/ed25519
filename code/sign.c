#include "utils.h"
#include "ed25519.h"

int main(int argc, char *argv[])
{
  FILE *pub, *priv, *sig, *data;
  char *pubfilename, *privfilename;
  uint8_t pubkey[32], prvkey[32], signature[64];

  if(argc != 4)
  {
    puts("Bad usage. Read README file.");
    exit(EXIT_FAILURE);
  }
  pubfilename = malloc(strlen(argv[1]) + 4);
  privfilename = malloc(strlen(argv[1]) + 4);
  strcpy(pubfilename, argv[1]);
  strcpy(privfilename, argv[1]);
  strcat(pubfilename, ".pk");
  strcat(privfilename, ".sk");
  if(!(pub = fopen(pubfilename, "rb")) || 
     !(priv = fopen(privfilename, "rb")))
  {
    puts("Unable to read key files.");
    exit(EXIT_FAILURE);
  }
  if(!(data = fopen(argv[2], "rb")))
  {
    puts("Unable to read data file.");
    exit(EXIT_FAILURE);
  }
  if(!(sig = fopen(argv[3], "wb")))
  {
    puts("Unable to create signature file.");
    exit(EXIT_FAILURE);
  }
  
  if(fread(prvkey, 1, 32, priv) < 32 || fgetc(priv) != EOF)
  {
    puts("Invalid data in secret key file.");
    exit(EXIT_FAILURE);
  }  
 if(fread(pubkey, 1, 32, pub) < 32 || fgetc(pub) != EOF)
  {
    puts("Invalid data in public key file.");
    exit(EXIT_FAILURE);
  } 


  sign(data, prvkey, pubkey, signature);
  fwrite(signature, 1, 64, sig);

  fclose(priv);
  fclose(pub);
  fclose(data);
  fclose(sig);
  free(pubfilename);
  free(privfilename);
  return EXIT_SUCCESS;
}
