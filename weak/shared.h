#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t
#include <type_traits>

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() noexcept {
    }

    SharedPtr(std::nullptr_t) noexcept {
    }

    explicit SharedPtr(T* ptr) noexcept {
        block_ = new ControlBlock1<T>(ptr);
        block_->IncShared();
        ptr_ = ptr;
    }

    template <typename Y>
    explicit SharedPtr(Y* ptr) noexcept {
        block_ = new ControlBlock1<Y>(ptr);
        block_->IncShared();
        ptr_ = ptr;
    }

    SharedPtr(const SharedPtr& other) noexcept {
        block_ = other.block_;
        ptr_ = other.ptr_;

        if (*this) {
            block_->IncShared();
        }
    }

    SharedPtr(SharedPtr&& other) noexcept {
        Swap(other);
    }

    template <typename Y>
    friend class SharedPtr;

    template <typename Y>
    friend class WeakPtr;

    template <class Y>
    SharedPtr(const SharedPtr<Y>& other) noexcept {
        ptr_ = other.ptr_;
        block_ = other.block_;

        if (*this) {
            block_->IncShared();
        }
    }

    template <class Y>
    SharedPtr(SharedPtr<Y>&& other) noexcept {
        ptr_ = other.ptr_;
        block_ = other.block_;
        other.ptr_ = nullptr;
        other.block_ = nullptr;
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) noexcept {
        ptr_ = ptr;
        block_ = other.block_;

        if (*this) {
            block_->IncShared();
        }
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other) {
        if (other.Expired()) {
            throw BadWeakPtr();
        }

        ptr_ = other.ptr_;
        block_ = other.block_;

        if (block_ != nullptr) {
            block_->IncShared();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) {
        SharedPtr<T>(other).Swap(*this);
        return *this;
    }

    template <typename Y>
    SharedPtr& operator=(const SharedPtr<Y>& other) {
        SharedPtr<T>(other).Swap(*this);
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) {
        SharedPtr<T>(std::move(other)).Swap(*this);
        return *this;
    }

    template <typename Y>
    SharedPtr& operator=(SharedPtr<Y>&& other) {
        SharedPtr<T>(std::move(other)).Swap(*this);
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        if (!*this) {
            return;
        }

        block_->DecShared();
        if (block_->GetShared() == 0) {
            if (block_->GetWeak() == 0) {
                block_->ObjectDestructor();
                delete block_;
            } else {
                block_->ObjectDestructor();
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() noexcept {
        SharedPtr<T>().Swap(*this);
    }

    void Reset(T* ptr) noexcept {
        SharedPtr<T>(ptr).Swap(*this);
    }

    template <typename Y>
    void Reset(Y* ptr) noexcept {
        SharedPtr<T>(ptr).Swap(*this);
    }

    void Swap(SharedPtr& other) noexcept {
        std::swap(block_, other.block_);
        std::swap(ptr_, other.ptr_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
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
        if (!*this) {
            return 0;
        }

        return block_->GetShared();
    }

    explicit operator bool() const noexcept {
        return Get() != nullptr;
    }

    template <typename _T, typename... Args>
    friend SharedPtr<_T> MakeShared(Args&&... args);

private:
    BaseBlock* block_ = nullptr;
    T* ptr_ = nullptr;
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right);

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    SharedPtr<T> shared;
    auto block = new ControlBlock2<T>(std::forward<Args>(args)...);
    shared.block_ = block;
    shared.block_->IncShared();
    shared.ptr_ = block->Get();

    return shared;
}

// Look for usage examples in tests
template <typename T>
class EnableSharedFromThis {
public:
    SharedPtr<T> SharedFromThis();
    SharedPtr<const T> SharedFromThis() const;

    WeakPtr<T> WeakFromThis() noexcept;
    WeakPtr<const T> WeakFromThis() const noexcept;
};