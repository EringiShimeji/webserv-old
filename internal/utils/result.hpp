#ifndef RESULT_HPP
#define RESULT_HPP

#include "try.hpp"
#include <cstddef>
#include <exception>

template<class T, class E>
class Result;

namespace types {
    template<class T>
    class Ok {
    public:
        explicit Ok(T val) : val_(val) {}

        Ok(const Ok &other) {
            *this = other;
        }

        ~Ok() {}

        Ok &operator=(const Ok &other) {
            if (this != &other) {
                val_ = other.val_;
            }
            return *this;
        }

        T val() {
            return val_;
        }

        bool operator==(const Ok &other) const {
            return val_ == other.val_;
        }

        bool operator!=(const Ok &other) const {
            return val_ != other.val_;
        }

    private:
        T val_;
    };

    template<class E>
    class Err {
    public:
        explicit Err(E error) : error_(error) {}

        Err(const Err &other) {
            *this = other;
        }

        ~Err() {}

        Err &operator=(const Err &other) {
            if (this != &other) {
                error_ = other.error_;
            }
            return *this;
        }

        E error() {
            return error_;
        }

        bool operator==(const Err &other) const {
            return error_ == other.error_;
        }

        bool operator!=(const Err &other) const {
            return error_ != other.error_;
        }

    private:
        E error_;
    };
} // namespace types

template<class T>
types::Ok<T> Ok(T val) { // NOLINT(readability-identifier-naming)
    return types::Ok<T>(val);
}

template<class E>
types::Err<E> Err(E val) { // NOLINT(readability-identifier-naming)
    return types::Err<E>(val);
}

template<class T, class E>
class Result {
public:
    Result() : ok_(NULL), err_(NULL) {}

    explicit Result(types::Ok<T> *ok) : ok_(ok), err_(NULL) {}

    explicit Result(types::Err<E> *err) : ok_(NULL), err_(err) {}

    // Result<int, std::string> target = Ok(0); みたいなことができるようにexplicitをつけない
    // NOLINTNEXTLINE(google-explicit-constructor)
    Result(types::Ok<T> ok) : ok_(new types::Ok<T>(ok)), err_(NULL) {}

    // Result<int, std::string> target = Err<std::string>("error"); みたいなことができるようにexplicitをつけない
    // NOLINTNEXTLINE(google-explicit-constructor)
    Result(types::Err<E> err) : ok_(NULL), err_(new types::Err<E>(err)) {}

    Result(const Result &other) {
        this->ok_ = other.isOk() ? new types::Ok<T>(*other.ok_) : NULL;
        this->err_ = other.isErr() ? new types::Err<E>(*other.err_) : NULL;
    }

    ~Result() {
        delete ok_;
        delete err_;
    }

    Result &operator=(const Result &other) {
        if (this != &other) {
            if (other.isOk()) {
                if (ok_ == NULL) {
                    ok_ = new types::Ok<T>(*other.ok_);
                } else {
                    *ok_ = *other.ok_;
                }
                err_ = NULL;
            } else if (other.isErr()) {
                if (err_ == NULL) {
                    err_ = new types::Err<E>(*other.err_);
                } else {
                    *err_ = *other.err_;
                }
                ok_ = NULL;
            }
        }
        return *this;
    }

    bool operator==(const Result &other) const {
        if (isOk() && other.isOk()) {
            return *ok_ == *other.ok_;
        }
        if (isErr() && other.isErr()) {
            return *err_ == *other.err_;
        }
        return false;
    }

    bool operator!=(const Result &other) const {
        if (isOk() && other.isOk()) {
            return *ok_ != *other.ok_;
        }
        if (isErr() && other.isErr()) {
            return *err_ != *other.err_;
        }
        return true;
    }

    bool isOk() const {
        return ok_ != NULL;
    }

    bool isErr() const {
        return err_ != NULL;
    }

    T unwrap() const {
        return ok_->val();
    }

    T unwrapOr(T val) const {
        if (isOk())
            return ok_->val();
        return val;
    }

    bool canUnwrap() const {
        return isOk();
    }

private:
    types::Ok<T> *ok_;
    types::Err<E> *err_;
};

#endif
