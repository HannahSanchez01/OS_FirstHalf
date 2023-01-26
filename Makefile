all:  bytecheck 


bytecheck: bytecheck.c
	gcc -Wall -Werror -Wconversion -o bytecheck bytecheck.c

clean:
	rm -f bytecheck
