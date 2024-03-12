#include "utils.h"
#include "ed25519.h"

int main(int argc, char *argv[])
{
  FILE *pub, *priv;
  char *pubfilename, *privfilename;
  uint8_t pubkey[32], prvkey[32];

  if(argc != 2)
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
  if(!(pub = fopen(pubfilename, "wb")) || 
     !(priv = fopen(privfilename, "wb")))
  {
    puts("Unable to create key files.");
    exit(EXIT_FAILURE);
  }

  generate_key(prvkey, pubkey);
  fwrite(prvkey, 1, 32, priv);
  fwrite(pubkey, 1, 32, pub);

  fclose(priv);
  fclose(pub);
  free(pubfilename);
  free(privfilename);
  return EXIT_SUCCESS;
}
