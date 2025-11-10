//
// Created by y.grallan on 10/11/2025.
// Copyright (c) 2025 Yann Grallan All rights reserved.
//

#pragma once
#include <list>
#include <optional>
#include <vector>
#include <utility>

namespace ClosedHashMap
{

    template<typename K, typename V>
    class HashMap
    {
        class Bucket
        {

        public:
            using Pair = std::pair<K,V>;

            Bucket() = default;
            Bucket(const Bucket& other)
            {
                for (auto& element: other.get_all_elements())
                {
                    insert(element.first, element.second);
                }
            }

            friend void swap(Bucket& first, Bucket& second) noexcept
            {
                using std::swap;
                swap(first.pair_list, second.pair_list);
            }

            Bucket& operator=(Bucket other)
            {
                swap(*this,other);
                return *this;
            }

            const std::list<Pair>& get_all_elements() const
            {
                return pair_list;
            }

            std::list<Pair>& get_all_elements()
            {
                return pair_list;
            }

            std::optional<std::reference_wrapper<V>> get_at(const K& key)
            {
                for (auto& element: pair_list)
                {
                    if (element.first == key)
                    {
                        return element.second;
                    }
                }
                return std::nullopt;
            }

            template<typename Key_ = K,typename Value_ = V>
            void insert(Key_&& key, Value_&& value)
            {
                for (auto& element: pair_list)
                {
                    if (element.first == key)
                    {
                        element.second = std::forward<Value_>(value);
                        return;
                    }
                }
                pair_list.push_back(std::pair{std::forward<Key_>(key), std::forward<Value_>(value)});
            }

            void erase(const K& key)
            {
                pair_list.remove_if([&](const auto& element){return element.first == key;});
            }

            [[nodiscard]] bool is_empty() const noexcept
            {
                return pair_list.empty();
            }

        private:
            std::list<Pair> pair_list;

        };

        std::vector<Bucket> m_map;
        size_t real_size = 0ull;

        size_t get_hash(const K& key)
        {
            size_t hash_index = std::hash<K>{}(key);
            hash_index %= m_map.size();
            return hash_index;
        }

        std::pair<K,V>& get_at_index(size_t index)
        {
            if (index >= real_size)
            {
                throw std::out_of_range("Iterator out of range");
            }

            size_t id = 0ull;
            for (auto & element: m_map)
            {
                if (!element.is_empty())
                {
                    for (auto& pair: element.get_all_elements())
                    {
                        if (id++ == index)
                        {
                            return pair;
                        }
                    }
                }
            }

            throw std::out_of_range("Iterator out of range");
        }

    public:

        HashMap()
        {
            m_map.resize(10);
        }

        template<typename Key_ = K, typename Value_ = V>
        void insert(Key_&& key, Value_&& value)
        {
            m_map[get_hash(key)].insert(std::forward<Key_>(key), std::forward<Value_>(value));
            ++real_size;
        }

        V& find(const K& key)
        {
            auto result = m_map[get_hash(key)].get_at(key);
            if (!result)
            {
                throw std::out_of_range("Key doesn't exist");
            }
            return *result;
        }

        void remove(const K& key)
        {
            m_map[get_hash(key)].erase(key);
            --real_size;
        }

        void rehash()
        {
            real_size = 0ull;
            auto previous = m_map;

            m_map.clear();
            m_map.resize(previous.size() * 2);

            for (auto & bucket: previous)
            {
                for (auto& element: bucket.get_all_elements())
                {
                    insert(std::move(element.first), std::move(element.second));
                }
            }
        }

        friend class Iterator;

        template<typename Type>
        class Iterator : public std::iterator<std::bidirectional_iterator_tag, Type>
        {
            HashMap* map = nullptr;
            size_t index = 0ull;

            using Reference = Type&;
            using Pointer = Type*;
        public:


            Iterator() = default;
            Iterator(HashMap* map_, const size_t index_) : map(map_), index(index_)
            {}

            Reference operator*()
            {
                return map->get_at_index(index);
            }

            Pointer operator->()
            {
                return &map->get_at_index(index);
            }

            bool operator==(const Iterator & other) const
            {
                return other.map == map && index == other.index;
            }

            bool operator!=(const Iterator & other) const
            {
                return !(other == *this);
            }

            bool operator<=>(const Iterator& other) const = default;

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


        Iterator<std::pair<K,V>> begin()
        {
            return Iterator<std::pair<K,V>>(this, 0);
        }

