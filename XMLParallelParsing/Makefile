xml : main.o step1.o step2.o step3.o
	gcc main.o step1.o step2.o step3.o -o xml -lpthread
main.o  : main.c myXml.h
	gcc -c main.c
step1.o : step1.c myXml.h
	gcc -c step1.c
step2.o : step2.c myXml.h
	gcc -c step2.c
step3.o : step3.c myXml.h
	gcc -c step3.c
