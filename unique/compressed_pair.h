#pragma once

#include <type_traits>
#include <memory>

enum ElementType { FIRST, SECOND };

template <typename T, ElementType Etype, bool empty = std::is_empty_v<T> && !std::is_final_v<T>>
class CompressedElement {
public:
    CompressedElement() = default;

    template <typename E>
    CompressedElement(E&& elem) : elem_(std::forward<E>(elem)) {
    }

    T& Get() {
        return elem_;
    }

    const T& Get() const {
        return elem_;
    }

private:
    T elem_;
};

template <typename T, ElementType Etype>
class CompressedElement<T, Etype, true> : public T {
public:
    CompressedElement() = default;

    template <typename E>
    CompressedElement(E&& elem) : T(std::forward<E>(elem)) {
    }

    T& Get() {
        return *this;
    }

    const T& Get() const {
        return *this;
    }
};

template <typename F, typename S>
class CompressedPair : public CompressedElement<F, FIRST>, public CompressedElement<S, SECOND> {
public:
    CompressedPair() = default;

    template <typename T1, typename T2>
    CompressedPair(T1&& first, T2&& second)
        : CompressedElement<F, FIRST>(std::forward<T1>(first)),
          CompressedElement<S, SECOND>(std::forward<T2>(second)) {
    }

    F& GetFirst() {
        return CompressedElement<F, FIRST>::Get();
    }

    S& GetSecond() {
        return CompressedElement<S, SECOND>::Get();
    };

    const F& GetFirst() const {
        return CompressedElement<F, FIRST>::Get();
    }

    const S& GetSecond() const {
        return CompressedElement<S, SECOND>::Get();
    };
};
