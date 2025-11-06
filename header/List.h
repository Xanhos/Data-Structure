//
// Created by y.grallan on 06/11/2025.
//

#pragma once
#include <utility>
#include <iostream>

template<typename T>
class List
{

    struct Node
    {
        T data;
        Node* next;

        ~Node() = default;
    };

    Node* head = nullptr;
    Node* cursor = nullptr;
    size_t size = 0ull;

    Node* get_last_node()
    {
        if (!head)
            return nullptr;

        auto it = head;
        while (it->next)
        {
            it = it->next;
        }
        return it;
    }

public:
    List() = default;
    List(const List& other)
    {
        if (!other.head)
            return;

        auto it = other.head;
        while (it)
        {
            push_back(it->data);
            it = it->next;
            if (other.cursor == it)
            {
                cursor = get_last_node();
            }
        }
    }

    List& operator=(List other)
    {
        swap(*this,other);
        return *this;
    }

    ~List()
    {
      reset();
    }

    friend void swap(List& first, List& second) noexcept
    {
        using std::swap;
        std::swap(first.size,second.size);
        std::swap(first.head,second.head);
        std::swap(first.cursor,second.cursor);
    }

    template<typename U = T>
    T* push_back(U&& element)
    {
        auto new_node = new Node({.data = (std::forward<U>(element)), .next = nullptr});
        size++;
        if (!head)
        {
            head = new_node;
            cursor = new_node;
            return &head->data;
        }

        get_last_node()->next = new_node;

        return &new_node->data;
    }

    void move_forward_cursor()
    {
        if (cursor && cursor->next)
        {
            cursor = cursor->next;
        }
        else if (!cursor)
        {
            reset_cursor();
        }
    }

    void reset_cursor()
    {
        cursor = head;
    }

    T* insert_after_cursor(T element)
    {
        if (!cursor)
            return nullptr;

        auto new_node = new Node({.data = (std::move(element)), .next = nullptr});

        if (cursor->next)
        {
            new_node->next = cursor->next;
            cursor->next = new_node;
        }
        else
        {
            cursor->next = new_node;
        }
        size++;

        return &new_node->data;
    }

    void print_list(const std::string& list_name = "")
    {
        if (!list_name.empty())
        {
            std::cout << "List " << list_name << " : ";
        }

        auto it = head;
        while (it)
        {
            std::cout << it->data << " ";
            it = it->next;
        }
        std::cout << "\n";
    }

    void reset()
    {
        auto it = head;
        while (it)
        {
            auto temp = it->next;
            delete it;
            it = temp;
        }

        head = nullptr;
        cursor = nullptr;
    }
};
