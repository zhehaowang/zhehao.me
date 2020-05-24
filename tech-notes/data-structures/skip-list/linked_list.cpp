#include "linked_list.h"

#include <sstream>

template <typename T>
LinkedList<T>::LinkedList(const LinkedList& rhs) {}

template <typename T>
LinkedList<T>::~LinkedList() {
    LinkedListNode<T>* temp = _head;
    while (temp) {
        LinkedListNode<T>* next = temp->next();
        delete temp;
        temp = next;
    }
}

template <typename T>
LinkedList<T>& LinkedList<T>::operator=(const LinkedList& rhs) { return *this; }

template <typename T>
typename LinkedList<T>::iterator LinkedList<T>::appendValue(T&& val) {
    LinkedListNode<T>* node = new LinkedListNode(std::forward<T>(val));
    if (!_head) {
        _head = node;
        _tail = node;
    } else {
        _tail->setNext(node);
        node->setPrev(_tail);
        _tail = node;
    }
    return LinkedListIterator<T>(node);
}

#ifdef DEBUG

template <typename T>
std::string LinkedList<T>::toString() const {
    std::stringstream stream;
    LinkedListNode<T>* temp = _head;
    
    while (temp) {
        stream << temp->toString() << " ";
        temp = temp->next();
    }
    return stream.str();
}

template <typename T>
std::string LinkedListNode<T>::toString() const {
    std::stringstream stream;
    stream << _value;
    return stream.str();
}

#endif

// @todo: how does others around this? having everything in header is the only way?
// does that slow compile time down?
template class LinkedList<int>;
