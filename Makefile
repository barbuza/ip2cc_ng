CC = gcc
OBJ = ip2cc.o test.o

%.o: %.c
	$(CC) -c -o $@ $<

make_test: $(OBJ)
	$(CC) -o test $^ -lcunit

test: make_test
	./test

clean:
	rm -f test $(OBJ)
