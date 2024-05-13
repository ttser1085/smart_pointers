#pragma once

#include <exception>

class BadWeakPtr : public std::exception {};

class BaseBlock {
public:
    BaseBlock() noexcept = default;

    void IncShared() noexcept {
        ++counter_shared_;
    }

    void DecShared() noexcept {
        --counter_shared_;
    }

    void IncWeak() noexcept {
        ++counter_weak_;
    }

    void DecWeak() noexcept {
        --counter_weak_;
    }

    size_t GetShared() const noexcept {
        return counter_shared_;
    }

    size_t GetWeak() const noexcept {
        return counter_weak_;
    }

    virtual void ObjectDestructor() = 0;

    virtual ~BaseBlock() noexcept = default;

private:
    size_t counter_shared_ = 0;
    size_t counter_weak_ = 0;
};

template <typename T>
class ControlBlock1 final : public BaseBlock {
public:
    ControlBlock1(T* ptr) noexcept : BaseBlock() {
        ptr_ = ptr;
    }

    T* Get() const noexcept {
        return ptr_;
    }

    void ObjectDestructor() override {
        delete ptr_;
    }

    ~ControlBlock1() noexcept = default;

private:
    T* ptr_ = nullptr;
};

template <typename T>
class ControlBlock2 final : public BaseBlock {
public:
    template <typename... Args>
    ControlBlock2(Args&&... args) {
        new (Get()) T(std::forward<Args>(args)...);
    }

    T* Get() noexcept {
        return reinterpret_cast<T*>(&storage_);
    }

    void ObjectDestructor() override {
        Get()->~T();
    }

    ~ControlBlock2() noexcept = default;

private:
    std::aligned_storage_t<sizeof(T), alignof(T)> storage_;
};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

class ESFTBase {};

template <typename T>
class EnableSharedFromThis;
