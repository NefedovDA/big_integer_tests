#ifndef BIG_INTEGER_H
#define BIG_INTEGER_H

#include <cstddef>
#include <iosfwd>
#include <cstdint>
//#include <vector>
#include "utils/smart_vector.h"

struct big_integer {
    big_integer() = default;

    big_integer(big_integer const &other) = default;

    big_integer(int a);

    big_integer(uint32_t a);

    explicit big_integer(std::string const &str);

    ~big_integer() = default;

    big_integer &operator=(big_integer const &other) = default;

    big_integer &operator+=(big_integer const &rhs);

    big_integer &operator-=(big_integer const &rhs);

    big_integer &operator*=(big_integer const &rhs);

    big_integer &operator/=(big_integer const &rhs);

    big_integer &operator%=(big_integer const &rhs);

    big_integer &operator&=(big_integer const &rhs);

    big_integer &operator|=(big_integer const &rhs);

    big_integer &operator^=(big_integer const &rhs);

    big_integer &operator<<=(int rhs);

    big_integer &operator>>=(int rhs);

    big_integer operator+() const;

    big_integer operator-() const;

    big_integer operator~() const;

    big_integer &operator++();

    const big_integer operator++(int);

    big_integer &operator--();

    const big_integer operator--(int);

    friend bool operator==(big_integer const &a, big_integer const &b);

    friend bool operator!=(big_integer const &a, big_integer const &b);

    friend bool operator<(big_integer const &a, big_integer const &b);

    friend bool operator>(big_integer const &a, big_integer const &b);

    friend bool operator<=(big_integer const &a, big_integer const &b);

    friend bool operator>=(big_integer const &a, big_integer const &b);

    friend std::string to_string(big_integer const &a);

private:
    smart_vector data;
    //std::vector<uint32_t> data;
    bool is_negate = false;

    void sift_zeros();

    bool is_zero() const;

    friend int compare(const big_integer &a, const big_integer &b);

    void to_twos_complement();

    void from_twos_complement();

    template<class FunctorT>
    void bit_op(big_integer assistant, const FunctorT &op) {
        if (data.size() < assistant.data.size()) {
            data.resize(assistant.data.size());
        } else {
            assistant.data.resize(data.size());
        }

        to_twos_complement();
        assistant.to_twos_complement();

        data.resize(std::max(data.size(), assistant.data.size()));
        for (size_t i = 0; i < data.size(); ++i) {
            if (i < assistant.data.size()) {
                data[i] = op(data[i], assistant.data[i]);
            } else {
                data[i] = op(data[i], 0);
            }
        }

        is_negate = op(is_negate, assistant.is_negate);

        from_twos_complement();
    }
};

big_integer operator+(big_integer a, big_integer const &b);

big_integer operator-(big_integer a, big_integer const &b);

big_integer operator*(big_integer a, big_integer const &b);

big_integer operator/(big_integer a, big_integer const &b);

big_integer operator%(big_integer a, big_integer const &b);

big_integer operator&(big_integer a, big_integer const &b);

big_integer operator|(big_integer a, big_integer const &b);

big_integer operator^(big_integer a, big_integer const &b);

big_integer operator<<(big_integer a, int b);

big_integer operator>>(big_integer a, int b);

bool operator==(big_integer const &a, big_integer const &b);

bool operator!=(big_integer const &a, big_integer const &b);

bool operator<(big_integer const &a, big_integer const &b);

bool operator>(big_integer const &a, big_integer const &b);

bool operator<=(big_integer const &a, big_integer const &b);

bool operator>=(big_integer const &a, big_integer const &b);

std::string to_string(big_integer const &a);

std::ostream &operator<<(std::ostream &s, big_integer const &a);

#endif // BIG_INTEGER_H
