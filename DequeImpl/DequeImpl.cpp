#include <memory>
#include <utility>
#include <iostream>

template<typename T, size_t DequeSize_ = 100ull, size_t ChunkSize_ = 8ull>
class Deque
{
	class Chunk
	{
		T* data{ nullptr };
		size_t start_chunk_index{ ChunkSize_ };
		size_t end_chunk_index{ 0ull };
		static constexpr size_t invalid_index = ChunkSize_ + 1;

	public:
		friend void swap(Chunk& first, Chunk& second) noexcept
		{
			using std::swap;
			swap(first.data, second.data);
			swap(first.start_chunk_index, second.start_chunk_index);
			swap(first.end_chunk_index, second.end_chunk_index);
		}

	private:

		void copy_resources_from(const Chunk& source)
		{
			data = static_cast<T*>(::operator new(sizeof(T) * ChunkSize_));
			start_chunk_index = source.start_chunk_index;
			end_chunk_index = source.end_chunk_index;

			if (source.is_front_filled())
			{
				for (size_t i = source.start_chunk_index; i < ChunkSize_; ++i)
				{
					std::construct_at(&data[i], source.data[i]);
				}
			}
			else
			{
				for (size_t i = 0; i < source.end_chunk_index; ++i)
				{
					std::construct_at(&data[i], source.data[i]);
				}
			}
		}

		template<typename U = T>
		void allocate_at(size_t index, U&& element)
		{
			std::construct_at(&data[index], std::forward<U>(element));
		}

		void deallocate_at(size_t index)
		{
			data[index].~T();
		}

	public:

		Chunk()
		{
			data = static_cast<T*>(::operator new(sizeof(T) * ChunkSize_));
		}

		~Chunk()
		{
			if (!data) return;

			if (is_front_filled())
			{
				for (size_t i = start_chunk_index; i < ChunkSize_; ++i)
				{
					data[i].~T();
				}
			}
			else
			{
				for (size_t i = 0; i < end_chunk_index; ++i)
				{
					data[i].~T();
				}
			}

			::operator delete(data);
		}

		Chunk(const Chunk& other)
		{
			copy_resources_from(other);
		}

		Chunk& operator=(Chunk other)
		{
			swap(*this, other);
			return *this;
		}


		[[nodiscard]] bool is_front_filled() const noexcept
		{
			return end_chunk_index == invalid_index || (end_chunk_index == 0 && start_chunk_index == ChunkSize_);
		}

		[[nodiscard]] size_t size() const noexcept
		{
			if (is_front_filled())
			{
				return ChunkSize_ - start_chunk_index;
			}
			return end_chunk_index;
		}

		[[nodiscard]] bool is_full() const noexcept
		{
			return end_chunk_index == ChunkSize_ || start_chunk_index == 0;
		}

		[[nodiscard]] bool is_empty() const noexcept
		{
			return end_chunk_index == 0 || start_chunk_index == ChunkSize_;
		}


		template<typename U = T>
		void push_back(U&& element)
		{
			start_chunk_index = invalid_index;
			allocate_at(end_chunk_index, std::forward<U>(element));
			++end_chunk_index;
		}

		template<typename U = T>
		void push_front(U&& element)
		{
			end_chunk_index = invalid_index;
			--start_chunk_index;
			allocate_at(start_chunk_index, std::forward<U>(element));
		}

		void pop_back()
		{
			deallocate_at(--end_chunk_index);
		}

		void pop_front()
		{
			deallocate_at(start_chunk_index++);
		}

		[[nodiscard]] T& front()
		{
			return data[start_chunk_index];
		}
		[[nodiscard]] const T& front() const
		{
			return data[start_chunk_index];
		}

		[[nodiscard]] T& back()
		{
			return data[end_chunk_index - 1];
		}
		[[nodiscard]] const T& back() const
		{
			return data[end_chunk_index - 1];
		}

		[[nodiscard]] T& get_at(size_t index)
		{
			if (is_front_filled())
			{
				return data[start_chunk_index + index];
			}
			else
			{
				return data[index];
			}
		}
		
		[[nodiscard]] const T& get_at(size_t index) const
		{
			if (is_front_filled())
			{
				return data[start_chunk_index + index];
			}
			else
			{
				return data[index];
			}
		}		
	};


	static constexpr size_t starting_pos = (DequeSize_ + 2 - 1) / 2;

	size_t chunk_start_index{ starting_pos };
	size_t chunk_end_index{ starting_pos };
	size_t current_size{ 0ull };

	Chunk* map[DequeSize_]{};
		
	void allocate_first_chunk()
	{
		allocate_at(starting_pos);
	}

	void allocate_at(size_t index)
	{
		if (map[index] == nullptr)
		{
			map[index] = new Chunk();
		}
	}

	template<typename U = T>
	void push_at(size_t& index, U&& element, const bool is_front_push)
	{
		if (map[index]->is_full())
		{
			if ((is_front_push && static_cast<int>(index) - 1 < 0) ||
				(!is_front_push && index + 1 > DequeSize_))
			{
				throw std::bad_alloc();
			}
			
			allocate_at(is_front_push ? --index : ++index);
		}

		if (is_front_push)
		{
			map[index]->push_front(std::forward<U>(element));
		}
		else
		{			
			map[index]->push_back(std::forward<U>(element));
		}
		++current_size;
	}

