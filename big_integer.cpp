//
// Created by nefed on 10.06.2018.
//

#include <cstdlib>
#include <functional>
#include <algorithm>
#include <cassert>
#include "big_integer.h"

#define TWO_IN_32 4294967296ULL

//=================================================
//==============functions=for=help=================
//=================declaration=====================
//=================================================

int vector_compare(const std::vector<uint32_t> &a, const std::vector<uint32_t> &b);

void vector_shift_right(std::vector<uint32_t> &resource, size_t offset);

void vector_shift_left(std::vector<uint32_t> &resource, size_t offset);

uint32_t search_dividend(const big_integer &a, const big_integer &divider);

//=================================================
//=============units=for=help======================
//=================================================

union fast_split_ull {
    uint64_t ull;
    uint32_t u[2];
} helper;

uint32_t carry;

uint32_t safe_plus(uint64_t a, uint64_t b = 0) {
    helper.ull = a + b + carry;
    carry = helper.u[1];
    return helper.u[0];
}

uint32_t safe_minus(uint64_t a, uint64_t b) {
    helper.ull = TWO_IN_32 + a - b - carry;
    if (helper.ull >= TWO_IN_32) {
        helper.ull -= TWO_IN_32;
        carry = 0;
    } else {
        carry = 1;
    }
    return helper.u[0];
}

uint32_t safe_multiplies(uint64_t a, uint64_t b, uint64_t d) {
    helper.ull = a * b + carry + d;
    carry = helper.u[1];
    return helper.u[0];
}

const auto bit_and = std::bit_and<uint32_t>();
const auto bit_or = std::bit_or<uint32_t>();
const auto bit_xor = std::bit_xor<uint32_t>();

uint32_t bit_shl(uint64_t v, uint32_t offset) {
    helper.ull = (v << offset) + carry;
    carry = helper.u[1];
    return helper.u[0];
}

//=================================================
//==================constructors===================
//=================================================

big_integer::big_integer(int a) : data(1), is_negate(a < 0) {
    data[0] = std::llabs(a);
    sift_zeros();
}

big_integer::big_integer(uint32_t a) : data(1) {
    data[0] = a;
    sift_zeros();
}

big_integer::big_integer(std::string const &str) : data() {
    if (str.empty()) {
        throw std::runtime_error("invalid string");
    }
    size_t minus_count = 0;
    for (char c : str) {
        if (c == '-') {
            ++minus_count;
            continue;
        } else if (c >= '0' && c <= '9') {
            *this *= 10;
            *this += (c - '0');
        } else {
            throw std::runtime_error("invalid string");
        }
    }
    if (str[0] == '-' && minus_count == 1) {
        is_negate = true;
    } else if (minus_count != 0) {
        throw std::runtime_error("invalid string");
    }
}

//=================================================
//=================compare=========================
//=================================================

bool operator==(big_integer const &a, big_integer const &b) {
    return compare(a, b) == 0;
}

bool operator!=(big_integer const &a, big_integer const &b) {
    return compare(a, b) != 0;
}

bool operator<(big_integer const &a, big_integer const &b) {
    return compare(a, b) == -1;
}

bool operator>(big_integer const &a, big_integer const &b) {
    return compare(a, b) == 1;
}

bool operator<=(big_integer const &a, big_integer const &b) {
    return compare(a, b) != 1;
}

bool operator>=(big_integer const &a, big_integer const &b) {
    return compare(a, b) != -1;
}

//=================================================
//===============unary=arithmetic==================
//=================================================

big_integer &big_integer::operator+=(big_integer const &rhs) {
    if (rhs.is_zero()) {
        return *this;
    }
    if (is_zero()) {
        return *this = rhs;
    }
    if (is_negate == rhs.is_negate) {
        carry = 0;
        data.resize(std::max(data.size(), rhs.data.size()));
        size_t i = 0;
        for (; i < rhs.data.size(); ++i) {
            data[i] = safe_plus(data[i], rhs.data[i]);
        }
        for (; i < data.size(); ++i) {
            data[i] = safe_plus(data[i]);
        }
        if (carry != 0) {
            data.push_back(carry);
        }
    } else {
        *this -= -rhs;
    }
    return *this;
}

