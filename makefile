obj = dynstring.o whiplace.o
whiplace: $(obj)
	cc -o whiplace dynstring.c whiplace.c
dynstring.o:
whiplace.o:
clean:
	rm $(obj) whiplace
