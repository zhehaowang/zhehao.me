linked_list.test: linked_list.t.cpp
	g++ -o linked_list.test -std=gnu++17 linked_list.t.cpp linked_list.a -lgtest

linked_list.a: linked_list.cpp linked_list.h
	g++ -c -std=gnu++17 linked_list.cpp
	ar rvs linked_list.a linked_list.o
