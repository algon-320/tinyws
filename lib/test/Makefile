test.out: test.c deque.o
	gcc $^ -o $@

deque.o: ../deque.c
	gcc -c $^ -o $@

clean:
	rm *.out *.o