big_integer &big_integer::operator-=(big_integer const &rhs) {
    if (rhs.is_zero()) {
        return *this;
    }
    if (is_zero()) {
        return *this = -rhs;
    }
    if (is_negate == rhs.is_negate) {
        carry = 0;
        int comp = vector_compare(data, rhs.data);
        if (comp == 0) {
            return *this = 0;
        } else if (comp == 1) {
            for (size_t i = 0; i < data.size(); ++i) {
                if (i < rhs.data.size()) {
                    data[i] = safe_minus(data[i], rhs.data[i]);
                } else {
                    data[i] = safe_minus(data[i], 0);
                }
            }
        } else {
            data.resize(rhs.data.size());
            for (size_t i = 0; i < data.size(); ++i) {
                data[i] = safe_minus(rhs.data[i], data[i]);
            }
            is_negate = !is_negate;
        }
        sift_zeros();
    } else {
        *this += -rhs;
    }

    return *this;
}

big_integer &big_integer::operator*=(big_integer const &rhs) {
    std::vector<uint32_t> buf;
    buf.resize(data.size() + rhs.data.size());
    for (size_t i = 0; i < data.size(); i++) {
        carry = 0;
        for (size_t j = 0; j < rhs.data.size(); j++) {
            buf[i + j] = safe_multiplies(data[i], rhs.data[j], buf[i + j]);
        }
        buf[i + rhs.data.size()] = carry;
    }
    is_negate ^= rhs.is_negate;
    data.swap(buf);
    sift_zeros();
    return *this;
}

uint32_t find_d(uint32_t a) {
    uint32_t mask = 2147483648;
    uint32_t r = 0;
    while (true) {
        if ((mask & a) != 0) {
            return r;
        }
        ++r;
        mask >>= 1;
    }
}

uint32_t get(const std::vector<uint32_t> &v, size_t i) {
    if (i < v.size()) {
        return v[i];
    } else {
        return 0;
    }
}

uint32_t div_3_2(uint32_t u2, uint32_t u1, uint32_t u0, uint32_t d1, uint32_t d0) {
    if (u2 == d1 && u1 == d0) {
        return UINT32_MAX;
    }
    helper.u[0] = u1;
    helper.u[1] = u2;
    uint64_t U = helper.ull;
    helper.u[0] = d0;
    helper.u[1] = d1;
    uint64_t D = helper.ull;
    uint64_t Q = U / d1;
    if (Q > UINT32_MAX) {
        helper.u[1] = d0 - u1;
        helper.ull -= u0;
        if (helper.ull <= D) {
            return UINT32_MAX;
        } else {
            return UINT32_MAX - 1;
        }
    }
    uint64_t DQ = Q * d0;
    helper.ull = U - Q * d1;
    helper.u[1] = helper.u[0];
    helper.u[0] = u0;

    if (helper.ull < DQ) {
        --Q;
        helper.ull += D;
        if (helper.ull >= D && helper.ull < DQ) {
            --Q;
        }
    }
    helper.ull = Q;
    return helper.u[0];
}

