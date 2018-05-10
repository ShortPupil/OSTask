out: main.c   my_print.o
	gcc -m64 -o out main.c my_print.o
my_print.o:  my_print.asm
	nasm -f elf64 -o my_print.o my_print.asm
clean:
	rm my_print.o out
