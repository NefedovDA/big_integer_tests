//
// Created by nefed on 14.06.2018.
//

#ifndef SMART_VECTOR_H
#define SMART_VECTOR_H

#include <stdint-gcc.h>
#include <vector>

struct smart_vector {
    smart_vector();

    explicit smart_vector(size_t size);

    smart_vector(smart_vector const &other) noexcept;

    smart_vector &operator=(smart_vector const &other) noexcept;

    ~smart_vector();

    size_t size() const;

    bool empty() const;

    void resize(size_t size);

    const uint32_t &operator[](int i) const;

    uint32_t &operator[](int i);

    const uint32_t &back() const;

    uint32_t &back();

    void push_back(uint32_t a);

    void pop_back();

    void swap(smart_vector &other);

private:
    size_t length;

    struct smart_data {
        const size_t capacity;
        uint32_t *data;
        uint64_t count_of_owners = 1;

        smart_data() = delete;

        smart_data &operator=(smart_data const &other) = delete;

        explicit smart_data(size_t new_capacity);

        smart_data(smart_data const &other);

        smart_data(smart_data const &other, size_t new_capacity);

        smart_data *hy();

        void by();

    private:
        ~smart_data();
    };

    union {
        smart_data *big_object;
        uint32_t little_object = 0;
    };

    inline void update();
};

#endif //SMART_VECTOR_H
