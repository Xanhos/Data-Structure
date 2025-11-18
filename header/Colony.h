//
// Created by y.grallan on 17/11/2025.
// Copyright (c) 2025 Yann Grallan All rights reserved.
//

#pragma once
#include <algorithm>
#include <array>
#include <forward_list>
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

    class Cell
    {
        static constexpr size_t Size = sizeof(Ty_);
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


    template<size_t BlockSize, bool auto_delete = true>
    class Block
    {
        std::array<Cell, BlockSize> cell_array;
        std::stack<Cell*> free_list;
        size_t real_size = 0ull;

        Block* next = nullptr;
    public:
        Block()
        {
            for (int i = BlockSize - 1; i > -1; --i)
            {
                free_list.push(&cell_array[i]);
            }
        };

        ~Block()
        {
            if constexpr(auto_delete)
            {
                delete next;
            }
        }

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

        bool is_tail() const noexcept
        {
            return next == nullptr;
        }

        void set_next(Block* block)
        {
            next = block;
        }

        Block* get_next()
        {
            return next;
        }
    };

    std::pair<Block<BlockSize_>*, size_t> get_block(int index)
    {
        auto block_it = colony_array;

        while (block_it)
        {
            const size_t size_of_this_block = block_it->get_real_size();

            if (index < size_of_this_block)
            {
                return { block_it, index };
            }

            index -= size_of_this_block;

            block_it = block_it->get_next();
        }
         throw std::out_of_range("Index out of range");
    }


    using FreeList = std::list<Block<BlockSize_>*>;

    using BlockType = Block<BlockSize_>;

    size_t current_size = 0ull;
    BlockType* colony_array = nullptr;
    FreeList free_list;


    void allocate_new_block()
    {
        auto new_block = new BlockType();

        auto it = colony_array;
        if (!colony_array)
        {
            colony_array = new_block;
        }
        else
        {
            while (!it->is_tail())
            {
                it = it->get_next();
            }
            it->set_next(new_block);
        }
        free_list.push_back(new_block);
    }

public:
    Colony() = default;
    ~Colony()
    {
        delete colony_array;
        colony_array = nullptr;
    }

    template<typename T = Ty_>
    void insert_back(T&& element)
    {
        if (free_list.empty())
        {
            allocate_new_block();
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
        if (!std::ranges::any_of(free_list, [&](const BlockType* free_block) {return free_block == block.first;}))
        {
            free_list.push_back(block.first);
        }
        --current_size;
    }

    Ty_& get_at(const size_t index)
    {
        auto block = get_block(index);
        return *block.first->get_at(block.second);
    }

    size_t size() const noexcept
    {
        return current_size;
    }

    class Iterator : std::iterator<std::forward_iterator_tag, Ty_>
    {
        using pointer = Ty_*;
        using reference = Ty_&;

        size_t index = 0ull;
        Colony* colony;
    public:
        Iterator() = default;
        explicit Iterator(Colony* colony_, size_t index_) : colony(colony_), index(index_){}

        reference operator*()
        {
            return colony->get_at(index);
        }

        pointer operator->()
        {
            return &colony->get_at(index);
        }

        bool operator==(const Iterator & other) const
        {
            return index == other.index && colony == other.colony;
        }

        bool operator!=(const Iterator &) const = default;
        auto operator<=>(const Iterator&) const = default;

        Iterator& operator++()
        {
            ++index;
            return *this;
        }

        Iterator operator++(int)
        {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }
    };

    Iterator begin()
    {
        return Iterator(this, 0);
    }

    Iterator end()
    {
        return Iterator(this, current_size);
    }
};
