//
// Created by y.grallan on 06/11/2025.
//
#pragma once
#include <stdexcept>
#include <utility>

template<typename T>
class Vector
{
    T* array = nullptr;

    size_t capacity = 0ull;
    size_t size = 0ull;

    void reserve_impl(const size_t new_capacity,const bool is_from_shrink_to_fit)
    {
        if (capacity >= new_capacity && !is_from_shrink_to_fit)
            return;

        T* new_array = static_cast<T*>(::operator new(new_capacity * sizeof(T)));

        for (size_t i = 0; i < size; ++i)
        {
            new (&new_array[i]) T(std::move(array[i]));
        }

        ::operator delete[](array);

        array = new_array;
        capacity = new_capacity;
    }

public:
    Vector()
    {
        reserve(3);
    }
    Vector(const Vector& other)
    {
        reserve(other.capacity);
        for (size_t i = 0; i < other.size; i++)
        {
            push_back(other[i]);
        }
    }

    Vector& operator=(Vector other)
    {
        swap(*this, other);
        return *this;
    }

    friend void swap(Vector& first, Vector& second) noexcept
    {
        using std::swap;
        swap(first.array, second.array);
        swap(first.capacity, second.capacity);
        swap(first.size, second.size);
    }

    [[nodiscard]] size_t get_size() const noexcept
    {
        return size;
    }

    [[nodiscard]] size_t get_capacity() const noexcept
    {
        return capacity;
    }

    [[nodiscard]] bool is_empty() const noexcept
    {
        return size == 0ull;
    }

    void reserve(const size_t new_capacity)
    {
        reserve_impl(new_capacity, false);
    }

    template<typename U = T>
    void push_back(U&& element)
    {
        if (size >= capacity)
        {
            reserve(capacity * 2);
        }
        array[size++] = std::forward<U>(element);
    }

    void erase_swap(const size_t index)
    {
        array[index].~T();
        std::swap(array[index],array[--size]);
    }

    void pop_back()
    {
        if (size)
        {
            array[size--].~T();
        }
    }

    void clear()
    {
        for (size_t i = 0ull; i < size; ++i)
        {
            array[i].~T();
        }
        size = 0;
    }

    void resize(const size_t count, const T& initializer = T())
    {
        if (count == size)
            return;

        const int current_size = size;
        if (count > size)
        {
            for (int i = 0; i < count - current_size; ++i)
            {
                push_back(initializer);
            }
        }
        else
        {
            for (int i = current_size - 1; i < count - 1; ++i)
            {
                array[i].~T();
            }
        }
        size = count;
    }


    void shrink_to_fit()
    {
        reserve_impl(size, true);
    }

    T& operator[](size_t index)
    {
        if (index >= size || is_empty())
        {
            throw std::out_of_range("index out of bound");
        }

        return array[index];
    }
};