        Iterator<std::pair<K,V>> end()
        {
            return Iterator<std::pair<K,V>>(this, real_size);
        }
    };
}


namespace OpenHashMap
{
    template<typename K, typename V>
    class HashMap
    {
        enum class State {EMPTY, OCCUPIED, TRASH};

        struct Cell
        {
            State state = State::EMPTY;
            std::pair<K,V> pair;
        };

        std::vector<Cell> m_map;
        size_t real_size = 0ull;

        size_t get_hash(const K& key)
        {
            size_t hash_index = std::hash<K>{}(key);
            hash_index %= m_map.size();
            return hash_index;
        }

        template<typename Key_ = K, typename Value_ = V>
        void insert_at(size_t index, Key_&& key, Value_&& value)
        {
            if (key == K())
                return;

            if (index >= m_map.size())
            {
                rehash();
                insert(std::forward<Key_>(key), std::forward<Value_>(value));
                return;
            }

            if (m_map[index].state == State::EMPTY || m_map[index].state == State::TRASH)
            {
                m_map[index].state = State::OCCUPIED;
                m_map[index].pair.first = std::forward<Key_>(key);
                m_map[index].pair.second = std::forward<Value_>(value);
                ++real_size;
                return;
            }
            else
            {
                insert_at(index + 1, std::forward<Key_>(key), std::forward<Value_>(value));
            }
        }

        void rehash()
        {
            real_size = 0;
            auto previous = m_map;
            m_map.clear();
            m_map.resize(previous.size() * 2);

            for (auto& element: previous)
            {
                insert(std::move(element.pair.first), std::move(element.pair.second));
            }
        }

        std::pair<K,V>& get_at_index(size_t index)
        {
            if (index >= real_size)
            {
                throw std::out_of_range("Iterator out of range");
            }

            size_t id = 0ull;
            for (auto & element: m_map)
            {
                if (element.state == State::OCCUPIED)
                {
                    if (index == id++)
                    {
                        return element.pair;
                    }
                }
            }

            throw std::out_of_range("Iterator out of range");
        }

    public:
        friend void swap(HashMap& first, HashMap& second) noexcept
        {
            using std::swap;
            swap(first.m_map, second.m_map);
        }

        HashMap() : m_map(10)
        {}

        HashMap(const HashMap& other)
        {
            for (auto& element: other.m_map)
            {

            }
        }

        HashMap& operator=(HashMap other)
        {
            swap(*this, other);
            return *this;
        }

        template<typename Key_ = K,typename Value_ = V>
        void insert(Key_&& key, Value_&& value)
        {
            size_t index = get_hash(key);
            insert_at(index, std::forward<Key_>(key), std::forward<Value_>(value));
        }

        void remove(const K& key)
        {
            for (size_t i = get_hash(key); i < m_map.size(); ++i)
            {
                if (m_map[i].pair.first == key)
                {
                    if(m_map[i].state == State::OCCUPIED && m_map[i].pair.first == key)
                    {
                        --real_size;
                        m_map[i].state = State::TRASH;
                        return;
                    }
                }
            }
        }

        V& find(const K& key)
        {
            for (size_t i = get_hash(key); i < m_map.size(); ++i)
            {
                if (m_map[i].pair.first == key)
                {
                    if(m_map[i].state == State::OCCUPIED && m_map[i].pair.first == key)
                    {
                        return m_map[i].pair.second;
                    }
                }
            }
            throw std::out_of_range("Key doesn't exist");
        }

        friend class Iterator;

        template<typename Type>
        class Iterator : public std::iterator<std::bidirectional_iterator_tag, Type>
        {
            HashMap* map = nullptr;
            size_t index = 0ull;

            using Reference = Type&;
            using Pointer = Type*;
        public:


            Iterator() = default;
            Iterator(HashMap* map_, const size_t index_) : map(map_), index(index_)
            {}

            Reference operator*()
            {
                return map->get_at_index(index);
            }

            Pointer operator->()
            {
                return &map->get_at_index(index);
            }

            bool operator==(const Iterator & other) const
            {
                return other.map == map && index == other.index;
            }

            bool operator!=(const Iterator & other) const
            {
                return !(other == *this);
            }

            bool operator<=>(const Iterator& other) const = default;

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

        Iterator<std::pair<K,V>> begin()
        {
            return Iterator<std::pair<K,V>>(this,0);
        }

        Iterator<std::pair<K,V>> end()
        {
            return Iterator<std::pair<K,V>>(this,real_size);
        }

    };
}