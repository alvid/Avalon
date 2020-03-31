#pragma once

#include <iostream>
#include <vector>
#include <stdexcept>
#include <mutex>

template <typename T>
class MtStack {
public:
    explicit MtStack(std::string const& a_name);

    MtStack(MtStack const& ref) noexcept;
    MtStack(MtStack&& ref) noexcept;

    MtStack& operator=(MtStack const& ref) noexcept;
    MtStack& operator=(MtStack&& ref) noexcept;

    bool empty() const;
    size_t size() const;
    void push(T elem);
    void pop(T& elem);
    std::unique_ptr<T> pop();
    void clear();

private:
    std::string name;
    std::vector<T> elems;
    mutable std::mutex mt; 
};

template <typename T>
MtStack<T>::MtStack(std::string const& a_name) 
    : name(a_name)
{
    std::cout << "stack[" << name << "]: created" << std::endl;
}

template <typename T>
MtStack<T>::MtStack(MtStack<T> const& ref) noexcept
{
    std::unique_lock lock(ref.mt);

    std::cout << "stack[" << name << "]: copy ctor with " << "stack[" << ref.name << "]" << std::endl;

    elems = ref.elems;
    name = "copy of " + ref.name;
}

template <typename T>
MtStack<T>::MtStack(MtStack<T>&& ref) noexcept
{
    std::lock(ref.mt, mt);
    std::unique_lock lock1(ref.mt, std::adopt_lock);
    std::unique_lock lock2(mt, std::adopt_lock);

    std::cout << "stack[" << name << "]: move ctor with " << "stack[" << ref.name << "]" << std::endl;

    elems = ref.elems;
    ref.elems.clear();
    std::swap(ref.name, name);
}

template<typename T>
MtStack<T>& MtStack<T>::operator=(MtStack<T> const& ref) noexcept
{
    if (this != &ref) {
        std::cout << "stack[" << name << "]: assigment to " << "stack[" << ref.name << "]" << std::endl;

        std::lock(ref.mt, mt);
        std::unique_lock lock1(ref.mt, std::adopt_lock);
        std::unique_lock lock2(mt, std::adopt_lock);

        elems = ref.elems;
        name = "copy of " + ref.name;
    }
    return *this;
}

template<typename T>
MtStack<T>& MtStack<T>::operator=(MtStack<T>&& ref) noexcept
{
    if (this != &ref) {
        std::cout << "stack[" << name << "]: replaced by " << "stack[" << ref.name << "]" << std::endl;

        std::lock(ref.mt, mt);
        std::unique_lock lock1(ref.mt, std::adopt_lock);
        std::unique_lock lock2(mt, std::adopt_lock);

        elems.clear();
        elems.swap(ref.elems);

        std::swap(ref.name, name);
    }
    return *this;
}

template <typename T>
bool MtStack<T>::empty() const
{
    std::unique_lock lock(mt);
    return elems.empty();
}

template <typename T>
size_t MtStack<T>::size() const
{
    std::unique_lock lock(mt);
    return elems.size();
}

template <typename T>
void MtStack<T>::push(T elem)
{
    std::unique_lock lock(mt);
    elems.push_back(elem);
}

template <typename T>
void MtStack<T>::pop(T& elem)
{
    std::unique_lock lock(mt);

    if (elems.empty())
        throw std::out_of_range("MtStack::pop(T&): is empty");

    elem = elems.back();
    elems.pop_back();
}

template <typename T>
std::unique_ptr<T> MtStack<T>::pop()
{
    std::unique_lock lock(mt);

    if (elems.empty())
        throw std::out_of_range("MtStack::pop(): is empty");

    auto elem = std::make_unique<T>(elems.back());
    elems.pop_back();

    return elem;
}

template <typename T>
void MtStack<T>::clear()
{
    std::unique_lock lock(mt);
    elems.clear();
}

