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

    T get() const { return _value; }

#ifdef DEBUG
    std::string toString() const;
#endif

  private:
    T _value;
    LinkedListNode* _next = nullptr;
    LinkedListNode* _prev = nullptr;
};

template <typename T>
class LinkedListIterator;

template <typename T>
struct DefaultComparator {
    bool operator()(const T& lhs, const T& rhs) const {
        return lhs < rhs;
    }
};

template <typename T>
class LinkedList {
  public:
    using iterator = LinkedListIterator<T>;
    using size_type = uint64_t;

    LinkedList() = default;
    ~LinkedList();

    LinkedList(const LinkedList& rhs);
    LinkedList& operator=(const LinkedList& rhs);

    iterator appendValue(T&& val);

    template <typename Comparator = DefaultComparator<T>>
    iterator insertValueOrdered(T&& val, Comparator comp = DefaultComparator<T>());

    iterator insertAfter(T&& val, iterator iter);

    // void append(LinkedListNode<T> node);

    iterator begin() const;
    // non-standard
    iterator end() const;

    bool empty() const { return _head == nullptr; }
    size_type size() const;

#ifdef DEBUG
    std::string toString() const;
#endif

  private:
    LinkedListNode<T>* _head = nullptr;
    LinkedListNode<T>* _tail = nullptr;
};

// bidirectional iterator
template <typename T>
class LinkedListIterator {
  public:
    LinkedListIterator(LinkedListNode<T>* data) : _data(data) {}
    
    LinkedListIterator& operator=(const LinkedListIterator& rhs) = default;
    LinkedListIterator(const LinkedListIterator& rhs) = default;
    
    LinkedListIterator& operator=(LinkedListIterator&& rhs) = default;
    LinkedListIterator(LinkedListIterator&& rhs) = default;

    bool hasNext() const { return _data->next() != nullptr; }
    void next() { _data = _data->next(); }

    bool hasPrev() const { return _data->prev() != nullptr; }
    void prev() { _data = _data->prev(); }

    bool operator==(LinkedListIterator<T> rhs) const { return _data == rhs._data; }

    const LinkedListNode<T>* get() const { return _data; }
    LinkedListNode<T>* get() { return _data; }
    
    T operator->() const { return _data->get(); }
    T operator*()  const { return _data->get(); }
  private:
    LinkedListNode<T>* _data;
};

template <typename T>
typename LinkedList<T>::iterator LinkedList<T>::begin() const {
    return LinkedListIterator<T>(_head);
}

template <typename T>
typename LinkedList<T>::iterator LinkedList<T>::end() const {
    return LinkedListIterator<T>(_tail);
}

template <typename T>
template <typename Comparator>
typename LinkedList<T>::iterator
LinkedList<T>::insertValueOrdered(T&& val, Comparator comp) {
    auto temp = _head;
    while (temp) {
        if (!comp(temp->get(), val)) {
            if (temp->prev()) {
                return insertAfter(
                    std::forward<T>(val), LinkedListIterator<T>(temp->prev()));
            } else {
                LinkedListNode<T>* node = new LinkedListNode(
                    std::forward<T>(val));
                _head = node;
                temp->setPrev(node);
                node->setNext(temp);
                return LinkedListIterator<T>(node);
            }
        }
        temp = temp->next();
    }

    if (_tail) {
        return insertAfter(std::forward<T>(val), LinkedListIterator<T>(_tail));
    } else {
        return appendValue(std::forward<T>(val));
    }
}

#endif