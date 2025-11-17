//
// Created by y.grallan on 17/11/2025.
// Copyright (c) 2025 Yann Grallan All rights reserved.
//

#pragma once
#include <algorithm>
#include <array>
#include <list>
#include <stack>
#include <stdexcept>


template<typename Ty_, size_t BlockSize_ = 16ull>
class Colony
{
    enum class State
    {
        Empty,
        Occupied,
        Garbage
    };

    template<size_t Size = sizeof(Ty_)>
    class Cell
    {
        alignas(Ty_) std::array<std::byte, Size> object;
        State state = State::Empty;

    public:
        Cell() = default;

        template<typename T = Ty_>
        requires (!std::same_as<std::remove_cvref_t<T>, Cell>)
        explicit Cell(T&& object_) : state(State::Occupied)
        {
            std::construct_at(reinterpret_cast<Ty_*>(&object), std::forward<T>(object_));
        }

        template<typename T = Ty_>
        requires (!std::same_as<std::remove_cvref_t<T>, Cell>)
        void set_object(T&& object_)
        {
            state = State::Occupied;
            std::construct_at(reinterpret_cast<Ty_*>(&object), std::forward<T>(object_));
        }

        Ty_* get()
        {
            return reinterpret_cast<Ty_ *>(&object);
        }

        const Ty_* get() const
        {
            return reinterpret_cast<Ty_ *>(&object);
        }


        Ty_* operator*()
        {
            return *(get());
        }

        const Ty_* operator*() const
        {
            return *(get());
        }

        Ty_* operator->()
        {
            return get();
        }

        const Ty_* operator->() const
        {
            return get();
        }


        bool is_free() const noexcept
        {
            return !(state == State::Occupied);
        }

        void free()
        {
            get()->~Ty_();
            state = State::Garbage;
        }
    };


    template<size_t BlockSize>
    class Block
    {
        std::array<Cell<>, BlockSize> cell_array;
        size_t real_size = 0ull;
        std::stack<Cell<>*> free_list;

    public:
        Block()
        {
            for (int i = BlockSize - 1; i > -1; --i)
            {
                free_list.push(&cell_array[i]);
            }
        };

        bool is_full() const noexcept
        {
            return free_list.empty();
        }

        template<typename T = Ty_>
        bool insert(T&& element)
        {
            if (is_full())
            {
                return false;
            }


            auto emplacement = free_list.top();
            free_list.pop();

            emplacement->set_object(std::forward<T>(element));
            ++real_size;
            return true;
        }



        Ty_* get_at(const size_t index)
        {
            if (index >= real_size)
            {
                throw std::out_of_range("Index out of range");
            }

            int id = 0;
            for (auto & element: cell_array)
            {
                if (!element.is_free())
                {
                    if (id++ == index)
                    {
                        return element.get();
                    }
                }
            }

            return nullptr;
        }


        void remove(const size_t index)
        {
            if (index >= real_size)
            {
                throw std::out_of_range("Index out of range");
            }

            int id = 0;
            for (auto & element: cell_array)
            {
                if (!element.is_free())
                {
                    if (id++ == index)
                    {
                        element.free();
                        free_list.push(&element);
                        --real_size;
                        return;
                    }
                }
            }
        }

        size_t get_real_size() const noexcept
        {
            return real_size;
        }
    };

    std::pair<Block<BlockSize_>*, size_t> get_block(int index)
    {
        auto block = colony_array.begin();
        index -= block->get_real_size();


        while (index > 0)
        {
            ++block;
            if (block == colony_array.end())
            {
                throw std::out_of_range("Index out of range");
            }

            index -= block->get_real_size();
        }
        return {&(*block), index + block->get_real_size()};
    }


    using FreeList = std::list<Block<BlockSize_>*>;


    size_t current_size = 0ull;
    std::list<Block<BlockSize_>> colony_array;
    FreeList free_list;

public:
    Colony() = default;

    template<typename T = Ty_>
    void insert_back(T&& element)
    {
        if (free_list.empty())
        {
            colony_array.emplace_back();
            free_list.push_back(&colony_array.back());
        }

        Block<BlockSize_>* block = free_list.back();
        block->insert(std::forward<T>(element));
        ++current_size;

        if (block->is_full())
        {
            free_list.remove(block);
        }
    }

    void remove(const size_t index)
    {
        auto block = get_block(index);
        block.first->remove(block.second);
        --current_size;
    }

};
