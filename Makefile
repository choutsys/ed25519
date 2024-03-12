SRCF    = ed25519.c edwards.c montgomery.c utils.c 

CODE    = cd code
CC      = gcc

.SILENT:
.PHONY:     default
default:    all	

.PHONY:			all
all: keygen sign verify
	 

.PHONY:     keygen
keygen: clean 
	@$(CODE) && $(CC) -Wno-deprecated-declarations $(SRCF) keygen.c -lgmp -lcrypto -o ../keygen 
	@echo "Built keygen."

.PHONY:    sign
sign: clean
	@$(CODE) && $(CC) -Wno-deprecated-declarations $(SRCF) sign.c -lgmp -lcrypto -o ../sign 
	@echo "Built sign executable."


.PHONY:   verify
verify: clean
	@$(CODE) && $(CC) -Wno-deprecated-declarations $(SRCF) verify.c -lgmp -lcrypto -o ../verify
	@echo "Built verify executable."





.PHONY:     clean
clean:  
	@$(RM) keygen sign verify code/*.o
	@echo "Done cleaning up."


