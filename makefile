OBJECTS = array_lookup.o radixtree.o

all: radixtrie

radixtrie: radixtrie.o array_lookup.o
	gcc radixtrie.o array_lookup.o -o radixtrie

radixtrie.o: radixtrie.c
	gcc -fnested-functions  -g -O3  -c radixtrie.c

clean:
	rm -f $(OBJECTS) radixtrie
