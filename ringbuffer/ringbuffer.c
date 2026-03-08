#include "ringbuffer.h"

#define MAX_SIZE 4096

static volatile size_t head = 0;
static volatile  size_t tail = 0;
static size_t size = 0;
static volatile bool empty = true;

static int16_t buffer[MAX_SIZE];

bool ringbuffer_init(size_t wanted_size) {

    if (wanted_size > MAX_SIZE || (wanted_size & (wanted_size - 1)) != 0) { // Check for power of 2.
        return false;
    }

    head = 0;
    tail = 0;
    size = wanted_size;
    empty = true;
    return true;
}

bool ringbuffer_is_empty(void) {
    return empty;
}

bool ringbuffer_is_full(void) {
    return head == tail && !empty;
}

bool ringbuffer_push(int16_t pushed_data) {

    if (ringbuffer_is_full()) {
        return false;
    }

    if (empty) {
        empty = false;
    }

    buffer[head] = pushed_data;
    head = (head + 1) & (size - 1);
    return true;
}

bool ringbuffer_pop(int16_t *popped_data) {

    if (ringbuffer_is_empty()) {
        return false;
    }

    size_t next_index = (tail + 1) & (size - 1);

    if (next_index == head) {
        empty = true;
    }

    *popped_data = buffer[tail];
    tail = next_index;
    return true;
}