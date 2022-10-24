CC = gcc
CFLAGS = -I utils/includes
LDLIBS = -pthread

MAIN = farm

all: $(MAIN)

$(MAIN): $(MAIN).o boundedqueue.o utilities.o

clean:
	rm -f $(MAIN) *.o *.dat

test:
	make
	chmod +x ./test.sh
	./test.sh
	make clean

info:
	@echo "Usa -make test per compilare, eseguire il test, e pulire i file generati"
	@echo "Usa -make per compilare il file ed ottenere l'eseguibile -farm"
	@echo "Usa -make clean per eliminare tutti i file generati"
