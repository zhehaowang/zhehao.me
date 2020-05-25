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
void LinkedList<T>::removeValue(T&& val) {
    if (empty()) {
        return;
    }

    auto it = begin();
    while (it != end()) {
        if (*it == val) {
            it = removeAt(it);
        } else {
            it.next();
        }
    }
    return;
}

template <typename T>
typename LinkedList<T>::iterator LinkedList<T>::removeAt(iterator it) {
    auto node = it.get();
    auto nextNode = node->next();
    auto prevNode = node->prev();

    if (node == _tail) {
        _tail = node->prev();
    } else {
        nextNode->setPrev(prevNode);
    }
    if (node == _head) {
        _head = nextNode;
    } else {
        prevNode->setNext(nextNode);
    }
    
    delete node;
    return LinkedList<T>::iterator(nextNode);
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

template <typename T>
typename LinkedList<T>::iterator LinkedList<T>::find(T&& val) const {
    auto temp = _head;
    while (temp) {
        if (temp->get() == val) {
            break;
        }
        temp = temp->next();
    }
    return LinkedListIterator<T>(temp);
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
