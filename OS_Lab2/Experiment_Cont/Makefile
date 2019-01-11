lab : main.c myPrint.o
	gcc -o lab main.c myPrint.o
myPrint.o : myPrint.asm
	nasm -f elf -o myPrint.o myPrint.asm
clean :
	rm lab main.o myPrint.o
