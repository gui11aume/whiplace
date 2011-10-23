obj = dynstring.o whiplace.o
whiplace: $(obj)
	gcc -o whiplace dynstring.c whiplace.c
dynstring.o:
whiplace.o:
clean:
	rm $(obj)
