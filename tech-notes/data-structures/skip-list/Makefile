linked_list.test: linked_list.t.cpp linked_list.a
	g++ -o linked_list.test -std=gnu++17 linked_list.t.cpp linked_list.a -lgtest

skip_list.test: skip_list.t.cpp skip_list.a
	g++ -o skip_list.test -std=gnu++17 skip_list.t.cpp skip_list.a -lgtest

linked_list.a: linked_list.cpp linked_list.h
	g++ -c -std=gnu++17 linked_list.cpp
	ar rvs linked_list.a linked_list.o

skip_list.a: skip_list.cpp skip_list.h linked_list.cpp linked_list.h
	g++ -c -std=gnu++17 linked_list.cpp skip_list.cpp
	ar rvs skip_list.a linked_list.o skip_list.o

clean:
	rm -rf linked_list.o linked_list.a linked_list.test
