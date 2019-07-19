test.out: test.c deque.o linked_list.o
	gcc $^ -o $@

deque.o: ../deque.c
	gcc -c $^ -o $@

linked_list.o: ../linked_list.c
	gcc -c $^ -o $@

clean:
	rm *.out *.o