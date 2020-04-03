prom = icmpv6
lib = ../lib
obj = main.o $(lib)/sock_basis.o $(lib)/icmpv6_basis.o
hdr = $(lib)/sock_basis.h $(lib)/icmpv6_basis.h

$(prom): $(obj)
	gcc -o $(prom) $(obj) -g -lpthread

main.o: main.c $(hdr) 
	gcc -c main.c -I$(lib) -g

$(lib)/sock_basis.o: $(lib)/sock_basis.c $(lib)/sock_basis.h
	gcc -c $(lib)/sock_basis.c -o $(lib)/sock_basis.o -g

$(lib)/icmpv6_basis.o: $(lib)/icmpv6_basis.c $(hdr)
	gcc -c $(lib)/icmpv6_basis.c -o $(lib)/icmpv6_basis.o -g -lpthread

.PHONY : clean
clean: 
	rm $(prom) $(obj) test.o test