big_integer &big_integer::operator/=(big_integer const &rhs) {
    assert(!rhs.is_zero());
    if (vector_compare(data, rhs.data) == -1) {
        return *this = 0;
    }
    if (rhs.data.size() == 1) {
        uint32_t d = rhs.data[0];
        std::vector<uint32_t> out(data.size());
        carry = 0;
        for (size_t i = data.size(); i != 0; --i) {
            helper.u[1] = carry;
            helper.u[0] = data[i - 1];
            uint64_t tmp = helper.ull;
            helper.ull /= d;
            out[i - 1] = helper.u[0];
            helper.ull = tmp - helper.ull * d;
            carry = helper.u[0];
        }
        data.swap(out);
        is_negate ^= rhs.is_negate;
        sift_zeros();
        return *this;
    } else {
        bool sign = is_negate ^rhs.is_negate;

        uint32_t d = find_d(rhs.data.back());

        *this <<= d;
        big_integer v = rhs << d;

        is_negate = false;
        v.is_negate = false;

        size_t n = v.data.size();
        size_t m = data.size();
        size_t k = m - n;

        std::vector<uint32_t> buf(k + 1);

        vector_shift_right(v.data, k);

        if (data.back() >= v.data.back()) {
            buf[k] = 1;
            *this -= v;
        }

        uint32_t d0 = v.data[v.data.size() - 2];
        uint32_t d1 = v.data[v.data.size() - 1];


        while (k != 0) {
            --k;
            v >>= 32;

            size_t l = v.data.size();
            buf[k] = div_3_2(get(data, l), get(data, l - 1), get(data, l - 2), d1, d0);
            *this -= v * buf[k];
            if (*this < 0) {
                *this += v;
                ++buf[k];
            }
        }

        is_negate = sign;
        data.swap(buf);
        sift_zeros();
        return *this;
    }
}

big_integer &big_integer::operator%=(big_integer const &rhs) {
    *this -= (*this / rhs) * rhs;
    return *this;
}

//=================================================
//======================bits=======================
//=================================================

big_integer &big_integer::operator&=(big_integer const &rhs) {
    bit_op(rhs, bit_and);
    return *this;
}

big_integer &big_integer::operator|=(big_integer const &rhs) {
    bit_op(rhs, bit_or);
    return *this;
}

big_integer &big_integer::operator^=(big_integer const &rhs) {
    bit_op(rhs, bit_xor);
    return *this;
}

big_integer big_integer::operator~() const {
    big_integer r = *this;
    r.to_twos_complement();
    for (size_t i = 0; i < r.data.size(); ++i) {
        r.data[i] = ~r.data[i];
    }
    r.is_negate = !r.is_negate;
    r.from_twos_complement();
    return r;
}

big_integer &big_integer::operator<<=(int rhs) {
    assert(rhs >= 0);
    uint32_t big_offset = rhs / 32;
    uint32_t little_offset = rhs - big_offset * 32;
    if (big_offset != 0) {
        vector_shift_right(data, big_offset);
    }
    if (little_offset != 0) {
        carry = 0;
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] = bit_shl(data[i], little_offset);
        }
        if (carry != 0) {
            data.push_back(carry);
        }
    }
    return *this;
}

big_integer &big_integer::operator>>=(int rhs) {
    assert(rhs >= 0);
    uint32_t big_offset = rhs / 32;
    uint32_t little_offset = rhs - big_offset * 32;
    if (big_offset >= data.size()) {
        if (is_negate) {
            return *this = -1;
        } else {
            return *this = 0;
        }
    }
    bool rounding_flag = false;
    if (is_negate) {
        for (size_t i = 0; i < big_offset; ++i) {
            if (data[i] != 0) {
                rounding_flag = true;
                break;
            }
        }
        if ((data[big_offset] << (32 - little_offset)) != 0) {
            rounding_flag = true;
        }
    }
    if (big_offset != 0) {
        vector_shift_left(data, big_offset);
    }
    if (little_offset != 0) {
        for (size_t i = 0; i < data.size() - 1; ++i) {
            data[i] >>= little_offset;
            data[i] += data[i + 1] << (32 - little_offset);
        }
        data.back() >>= little_offset;
    }
    sift_zeros();
    if (rounding_flag) {
        *this -= 1;
    }
    return *this;
}

//=================================================
//===================binary========================
//=================================================

big_integer operator+(big_integer a, big_integer const &b) {
    return a += b;
}

big_integer operator-(big_integer a, big_integer const &b) {
    return a -= b;
}

big_integer operator*(big_integer a, big_integer const &b) {
    return a *= b;
}

big_integer operator/(big_integer a, big_integer const &b) {
    return a /= b;
}

big_integer operator%(big_integer a, big_integer const &b) {
    return a %= b;
}

