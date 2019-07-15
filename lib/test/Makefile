test.out: test.c deque.o
	gcc test.c deque.o -o test.out

deque.o: ../deque.c
	gcc -c ../deque.c -o deque.o

clean:
	rm *.out *.o