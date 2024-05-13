#pragma once

#include "compressed_pair.h"

#include <cstddef>      // std::nullptr_t
#include <type_traits>  // std::nullptr_t

template <typename T>
struct Slug {
    Slug() = default;

    template <typename U>
    Slug(Slug<U> other) {
    }

    void operator()(T* ptr) {
        delete ptr;
    }
};

template <typename T>
struct Slug<T[]> {
    Slug() = default;

    template <typename U>
    Slug(Slug<U> other) {
    }

    void operator()(T* ptr) {
        delete[] ptr;
    }
};

// Primary template
template <typename T, typename Deleter = Slug<T>>
class UniquePtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) noexcept {
        data_.GetFirst() = ptr;
        data_.GetSecond() = Deleter();
    }

    UniquePtr(T* ptr, Deleter deleter) {
        data_.GetFirst() = ptr;
        data_.GetSecond() = std::forward<decltype(deleter)>(deleter);
    }

    UniquePtr(UniquePtr&& other) noexcept {
        data_.GetFirst() = other.Release();
        GetDeleter() = std::forward<Deleter>(other.GetDeleter());
    }

    template <typename U, typename E>
    UniquePtr(UniquePtr<U, E>&& other) noexcept {
        data_.GetFirst() = other.Release();
        GetDeleter() = std::forward<E>(other.GetDeleter());
    }

    UniquePtr(const UniquePtr& other) noexcept = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(const UniquePtr& other) noexcept = delete;

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        Reset(other.Release());
        GetDeleter() = std::forward<Deleter>(other.GetDeleter());

        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) noexcept {
        Reset();
        return *this;
    }

    template <typename U, typename E>
    UniquePtr& operator=(UniquePtr<U, E>&& other) noexcept {
        Reset(other.Release());
        GetDeleter() = std::forward<E>(other.GetDeleter());

        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        GetDeleter()(Get());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() noexcept {
        T* ptr = Get();
        data_.GetFirst() = nullptr;
        return ptr;
    }

    void Reset(T* ptr = nullptr) noexcept {
        T* old = Get();
        data_.GetFirst() = ptr;

        GetDeleter()(old);
    }

    void Swap(UniquePtr& other) noexcept {
        std::swap(data_.GetFirst(), other.data_.GetFirst());
        std::swap(data_.GetSecond(), other.data_.GetSecond());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const noexcept {
        return data_.GetFirst();
    }

    Deleter& GetDeleter() noexcept {
        return data_.GetSecond();
    }

    const Deleter& GetDeleter() const noexcept {
        return data_.GetSecond();
    }

    explicit operator bool() const noexcept {
        return Get() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator*() const noexcept {
        return *Get();
    }

    T* operator->() const noexcept {
        return Get();
    }

private:
    CompressedPair<T*, Deleter> data_;
};

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) noexcept {
        data_.GetFirst() = ptr;
        data_.GetSecond() = Deleter();
    }

    UniquePtr(T* ptr, Deleter deleter) {
        data_.GetFirst() = ptr;
        data_.GetSecond() = std::forward<decltype(deleter)>(deleter);
    }

    UniquePtr(UniquePtr&& other) noexcept {
        data_.GetFirst() = other.Release();
        GetDeleter() = std::forward<Deleter>(other.GetDeleter());
    }

    template <typename U, typename E>
    UniquePtr(UniquePtr<U, E>&& other) noexcept {
        data_.GetFirst() = other.Release();
        GetDeleter() = std::forward<E>(other.GetDeleter());
    }

    UniquePtr(const UniquePtr& other) noexcept = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(const UniquePtr& other) noexcept = delete;

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        Reset(other.Release());
        GetDeleter() = std::forward<Deleter>(other.GetDeleter());

        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) noexcept {
        Reset();
        return *this;
    }

    template <typename U, typename E>
    UniquePtr& operator=(UniquePtr<U, E>&& other) noexcept {
        Reset(other.Release());
        GetDeleter() = std::forward<E>(other.GetDeleter());

        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        GetDeleter()(Get());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() noexcept {
        T* ptr = Get();
        data_.GetFirst() = nullptr;
        return ptr;
    }

    void Reset(T* ptr = nullptr) noexcept {
        T* old = Get();
        data_.GetFirst() = ptr;

        GetDeleter()(old);
    }

    void Swap(UniquePtr& other) noexcept {
        std::swap(data_.GetFirst(), other.data_.GetFirst());
        std::swap(data_.GetSecond(), other.data_.GetSecond());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const noexcept {
        return data_.GetFirst();
    }

    Deleter& GetDeleter() noexcept {
        return data_.GetSecond();
    }

    const Deleter& GetDeleter() const noexcept {
        return data_.GetSecond();
    }

    explicit operator bool() const noexcept {
        return Get() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    T& operator[](std::size_t i) const noexcept {
        return Get()[i];
    }

private:
    CompressedPair<T*, Deleter> data_;
};
