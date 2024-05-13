#pragma once

#include "sw_fwd.h"  // Forward declaration

#include "shared.h"

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
    template <typename Y>
    friend class WeakPtr;

    template <typename Y>
    friend class SharedPtr;

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() noexcept {
    }

    WeakPtr(const WeakPtr& other) {
        block_ = other.block_;
        ptr_ = other.ptr_;

        if (block_ != nullptr) {
            block_->IncWeak();
        }
    }

    WeakPtr(WeakPtr&& other) {
        Swap(other);
    }

    template <class Y>
    WeakPtr(const WeakPtr<Y>& other) {
        block_ = other.block_;
        ptr_ = other.ptr_;

        if (block_ != nullptr) {
            block_->IncWeak();
        }
    }

    template <class Y>
    WeakPtr(WeakPtr<Y>&& other) {
        ptr_ = other.ptr_;
        block_ = other.block_;
        other.ptr_ = nullptr;
        other.block_ = nullptr;
    }

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other) {
        ptr_ = other.ptr_;
        block_ = other.block_;

        if (block_ != nullptr) {
            block_->IncWeak();
        }
    }

    template <class Y>
    WeakPtr(const SharedPtr<Y>& other) {
        ptr_ = other.ptr_;
        block_ = other.block_;

        if (block_ != nullptr) {
            block_->IncWeak();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        WeakPtr<T>(other).Swap(*this);
        return *this;
    }

    WeakPtr& operator=(WeakPtr&& other) {
        WeakPtr<T>(std::move(other)).Swap(*this);
        return *this;
    }

    template <class Y>
    WeakPtr& operator=(const WeakPtr<Y>& other) {
        WeakPtr<T>(other).Swap(*this);
        return *this;
    }

    template <class Y>
    WeakPtr& operator=(WeakPtr<T>&& other) {
        WeakPtr<T>(std::move(other)).Swap(*this);
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        if (block_ == nullptr) {
            return;
        }

        block_->DecWeak();

        if (block_->GetShared() == 0 && block_->GetWeak() == 0) {
            delete block_;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() noexcept {
        WeakPtr().Swap(*this);
    }

    void Swap(WeakPtr& other) noexcept {
        std::swap(block_, other.block_);
        std::swap(ptr_, other.ptr_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const noexcept {
        if (block_ == nullptr) {
            return 0;
        }

        return block_->GetShared();
    }

    bool Expired() const noexcept {
        return UseCount() == 0;
    }

    SharedPtr<T> Lock() const {
        return Expired() ? SharedPtr<T>() : SharedPtr<T>(*this);
    }

private:
    BaseBlock* block_ = nullptr;
    T* ptr_ = nullptr;
};
