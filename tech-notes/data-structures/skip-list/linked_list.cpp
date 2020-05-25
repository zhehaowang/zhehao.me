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

template <typename T>
typename LinkedList<T>::iterator
LinkedList<T>::insertAfter(T&& val, iterator iter) {
    LinkedListNode<T>* node = new LinkedListNode(std::forward<T>(val));
    auto nextNode = iter.get()->next();

    if (nextNode) {
        node->setNext(nextNode);
        nextNode->setPrev(node);
    } else {
        _tail = node;
    }
    iter.get()->setNext(node);
    node->setPrev(iter.get());

    return LinkedListIterator<T>(node);
}

template <typename T>
typename LinkedList<T>::size_type LinkedList<T>::size() const {
    auto temp = _head;
    size_type total = 0;    
    while (temp) {
        total += 1;
        temp = temp->next();
    }
    return total;
}

#ifdef DEBUG

template <typename T>
std::string LinkedList<T>::toString() const {
    std::stringstream stream;
    auto temp = _head;
    
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
