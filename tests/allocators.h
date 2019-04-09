#ifndef DATASTRUCTURES_TESTS_ALLOCATORS_H
#define DATASTRUCTURES_TESTS_ALLOCATORS_H

namespace testing {

template <typename T>
class counting_allocator {
public:
    using value_type = T;
    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;

public:
    counting_allocator(unsigned * allocations, unsigned * deallocations)
     : m_allocations(allocations),
       m_deallocations(deallocations) {}

    template <typename U>
    counting_allocator(const counting_allocator<U> & other)
     : m_allocations(other.m_allocations),
       m_deallocations(other.m_deallocations) {}

    [[nodiscard]] T * allocate(std::size_t n)
    {
        if (m_allocations) { *m_allocations += 1; }
        return static_cast<T *>(::operator new(n * sizeof(T), static_cast<std::align_val_t>(alignof(T))));
    }
    void deallocate(T * ptr, std::size_t)
    {
        if (m_deallocations) { *m_deallocations += 1; }
        ::operator delete(ptr, static_cast<std::align_val_t>(alignof(T)));
    }

    friend bool operator==(const counting_allocator & lhs, const counting_allocator & rhs)
    {
        return lhs.m_allocations == rhs.m_allocations && lhs.m_deallocations == rhs.m_deallocations;
    }
    friend bool operator!=(const counting_allocator & lhs, const counting_allocator & rhs)
    {
        return !(lhs == rhs);
    }

private:
    unsigned * m_allocations = nullptr;
    unsigned * m_deallocations = nullptr;

    template <typename U> friend class counting_allocator;
};


} // namespace testing

#endif