	void pop_at(size_t& index, const bool is_front_pop)
	{
		if (is_front_pop)
		{
			map[index]->pop_front();
		}
		else
		{
			map[index]->pop_back();
		}
		
		if (map[index]->is_empty())
		{
			is_front_pop ? ++index : --index;
		}
		
		current_size--;
	}
	
public:
	Deque()
	{

	}
	Deque(const Deque& other) : chunk_end_index(other.chunk_end_index), chunk_start_index(other.chunk_start_index), current_size(other.current_size)
	{
		for (int i = 0; i < DequeSize_; ++i)
		{
			if (other.map[i])
			{
				map[i] = new Chunk(*other.map[i]);
			}
		}
	}
	
	Deque& operator=(Deque deque)
	{
		swap(*this,deque);
		return *this;
	}
	
	~Deque()
	{
		for (size_t i = 0; i < DequeSize_; ++i)
		{
			delete map[i];
		}
	}

	friend void swap(Deque& first, Deque& second) noexcept
	{
		using std::swap;

		swap(first.chunk_end_index, second.chunk_end_index);
		swap(first.chunk_start_index, second.chunk_start_index);
		swap(first.current_size, second.current_size);
		swap(first.map, second.map);
	}

	[[nodiscard]] size_t size() const noexcept
	{
		return current_size;
	}
	
	[[nodiscard]] bool is_empty() const noexcept
	{
		return size() == 0ull;
	}

	template<typename U = T>
	void push_back(U&& element)
	{
		if (is_empty())
		{
			allocate_first_chunk();
			allocate_at(--chunk_start_index);
		}

		push_at(chunk_end_index, std::forward<U>(element), false);	
	}
	
	template<typename U = T>
	void push_front(U&& element)
	{
		if (is_empty())
		{
			allocate_first_chunk();
			allocate_at(++chunk_end_index);
		}

		push_at(chunk_start_index, std::forward<U>(element), true);		
	}

	void pop_back()
	{
		pop_at(chunk_end_index, false);
	}

	void pop_front()
	{
		pop_at(chunk_start_index, true);
	}

	[[nodiscard]] T& front() 
	{
		if (is_empty())
		{
			throw std::out_of_range("deque is empty, can't access element");
		}
		return map[chunk_start_index]->front();
	}
	[[nodiscard]] const T& front() const 
	{
		if (is_empty())
		{
			throw std::out_of_range("deque is empty, can't access element");
		}
		return map[chunk_start_index]->front();
	}
	
	[[nodiscard]] T& back() 
	{
		if (is_empty())
		{
			throw std::out_of_range("deque is empty, can't access element");
		}
		return map[chunk_end_index]->back();
	}
	[[nodiscard]] const T& back() const 
	{
		if (is_empty())
		{
			throw std::out_of_range("deque is empty, can't access element");
		}
		return map[chunk_end_index]->back();
	}
	
	[[nodiscard]] T& get_at(size_t index)
	{
		if (is_empty())
		{
			throw std::out_of_range("deque is empty, can't access element");
		}
		
		Chunk* first_chunk = map[chunk_start_index];
		const size_t first_chunk_size = first_chunk->size();

		if (index < first_chunk_size)
		{
			return first_chunk->get_at(index);
		}

		const size_t remaining_index = index - first_chunk_size;
		const size_t chunk_offset = remaining_index / ChunkSize_;
		size_t internal_index = remaining_index % ChunkSize_;

		size_t target_map_index = chunk_start_index + 1 + chunk_offset;

		return map[target_map_index]->get_at(internal_index);
	}
	[[nodiscard]] const T& get_at(size_t index) const
	{
		if (is_empty())
		{
			throw std::out_of_range("deque is empty, can't access element");
		}
		
		Chunk* first_chunk = map[chunk_start_index];
		const size_t first_chunk_size = first_chunk->size();

		if (index < first_chunk_size)
		{
			return first_chunk->get_at(index);
		}

		const size_t remaining_index = index - first_chunk_size;
		const size_t chunk_offset = remaining_index / ChunkSize_;
		size_t internal_index = remaining_index % ChunkSize_;

		size_t target_map_index = chunk_start_index + 1 + chunk_offset;

		return map[target_map_index]->get_at(internal_index);
	}

	[[nodiscard]] T& operator[](const size_t index)
	{
		return get_at(index);
	}
	[[nodiscard]] const T& operator[](const size_t index) const
	{
		return get_at(index);
	}
};



int main()
{
	Deque<int> a;

	for (int i = 0; i < 5; ++i)
	{		
		a.push_back(i);
		a.push_front(i * 5);
	}

	for (int i = 0; i < a.size(); ++i)
	{
		std::cout << a[i] << " ";
	}

	Deque<int> b = a;
	
	return 0;
}