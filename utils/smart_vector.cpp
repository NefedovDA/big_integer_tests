#include <cassert>
#include <cstring>
#include "smart_vector.h"

smart_vector::smart_data::smart_data(size_t new_capacity) :
        capacity(new_capacity),
        data(new uint32_t[new_capacity]) {
    std::memset(data, 0, sizeof(uint32_t) * capacity);
}

smart_vector::smart_data::smart_data(const smart_vector::smart_data &other) :
        capacity(other.capacity),
        data(new uint32_t[other.capacity]) {
    std::memcpy(data, other.data, sizeof(uint32_t) * capacity);
}

smart_vector::smart_data::smart_data(const smart_vector::smart_data &other, size_t new_capacity) :
        capacity(new_capacity),
        data(new uint32_t[new_capacity]) {
    size_t real_size = std::min(capacity, other.capacity);
    std::memset(data + real_size, 0, sizeof(uint32_t) * (capacity - real_size));
    std::memcpy(data, other.data, sizeof(uint32_t) * real_size);
}

smart_vector::smart_data *smart_vector::smart_data::hy() {
    ++count_of_owners;
    return this;
}

void smart_vector::smart_data::by() {
    --count_of_owners;
    if (count_of_owners == 0) {
        delete this;
    }
}

smart_vector::smart_data::~smart_data() {
    delete[] data;
}

smart_vector::smart_vector() :
        length(0) {}

smart_vector::smart_vector(size_t size) :
        length(size) {
    if (length > 1) {
        big_object = new smart_data(size);
    }
}

smart_vector::smart_vector(smart_vector const &other) noexcept :
        length(other.length) {
    if (length > 1) {
        big_object = other.big_object->hy();
    } else {
        little_object = other.little_object;
    }
}

smart_vector &smart_vector::operator=(smart_vector const &other) noexcept {
    if (this == &other) {
        return *this;
    }
    if (length > 1) {
        big_object->by();
    }

    length = other.length;

    if (length > 1) {
        big_object = other.big_object->hy();
    } else {
        little_object = other.little_object;
    }
    return *this;
}

smart_vector::~smart_vector() {
    if (length > 1) {
        big_object->by();
    }
}

size_t smart_vector::size() const {
    return length;
}

bool smart_vector::empty() const {
    return length == 0;
}

void smart_vector::resize(size_t size) {
    switch (size) {
        case 0:
            if (length > 1) {
                big_object->by();
            }
            little_object = 0;
            break;
        case 1:
            if (length > 1) {
                uint32_t v = big_object->data[0];
                big_object->by();
                little_object = v;
            }
            break;
        default:
            if (length > 0) {
                if (big_object->count_of_owners != 1 || size > big_object->capacity || size < length) {
                    big_object = new smart_data(*big_object, size + 8);
                }
            } else {
                uint32_t v = little_object;
                big_object = new smart_data(size + 8);
                big_object->data[0] = v;
            }
            break;
    }
    length = size;
}

const uint32_t &smart_vector::operator[](int i) const {
    //assert(i >= 0);
    //assert(i < length);
    if (length > 1) {
        return big_object->data[i];
    } else {
        return little_object;
    }
}

uint32_t &smart_vector::operator[](int i) {
    //assert(i >= 0);
    //assert(i < length);
    if (length > 1) {
        update();
        return big_object->data[i];
    } else {
        return little_object;
    }
}

const uint32_t &smart_vector::back() const {
    //assert(length != 0);
    return (*this)[length - 1];
}

uint32_t &smart_vector::back() {
    //assert(length != 0);
    return (*this)[length - 1];
}

void smart_vector::push_back(uint32_t a) {
    switch (length) {
        case 0:
            little_object = a;
            break;
        case 1: {
            uint32_t v = little_object;
            big_object = new smart_data(8);
            big_object->data[0] = v;
            big_object->data[1] = a;
            break;
        }
        default:
            if (length == big_object->capacity) {
                big_object = new smart_data(*big_object, big_object->capacity * 2);
            } else {
                update();
            }
            big_object->data[length] = a;
            break;
    }
    ++length;
}

void smart_vector::pop_back() {
    //assert(length != 0);
    switch (length) {
        case 1:
            little_object = 0;
            break;
        case 2: {
            uint32_t v = big_object->data[0];
            big_object->by();
            little_object = v;
            break;
        }
        default:
            update();
            big_object->data[length - 1] = 0;
            break;
    }
    --length;
}

void smart_vector::swap(smart_vector &other) {
    smart_vector tmp = *this;
    *this = other;
    other = tmp;
}

inline void smart_vector::update() {
    if (length > 1 && big_object->count_of_owners != 1) {
        big_object = new smart_data(*big_object);
    }
}