big_integer operator&(big_integer a, big_integer const &b) {
    return a &= b;
}

big_integer operator|(big_integer a, big_integer const &b) {
    return a |= b;
}

big_integer operator^(big_integer a, big_integer const &b) {
    return a ^= b;
}

big_integer operator<<(big_integer a, int b) {
    return a <<= b;
}

big_integer operator>>(big_integer a, int b) {
    return a >>= b;
}

//=================================================
//===================for=out=======================
//=================================================

std::string to_string(big_integer const &a) {
    if (a.is_zero()) {
        return "0";
    }
    if (a.data.size() == 1) {
        return std::to_string(a.data[0]);
    }
    std::string str;
    big_integer copy = a;
    if (copy.is_negate) {
        str += "-";
        copy.is_negate = false;
    }
    std::vector<std::string> buf;
    while (!copy.is_zero()) {
        buf.push_back(to_string(copy % 1000000000));
        copy /= 1000000000;
    }
    str += buf.back();
    buf.pop_back();
    std::reverse(buf.begin(), buf.end());
    for (const auto &s : buf) {
        if (s.length() < 9) {
            str += std::string(9 - s.length(), '0');
        }
        str += s;
    }
    return str;
}

std::ostream &operator<<(std::ostream &s, big_integer const &a) {
    s << to_string(a);
    return s;
}

//=================================================
//=====================other=======================
//=================================================

big_integer big_integer::operator-() const {
    big_integer r = *this;
    if (!r.is_zero()) {
        r.is_negate = !r.is_negate;
    }
    return r;
}

big_integer big_integer::operator+() const {
    return *this;
}

big_integer &big_integer::operator++() {
    *this += 1;
    return *this;
}

const big_integer big_integer::operator++(int) {
    big_integer r = *this;
    ++*this;
    return r;
}

big_integer &big_integer::operator--() {
    *this -= 1;
    return *this;
}

const big_integer big_integer::operator--(int) {
    big_integer r = *this;
    --*this;
    return r;
}

//=================================================
//==============functions=for=help=================
//==================definition=====================
//=================================================

void big_integer::sift_zeros() {
    while (!data.empty() && data.back() == 0) {
        data.pop_back();
    }
    if (is_zero()) {
        is_negate = false;
    }
}

bool big_integer::is_zero() const {
    return data.empty();
}

int vector_compare(const std::vector<uint32_t> &a, const std::vector<uint32_t> &b) {
    if (a.size() < b.size()) {
        return -1;
    }
    if (a.size() > b.size()) {
        return 1;
    }

    for (size_t i = a.size(); i != 0; --i) {
        if (a[i - 1] != b[i - 1]) {
            if (a[i - 1] < b[i - 1]) {
                return -1;
            } else {
                return 1;
            }
        }
    }
    return 0;
}

void vector_shift_right(std::vector<uint32_t> &resource, size_t offset) {
    resource.resize(resource.size() + offset);
    for (size_t i = resource.size(); i != offset; --i) {
        resource[i - 1] = resource[i - 1 - offset];
    }
    for (size_t i = 0; i < offset; ++i) {
        resource[i] = 0;
    }
}

void vector_shift_left(std::vector<uint32_t> &resource, size_t offset) {
    for (size_t i = 0; i < resource.size() - offset; ++i) {
        resource[i] = resource[i + offset];
    }
    resource.resize(resource.size() - offset);
}

void big_integer::to_twos_complement() {
    if (is_negate) {
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] = ~data[i];
        }
        *this -= 1;
    }
}

void big_integer::from_twos_complement() {
    if (is_negate) {
        *this += 1;
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] = ~data[i];
        }
    }
    sift_zeros();
}

int compare(const big_integer &a, const big_integer &b) {
    if (a.is_negate) {
        if (b.is_negate) {
            return vector_compare(a.data, b.data) * -1;
        } else {
            return -1;
        }
    } else {
        if (b.is_negate) {
            return 1;
        } else {
            return vector_compare(a.data, b.data);
        }
    }
}


//=================================================
//=================================================
//=================================================
