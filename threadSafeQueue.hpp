#pragma once

#include <pthread.h>

namespace multicore {

/**
 * @author Chenyang Tang <ct1856@nyu.edu>
 *
 * @section DESCRIPTION
 *
 * A thread-safe queue class template. T is the type of element in the queue.
 */
template <typename T>
class ThreadSafeQueue {
  public:
    /**
     * Constructor. Makes an empty queue.
     */
    ThreadSafeQueue() {
        pthread_mutex_init(&enqueue_lock, nullptr);
        pthread_mutex_init(&dequeue_lock, nullptr);
        pthread_mutex_init(&cond_lock, nullptr);
        pthread_cond_init(&cond_v, nullptr);
        head = tail = new Node; // sentinel node that head always points to
    }

    /**
     * Destructor. Will delete all nodes.
     */
    ~ThreadSafeQueue() {
        // Delete all data nodes
        if (head != tail) {
            Node *tmp = head->next;
            Node *tmp2;
            while (tmp != tail) {
                tmp2 = tmp;
                tmp = tmp->next;
                delete tmp2;
            }
        }
        // Delete sentinel
        delete head;
        pthread_mutex_destroy(&enqueue_lock);
        pthread_mutex_destroy(&dequeue_lock);
        pthread_mutex_destroy(&cond_lock);
        pthread_cond_destroy(&cond_v);
    }

    /**
     * Checks if the queue is empty.
     *
     * @return true if queue is empty;
     *         false if queue is not empty.
     */
    inline bool empty() const {
        return head == tail;
    }

    /**
     * Enqueue an element.
     *
     * @param elem the element to be enqueued.
     */
    void enqueue(const T& elem) {
        pthread_mutex_lock(&enqueue_lock);
        tail->next = new Node(elem);
        pthread_mutex_lock(&cond_lock);
        tail = tail->next;
        pthread_cond_signal(&cond_v);
        pthread_mutex_unlock(&cond_lock);
        pthread_mutex_unlock(&enqueue_lock);
    }

    /**
     * Dequeue an element. If the queue is empty, wait until there is an element available and then resume.
     *
     * @return the dequeued element.
     */
    T dequeue() {
        pthread_mutex_lock(&dequeue_lock);
        pthread_mutex_lock(&cond_lock);
        while (empty()) {
            pthread_cond_wait(&cond_v, &cond_lock);
        }
        pthread_mutex_unlock(&cond_lock);
        Node *tmp = head;
        head = head->next;
        delete tmp;
        T ret = head->data;
        pthread_mutex_unlock(&dequeue_lock);
        return ret;
    }

  private:
    struct Node {
        T data;
        Node *next;

        Node() {}
        Node(T elem): data(elem), next(nullptr) {}
    };

    Node *head, *tail;
    pthread_mutex_t enqueue_lock, dequeue_lock, cond_lock;
    pthread_cond_t cond_v;
};

} // namespace multicore
