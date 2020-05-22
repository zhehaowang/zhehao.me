#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#define DEBUG

#ifdef DEBUG
#include <string>
#endif

template <typename T>
class LinkedListNode {
  public:
    LinkedListNode() = default;
    // @todo: is providing a universal reference interface only good practice?
    LinkedListNode(T&& val) : _value(val) {}

    ~LinkedListNode() = default;

    LinkedListNode(const LinkedListNode&) = default;
    LinkedListNode& operator=(const LinkedListNode&) = default;
    
    LinkedListNode(LinkedListNode&&) = default;
    LinkedListNode& operator=(LinkedListNode&&) = default;

    LinkedListNode* next() const { return _next; }
    LinkedListNode* prev() const { return _prev; }

    void setNext(LinkedListNode* next) { _next = next; }
    void setPrev(LinkedListNode* prev) { _prev = prev; }

#ifdef DEBUG
    std::string toString() const;
#endif

  private:
    T _value;
    LinkedListNode* _next = nullptr;
    LinkedListNode* _prev = nullptr;
};

template <typename T>
class LinkedList {
  public:
    LinkedList() = default;
    ~LinkedList();

    LinkedList(const LinkedList& rhs);
    LinkedList& operator=(const LinkedList& rhs);

    // @todo: implement iterator
    void appendValue(T&& val);

    // void append(LinkedListNode<T> node);

#ifdef DEBUG
    std::string toString() const;
#endif

  private:
    LinkedListNode<T>* _head = nullptr;
    LinkedListNode<T>* _tail = nullptr;
};

#endif