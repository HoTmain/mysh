OBJS= mysh.o mysh_modl.o
EXEC= mysh
HEADER= myshheadr.h

$(EXEC): $(OBJS) $(HEADER)
	gcc $(OBJS) -o $(EXEC)


clean:
	rm -f $(OBJS) $(EXEC)