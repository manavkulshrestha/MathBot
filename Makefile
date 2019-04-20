SRC := $(shell ls *.c)
OBJ := $(subst .c,.o,$(SRC))

PROG := client

$(PROG): $(OBJ) $(HEADERS)
		gcc $(OBJ) $(HEADERS) -o $(PROG)

%.o: %.c
		gcc -c $< -o $@

clean:
		rm *.o $(PROG)