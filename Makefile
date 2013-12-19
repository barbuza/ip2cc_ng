CC = cc
DEPS = ip2cc.h
OBJ = ip2cc.o test.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $<

test: $(OBJ)
	$(CC) -o $@ $^ -lcunit
	./test

clean:
	rm -f *.o test
