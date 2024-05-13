#pragma once

#include <cstddef>  // for std::nullptr_t
#include <utility>  // for std::exchange / std::swap

class SimpleCounter {
public:
    size_t IncRef() noexcept {
        return ++count_;
    }

    size_t DecRef() noexcept {
        return --count_;
    }

    size_t RefCount() const noexcept {
        return count_;
    }

private:
    size_t count_ = 0;
};

struct DefaultDelete {
    template <typename T>
    static void Destroy(T* object) {
        delete object;
    }
};

template <typename Derived, typename Counter, typename Deleter>
class RefCounted {
public:
    // Increase reference counter.
    void IncRef() {
        counter_.IncRef();
    }

    // Decrease reference counter.
    // Destroy object using Deleter when the last instance dies.
    void DecRef() {
        if (RefCount() <= 1) {
            Deleter::Destroy(static_cast<Derived*>(this));
            return;
        }

        counter_.DecRef();
    }

    // Get current counter value (the number of strong references).
    size_t RefCount() const {
        return counter_.RefCount();
    }

private:
    Counter counter_;
};

template <typename Derived, typename D = DefaultDelete>
using SimpleRefCounted = RefCounted<Derived, SimpleCounter, D>;

template <typename T>
class IntrusivePtr {
    template <typename Y>
    friend class IntrusivePtr;

public:
    // Constructors
    IntrusivePtr() noexcept {
    }

    IntrusivePtr(std::nullptr_t) noexcept {
    }

    IntrusivePtr(T* ptr) noexcept {
        ptr_ = ptr;
        ptr_->IncRef();
    }

    template <typename Y>
    IntrusivePtr(const IntrusivePtr<Y>& other) noexcept {
        ptr_ = other.ptr_;
        if (*this) {
            ptr_->IncRef();
        }
    }

    template <typename Y>
    IntrusivePtr(IntrusivePtr<Y>&& other) noexcept {
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
    }

    IntrusivePtr(const IntrusivePtr& other) noexcept {
        ptr_ = other.ptr_;
        if (*this) {
            ptr_->IncRef();
        }
    }

    IntrusivePtr(IntrusivePtr&& other) noexcept {
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
    }

    // `operator=`-s
    template <typename Y>
    IntrusivePtr& operator=(const IntrusivePtr<Y>& other) noexcept {
        IntrusivePtr<T>(other).Swap(*this);
        return *this;
    }

    template <typename Y>
    IntrusivePtr& operator=(IntrusivePtr<Y>&& other) noexcept {
        IntrusivePtr<T>(std::move(other)).Swap(*this);
        return *this;
    }

    IntrusivePtr& operator=(const IntrusivePtr& other) noexcept {
        IntrusivePtr<T>(other).Swap(*this);
        return *this;
    }

    IntrusivePtr& operator=(IntrusivePtr&& other) noexcept {
        IntrusivePtr<T>(std::move(other)).Swap(*this);
        return *this;
    }

    // Destructor
    ~IntrusivePtr() {
        if (ptr_ == nullptr) {
            int a = 1;
            return;
        }

        ptr_->DecRef();
    }

    // Modifiers
    void Reset() {
        IntrusivePtr<T>().Swap(*this);
    }

    void Reset(T* ptr) {
        IntrusivePtr<T>(ptr).Swap(*this);
    }

    void Swap(IntrusivePtr& other) {
        std::swap(ptr_, other.ptr_);
    }

    // Observers
    T* Get() const noexcept {
        return ptr_;
    }

    T& operator*() const noexcept {
        return *Get();
    }

    T* operator->() const noexcept {
        return Get();
    }

    size_t UseCount() const noexcept {
        if (Get() == nullptr) {
            return 0;
        }

        return ptr_->RefCount();
    }

    explicit operator bool() const noexcept {
        return UseCount() != 0;
    }

    template <typename T_, typename... Args>
    friend IntrusivePtr<T_> MakeIntrusive(Args&&... args);

private:
    T* ptr_ = nullptr;
};

template <typename T, typename... Args>
IntrusivePtr<T> MakeIntrusive(Args&&... args) {
    IntrusivePtr<T> intrusive;
    intrusive.ptr_ = new T(std::forward<Args>(args)...);
    intrusive.ptr_->IncRef();
    return intrusive;
}
