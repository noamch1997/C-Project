FLAGS = -ansi -Wall -pedantic
GLOBALS = globals.h
EXE = assembler.o code.o fpass.o spass.o instructions.o table.o utils.o writefiles.o

assembler: $(EXE) $(GLOBSALS)
	gcc -g $(EXE) $(FLAGS) -o $@

assembler.o: assembler.c $(GLOBSALS)
	gcc -c assembler.c $(FLAGS) -o $@

code.o: code.c code.h $(GLOBSALS)
	gcc -c code.c $(FLAGS) -o $@

fpass.o: first_pass.c first_pass.h $(GLOBSALS)
	gcc -c first_pass.c $(FLAGS) -o $@

spass.o: second_pass.c second_pass.h $(GLOBSALS)
	gcc -c second_pass.c $(FLAGS) -o $@

instructions.o: instructions.c instructions.h $(GLOBSALS)
	gcc -c instructions.c $(FLAGS) -o $@

table.o: table.c table.h $(GLOBSALS)
	gcc -c table.c $(FLAGS) -o $@

utils.o: utils.c instructions.h $(GLOBSALS)
	gcc -c utils.c $(FLAGS) -o $@

writefiles.o: writefiles.c writefiles.h $(GLOBSALS)
	gcc -c writefiles.c $(FLAGS) -o $@

clean:
	rm -rf *.o
