// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_CORE_DVECTOR_H
#define JACTORIO_INCLUDE_CORE_DVECTOR_H
#pragma once

#include <algorithm>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "jactorio.h"

#include "core/convert.h"
#include "data/cereal/serialize.h"

namespace jactorio
{
    // Adopt stl naming to be drop in replacement
    // ReSharper disable CppInconsistentNaming

    template <typename T>
    struct is_iterator
    {
        // No need to implement these methods, they are never actually called

        static char test(...); // False

        template <typename U,
                  typename = typename std::iterator_traits<U>::difference_type,
                  typename = typename std::iterator_traits<U>::pointer,
                  typename = typename std::iterator_traits<U>::reference,
                  typename = typename std::iterator_traits<U>::value_type,
                  typename = typename std::iterator_traits<U>::iterator_category>
        static long test(U&&); // True

        constexpr static bool value = std::is_same_v<decltype(test(std::declval<T>())), long>;
    };
    template <typename T>
    constexpr bool is_iterator_v = is_iterator<T>::value;


    /// Specifies traits of iterator
    template <typename ValueT, typename DifferenceT, typename PointerT, typename ConstPointerT>
    struct DVectorIteratorTypes
    {
        using iterator_category = std::random_access_iterator_tag;
        using value_type        = ValueT;
        using difference_type   = DifferenceT;
        using pointer           = PointerT;
        using const_pointer     = ConstPointerT;
    };

    /// \tparam TypesT DVectorIteratorTypes
    template <typename TypesT>
    class DVectorIterator
    {
    public:
        using iterator_category = typename TypesT::iterator_category;
        using value_type        = typename TypesT::value_type;
        using difference_type   = typename TypesT::difference_type;
        using pointer           = typename TypesT::pointer;
        using const_pointer     = typename TypesT::const_pointer;
        using reference         = value_type&;

        explicit DVectorIterator(const pointer ptr) noexcept : ptr_(ptr) {}

        operator auto() noexcept {
            return DVectorIterator<DVectorIteratorTypes<const value_type,
                                                        difference_type,
                                                        typename TypesT::const_pointer,
                                                        typename TypesT::const_pointer>>(ptr_);
        }

        J_NODISCARD pointer GetPointer() const noexcept {
            return ptr_;
        }

        // Access

        J_NODISCARD reference operator*() noexcept {
            assert(ptr_ != nullptr);
            return *ptr_;
        }

        J_NODISCARD pointer operator->() noexcept {
            assert(ptr_ != nullptr);
            return ptr_;
        }

        // Increment

        DVectorIterator& operator++() noexcept {
            ++ptr_;
            return *this;
        }

        DVectorIterator operator++(int) noexcept {
            auto temp = *this;
            ++*this;
            return temp;
        }

        DVectorIterator& operator--() noexcept {
            --ptr_;
            return *this;
        }

        DVectorIterator operator--(int) noexcept {
            auto temp = *this;
            --*this;
            return temp;
        }

        DVectorIterator& operator+=(const difference_type offset) noexcept {
            ptr_ += offset;
            return *this;
        }

        J_NODISCARD DVectorIterator operator+(const difference_type offset) const noexcept {
            auto temp = *this;
            return temp += offset;
        }

        DVectorIterator& operator-=(const difference_type offset) noexcept {
            return *this += -offset;
        }

        J_NODISCARD DVectorIterator operator-(const difference_type offset) const noexcept {
            auto temp = *this;
            return temp -= offset;
        }

        J_NODISCARD reference operator[](const difference_type offset) const noexcept {
            return *(*this + offset);
        }

    private:
        pointer ptr_;
    };

    template <typename T, typename U>
    J_NODISCARD typename T::difference_type operator-(const DVectorIterator<T>& lhs,
                                                      const DVectorIterator<U>& rhs) noexcept {
        static_assert(std::is_same_v<typename T::difference_type, typename U::difference_type>,
                      "Both iterators should have same difference_type");
        return lhs.GetPointer() - rhs.GetPointer();
    }

    template <typename T, typename U>
    J_NODISCARD bool operator==(const DVectorIterator<T>& lhs, const DVectorIterator<U>& rhs) noexcept {
        return lhs.GetPointer() == rhs.GetPointer();
    }

    template <typename T, typename U>
    J_NODISCARD bool operator!=(const DVectorIterator<T>& lhs, const DVectorIterator<U>& rhs) noexcept {
        return !(lhs == rhs);
    }

    template <typename T, typename U>
    J_NODISCARD bool operator<(const DVectorIterator<T>& lhs, const DVectorIterator<U>& rhs) noexcept {
        return lhs.GetPointer() < rhs.GetPointer();
    }

    template <typename T, typename U>
    J_NODISCARD bool operator>(const DVectorIterator<T>& lhs, const DVectorIterator<U>& rhs) noexcept {
        return rhs < lhs;
    }

    template <typename T, typename U>
    J_NODISCARD bool operator<=(const DVectorIterator<T>& lhs, const DVectorIterator<U>& rhs) noexcept {
        return !(rhs < lhs);
    }

    template <typename T, typename U>
    J_NODISCARD bool operator>=(const DVectorIterator<T>& lhs, const DVectorIterator<U>& rhs) noexcept {
        return !(lhs < rhs);
    }


    /// \tparam Iter Forward iterator
    template <typename Iter>
    class DVectorReverseIterator
    {
    public:
        using iterator_category = typename Iter::iterator_category;
        using value_type        = typename Iter::value_type;
        using difference_type   = typename Iter::difference_type;
        using pointer           = typename Iter::pointer;
        using reference         = value_type&;

        explicit DVectorReverseIterator(const pointer ptr) noexcept : ptr_(ptr) {}

        operator auto() noexcept {
            return DVectorIterator<DVectorIteratorTypes<const value_type,
                                                        difference_type,
                                                        typename Iter::const_pointer,
                                                        typename Iter::const_pointer>>(ptr_);
        }

        /// Converts reverse iterator into forward iterator
        J_NODISCARD Iter base() noexcept {
            return Iter(ptr_);
        }

        J_NODISCARD pointer GetPointer() const noexcept {
            return ptr_;
        }

        // Access

        J_NODISCARD reference operator*() noexcept {
            assert(ptr_ != nullptr);
            return *ptr_;
        }

        J_NODISCARD pointer operator->() noexcept {
            assert(ptr_ != nullptr);
            return ptr_;
        }

        // Increment

        DVectorReverseIterator& operator++() noexcept {
            --ptr_;
            return *this;
        }

        DVectorReverseIterator operator++(int) noexcept {
            auto temp = *this;
            ++*this;
            return temp;
        }

        DVectorReverseIterator& operator--() noexcept {
            ++ptr_;
            return *this;
        }

        DVectorReverseIterator operator--(int) noexcept {
            auto temp = *this;
            --*this;
            return temp;
        }

        DVectorReverseIterator& operator+=(const difference_type offset) noexcept {
            ptr_ -= offset;
            return *this;
        }

        J_NODISCARD DVectorReverseIterator operator+(const difference_type offset) const noexcept {
            auto temp = *this;
            return temp += offset;
        }

        DVectorReverseIterator& operator-=(const difference_type offset) noexcept {
            return *this += -offset;
        }

        J_NODISCARD DVectorReverseIterator operator-(const difference_type offset) const noexcept {
            auto temp = *this;
            return temp -= offset;
        }

        J_NODISCARD reference operator[](const difference_type offset) const noexcept {
            return *(*this + offset);
        }

    private:
        pointer ptr_;
    };

    template <typename T, typename U>
    J_NODISCARD typename T::difference_type operator-(const DVectorReverseIterator<T>& lhs,
                                                      const DVectorReverseIterator<U>& rhs) noexcept {
        static_assert(std::is_same_v<typename T::difference_type, typename U::difference_type>,
                      "Both iterators should have same difference_type");
        return rhs.GetPointer() - lhs.GetPointer();
    }

    template <typename T, typename U>
    J_NODISCARD bool operator==(const DVectorReverseIterator<T>& lhs, const DVectorReverseIterator<U>& rhs) noexcept {
        return lhs.GetPointer() == rhs.GetPointer();
    }

    template <typename T, typename U>
    J_NODISCARD bool operator!=(const DVectorReverseIterator<T>& lhs, const DVectorReverseIterator<U>& rhs) noexcept {
        return !(lhs == rhs);
    }

    template <typename T, typename U>
    J_NODISCARD bool operator<(const DVectorReverseIterator<T>& lhs, const DVectorReverseIterator<U>& rhs) noexcept {
        return lhs.GetPointer() > rhs.GetPointer();
    }

    template <typename T, typename U>
    J_NODISCARD bool operator>(const DVectorReverseIterator<T>& lhs, const DVectorReverseIterator<U>& rhs) noexcept {
        return rhs < lhs;
    }

    template <typename T, typename U>
    J_NODISCARD bool operator<=(const DVectorReverseIterator<T>& lhs, const DVectorReverseIterator<U>& rhs) noexcept {
        return !(rhs < lhs);
    }

    template <typename T, typename U>
    J_NODISCARD bool operator>=(const DVectorReverseIterator<T>& lhs, const DVectorReverseIterator<U>& rhs) noexcept {
        return !(lhs < rhs);
    }


    /// Dual ended contiguous vector,
    /// almost resembles std::vector, some methods are removed
    /// - Allocator is not saved when serialized and is default constructed
    /// \tparam T Containing type
    /// \tparam Allocator Memory allocator type
    template <typename T, typename Allocator = std::allocator<T>>
    class DVector
    {
        /// Amount size is multiplied by when resizing up
        static constexpr auto kScalingFactor = 1.5f;

    public:
        using IndexT          = int64_t;
        using value_type      = T;
        using allocator_type  = Allocator;
        using size_type       = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference       = value_type&;
        using const_reference = const value_type&;
        using pointer         = typename std::allocator_traits<Allocator>::pointer;
        using const_pointer   = typename std::allocator_traits<Allocator>::const_pointer;

        using iterator = DVectorIterator<DVectorIteratorTypes<value_type, difference_type, pointer, const_pointer>>;
        using const_iterator =
            DVectorIterator<DVectorIteratorTypes<const value_type, difference_type, const_pointer, const_pointer>>;
        using reverse_iterator       = DVectorReverseIterator<iterator>;
        using const_reverse_iterator = DVectorReverseIterator<const_iterator>;

        // Constructors

        /// Constructs empty DVector
        DVector() noexcept(noexcept(Allocator())) : allocator_() {}

        /// Constructs empty DVector with given allocator
        explicit DVector(const Allocator& alloc) noexcept : allocator_(alloc) {}

        /// Constructs the container with count copies of elements with value value
        DVector(const size_type count, const T& value, const Allocator& alloc = Allocator()) : allocator_(alloc) {
            NoDestructAssign(count, value);
        }

        /// Constructs the container with count default-inserted instances of T
        /// No copies are made
        explicit DVector(const size_type count, const Allocator& alloc = Allocator()) : allocator_(alloc) {
            NoDestructAssign(count);
        }

        /// Constructs the container with the contents of the range [first, last)
        /// \tparam InputIt Must satisfy LegacyInputIterator
        template <class InputIt>
        DVector(const InputIt first,
                const InputIt last,
                const Allocator& alloc                         = Allocator(),
                std::enable_if_t<is_iterator_v<InputIt>, bool> = true)
            : allocator_(alloc) {
            NoDestructAssignItRange(first, last);
        }

        // Destructor

        ~DVector() {
            while (frontPtr_ != backPtr_) {
                std::allocator_traits<allocator_type>::destroy(allocator_, frontPtr_);
                ++frontPtr_;
            }
            if (data_ != nullptr) {
                // std::deallocate does NOT accept nullptr unlike delete
                allocator_.deallocate(data_, capacity_);
            }
        }

        // Copy constructors

        /// Constructs the container with the copy of the contents of other
        DVector(const DVector& other) : allocator_(other.allocator_) {
            CopyFromOther(other);
        }

        /// Constructs the container with the copy of the contents of other, using alloc as the allocator
        DVector(const DVector& other, const Allocator& alloc) : allocator_(alloc) {
            CopyFromOther(other);
        }

        // Move constructors

        /// Constructs the container with the contents of other using move semantics
        /// After the move, other is guaranteed to be empty()
        DVector(DVector&& other) noexcept
            : allocator_{other.allocator_},
              capacity_{other.capacity_},
              data_{other.data_},
              frontPtr_{other.frontPtr_},
              backPtr_{other.backPtr_},
              midpoint_{other.midpoint_} {
            other.data_     = nullptr;
            other.frontPtr_ = nullptr;
            other.backPtr_  = nullptr;
        }

        /// Using alloc as the allocator for the new container, moving the contents from other
        /// If alloc != other.get_allocator(), this results in an element-wise move
        DVector(DVector&& other, const Allocator& alloc) : allocator_(alloc) {
            if (alloc == other.get_allocator()) {
                capacity_ = other.capacity_;
                data_     = other.data_;
                frontPtr_ = other.frontPtr_;
                backPtr_  = other.backPtr_;
                midpoint_ = other.midpoint_;

                other.data_     = nullptr;
                other.frontPtr_ = nullptr;
                other.backPtr_  = nullptr;
            }
            else {
                reserve(other.capacity());

                for (auto it = other.begin(); it != other.end(); ++it) {
                    UncheckedEmplaceBack(std::move(*it));
                }
            }
        }

        // Initializer list constructor

        DVector(std::initializer_list<T> init_list, const Allocator& alloc = Allocator()) : allocator_(alloc) {
            NoDestructAssignInitList(init_list);
        }


        DVector& operator=(DVector other) {
            using std::swap;
            swap(*this, other);
            return *this;
        }

        friend void swap(DVector& lhs, DVector& rhs) noexcept(noexcept(lhs.swap(rhs))) {
            lhs.swap(rhs);
        }

        /// Replaces the contents with count copies of value value
        void assign(const size_type count, const T& value) {
            clear();
            NoDestructAssign(count, value);
        }

        /// Replaces the contents with copies of those in the range [first, last)
        /// InputIt must satisfy LegacyInputIterator
        template <class InputIt, std::enable_if_t<is_iterator_v<InputIt>, bool> = true>
        void assign(const InputIt first, const InputIt last) {
            clear();
            NoDestructAssignItRange(first, last);
        }

        /// Replaces the contents with the elements from the initializer list
        void assign(std::initializer_list<T> init_list) {
            clear();
            NoDestructAssignInitList(init_list);
        }

        J_NODISCARD allocator_type get_allocator() const noexcept {
            return allocator_;
        }

        // Element access

        /// Indices may be negative, see push_front, push_back
        /// \exception std::out_of_range If index out of range
        J_NODISCARD reference at(const IndexT index) {
            auto* target = midpoint_ + index;
            if (!ElementPointerValid(target))
                throw std::out_of_range("DVector element access out of range");

            return *target;
        }

        /// Indices may be negative, see push_front, push_back
        /// \exception std::out_of_range If index out of range
        J_NODISCARD const_reference at(const IndexT index) const {
            auto* target = midpoint_ + index;
            if (!ElementPointerValid(target))
                throw std::out_of_range("DVector element access out of range");

            return *target;
        }

        /// Indices may be negative, see push_front, push_back
        J_NODISCARD reference operator[](const IndexT index) noexcept {
            return *(midpoint_ + index);
        }

        /// Indices may be negative, see push_front, push_back
        J_NODISCARD const_reference operator[](const IndexT index) const noexcept {
            return *(midpoint_ + index);
        }

        /// Reference to the first element
        J_NODISCARD reference front() noexcept {
            return *frontPtr_;
        }

        /// Reference to the first element
        J_NODISCARD const_reference front() const noexcept {
            return *frontPtr_;
        }

        /// Reference to the last element
        J_NODISCARD reference back() noexcept {
            return *(backPtr_ - 1);
        }

        /// Reference to the last element
        J_NODISCARD const_reference back() const noexcept {
            return *(backPtr_ - 1);
        }

        /// Pointer to underlying data, contains size() elements
        J_NODISCARD T* data() noexcept {
            return frontPtr_;
        }

        /// Pointer to underlying data, contains size() elements
        J_NODISCARD const T* data() const noexcept {
            return frontPtr_;
        }

        // Iterators

        /// Iterator to first element
        /// If empty, the returned iterator will be equal to end()
        J_NODISCARD iterator begin() noexcept {
            return iterator(frontPtr_);
        }

        /// Iterator to first element
        /// If empty, the returned iterator will be equal to end()
        J_NODISCARD const_iterator begin() const noexcept {
            return const_iterator(frontPtr_);
        }

        /// Iterator to first element
        /// If empty, the returned iterator will be equal to end()
        J_NODISCARD const_iterator cbegin() const noexcept {
            return const_iterator(frontPtr_);
        }

        /// Iterator to one past the last element
        J_NODISCARD iterator end() noexcept {
            return iterator(backPtr_);
        }

        /// Iterator to one past the last element
        J_NODISCARD const_iterator end() const noexcept {
            return const_iterator(backPtr_);
        }

        /// Iterator to one past the last element
        J_NODISCARD const_iterator cend() const noexcept {
            return const_iterator(backPtr_);
        }

        /// Iterator to first element of reversed DVector
        /// If empty, the returned iterator will be equal to rend()
        J_NODISCARD reverse_iterator rbegin() noexcept {
            return reverse_iterator(backPtr_ - 1);
        }

        /// Iterator to first element of reversed DVector
        /// If empty, the returned iterator will be equal to rend()
        J_NODISCARD const_reverse_iterator rbegin() const noexcept {
            return const_reverse_iterator(backPtr_ - 1);
        }

        /// Iterator to first element of reversed DVector
        /// If empty, the returned iterator will be equal to rend()
        J_NODISCARD const_reverse_iterator crbegin() const noexcept {
            return const_reverse_iterator(backPtr_ - 1);
        }

        /// Iterator to one past last the last element of reversed DVector
        J_NODISCARD reverse_iterator rend() noexcept {
            return reverse_iterator(frontPtr_ - 1);
        }

        /// Iterator to one past last the last element of reversed DVector
        J_NODISCARD const_reverse_iterator rend() const noexcept {
            return const_reverse_iterator(frontPtr_ - 1);
        }

        /// Iterator to one past last the last element of reversed DVector
        J_NODISCARD const_reverse_iterator crend() const noexcept {
            return const_reverse_iterator(frontPtr_ - 1);
        }

        // Capacity

        /// \return True if container is empty, false otherwise
        J_NODISCARD bool empty() const noexcept {
            return backPtr_ - frontPtr_ == 0;
        }

        /// \return Number of elements in the container, equal to std::distance(begin(), end())
        J_NODISCARD size_type size() const noexcept {
            return backPtr_ - frontPtr_;
        }

        /// \return Number of elements in the container with indices < 0
        J_NODISCARD size_type size_front() const noexcept {
            return midpoint_ - frontPtr_;
        }

        /// \return Number of elements in the container with indices >= 0
        J_NODISCARD size_type size_back() const noexcept {
            return backPtr_ - midpoint_;
        }

        /// NOTE: to achieve max size must append half to front, half to back.
        /// Otherwise is only capable of achieving half of max size
        /// \return Maximum number of elements
        J_NODISCARD size_type max_size() const noexcept {
            return std::numeric_limits<size_type>::max();
        }

        /// Increase the capacity to a value greater or equal to new_cap
        /// If new_cap less than current capacity, does nothing
        /// - Does not change the size of the vector
        /// - Invalidates all iterators if capacity change was performed
        void reserve(const size_type new_cap) {
            if (new_cap > capacity_)
                resize(new_cap);
        }

        /// \return Number of elements the container has currently allocated space for
        J_NODISCARD size_type capacity() const noexcept {
            return capacity_;
        }

        /// Removes unused capacity
        /// - Invalidates all iterators if capacity change was performed
        void shrink_to_fit() {
            resize(size());
        }

        // Modifiers

        /// Erases all elements from the container
        /// - After this call, size() returns zero
        /// - Invalidates all iterators
        void clear() noexcept {
            while (frontPtr_ != backPtr_) {
                std::allocator_traits<allocator_type>::destroy(allocator_, frontPtr_);
                ++frontPtr_;
            }

            frontPtr_ = midpoint_;
            backPtr_  = midpoint_;
        }

        // If needed one day emplace_move_front, emplace_move_back; insert, erase front and back

        /*
        // Current emplace is non ideal, cannot write to index -1

        /// Inserts a new element into the container directly before pos
        /// - If pos <  midpoint: Existing elements moved forwards
        /// - If pos >= midpoint: Existing elements moved back
        /// \return Iterator pointing to the emplaced element
        template <class... TArgs>
        iterator emplace(const const_iterator pos, TArgs&&... args) {
            auto new_element = T(std::forward<TArgs>(args)...); // Strong exception guarantee

            // Given: 0 1 2 3 4
            //              ^
            // Move to front: 0 1 2 _ 3 4
            //                        ^
            // Move to back: 0 1 2 _ 3 4
            //                     ^

            auto index = pos.GetPointer() - data_; // Iterators will be invalidated
            if (pos.GetPointer() < midpoint_) {
                --index; // Since moving to the front, do not modify element at pos
                CreateElementGapFront(index, 1);
            }
            else {
                CreateElementGapBack(index, 1);
            }

            auto* emplace_ptr = data_ + index;
            std::allocator_traits<allocator_type>::construct(allocator_, emplace_ptr, std::move(new_element));
            return iterator(emplace_ptr);
        }

        /// Removes the element at pos
        /// - Invalidates all iterators after pos, including end()
        /// \return Iterator following the removed element
        iterator erase(const const_iterator pos) {
            auto* start_ptr = const_cast<pointer>(pos.GetPointer());
            RemoveElements(start_ptr, 1);
            return iterator(start_ptr);
        }

        /// Removes the elements in the range [first, last)
        /// - Invalidates all iterators after pos, including end()
        /// \return Iterator following the last removed element
        iterator erase(const_iterator first, const_iterator last) {
            if (last - first < 0)
                return iterator(const_cast<pointer>(last.GetPointer()));

            auto* start_ptr = const_cast<pointer>(first.GetPointer());
            RemoveElements(start_ptr, last - first + 1);
            return iterator(start_ptr);
        }
        */

        /// Appends to front of container starting at index -1
        /// - Invalidates all iterators if capacity change was performed
        void push_front(const T& val) {
            emplace_front(val);
        }

        /// Appends to front of container starting at index -1
        /// Moved into new element
        /// - Invalidates all iterators if capacity change was performed
        void push_front(const T&& val) {
            emplace_front(val);
        }

        /// Appends to end of container starting at index 0
        /// - Invalidates all iterators if capacity change was performed
        void push_back(const T& val) {
            emplace_back(val);
        }

        /// Appends to end of container starting at index 0
        /// Moved into new element
        /// - Invalidates all iterators if capacity change was performed
        void push_back(const T&& val) {
            emplace_back(val);
        }

        /// Appends to front of container starting at index -1
        /// Forwards arguments to constructor of T
        /// - Invalidates all iterators if capacity change was performed
        template <typename... TArgs>
        reference emplace_front(TArgs&&... args) {
            if (frontPtr_ <= data_) {
                ResizeUp();
            }

            return UncheckedEmplaceFront(std::forward<TArgs>(args)...);
        }

        /// Appends to end of container starting at index 0
        /// Forwards arguments to constructor of T
        /// - Invalidates all iterators if capacity change was performed
        template <typename... TArgs>
        reference emplace_back(TArgs&&... args) {
            if (backPtr_ >= data_ + capacity_) {
                ResizeUp();
            }

            return UncheckedEmplaceBack(std::forward<TArgs>(args)...);
        }

        /// Removes the first element
        /// It is possible to pop_front past index -1 to indices > -1
        void pop_front() noexcept {
            assert(!empty());
            std::allocator_traits<allocator_type>::destroy(allocator_, frontPtr_);
            ++frontPtr_;
        }

        /// Removes the last element
        /// It is possible to pop_back past index 0 to indices < 0
        void pop_back() noexcept {
            assert(!empty());
            --backPtr_;
            std::allocator_traits<allocator_type>::destroy(allocator_, backPtr_);
        }

        /// Changes container to have capacity of at least new_capacity elements
        ///
        /// If new_capacity is even, 1 is added to make it odd
        /// If resizing down, values furthest from the midpoint (index 0) are discarded first
        ///
        /// Valid index range [(-capacity() / 2), (capacity() / 2)]
        void resize(size_type new_capacity) {
            // Capacities must be odd for single value midpoint to exist
            if (new_capacity % 2 == 0)
                new_capacity++;

            std::unique_ptr<value_type[]> new_data(allocator_.allocate(new_capacity));

            auto* new_midpoint = new_data.get() + new_capacity / 2;

            // Max elements that can be ahead or behind midpoint
            const auto max_lead_trail_elements = new_capacity / 2;

            // Elements ahead of midpoint
            const auto midpoint_lead = std::min(SafeCast<size_type>(midpoint_ - frontPtr_), max_lead_trail_elements);

            // Where to start copying
            const auto* old_start = midpoint_ - midpoint_lead;
            auto* new_start       = new_midpoint - midpoint_lead;

            // Avoids writing past allocated memory
            const auto max_iterate = SafeCast<size_type>(new_data.get() + new_capacity - new_start);
            const auto old_size    = size();

            for (size_type i = 0; i < std::min(old_size, max_iterate); ++i) {
                std::allocator_traits<allocator_type>::construct(allocator_, new_start, std::move(*old_start));
                std::allocator_traits<allocator_type>::destroy(allocator_, old_start);
                ++new_start;
                ++old_start;
            }

            frontPtr_ = new_midpoint - (midpoint_ - frontPtr_);
            backPtr_  = new_midpoint + (backPtr_ - midpoint_);
            midpoint_ = new_midpoint;

            allocator_.deallocate(data_, capacity_);
            data_ = new_data.release();

            capacity_ = new_capacity;
        }


        // This strays from std::vector behavior, since the capacity is shared between front and back
        // If resizing to 5, indices -2, -1, 0, 1, 2 are constructed with val, instead of std::vector 0, 1, 2, 3, 4
        /*
        /// Resizes container to have capacity new_capacity
        /// initializes new elements with val
        void resize(const size_type new_capacity, const value_type& val) {
            DoResize<true>(new_capacity, val);
        }
        */

        void swap(DVector& other) noexcept(std::allocator_traits<Allocator>::propagate_on_container_swap::value ||
                                           std::allocator_traits<Allocator>::is_always_equal::value) {
            using std::swap;
            swap(allocator_, other.allocator_);
            swap(capacity_, other.capacity_);
            swap(data_, other.data_);
            swap(frontPtr_, other.frontPtr_);
            swap(backPtr_, other.backPtr_);
            swap(midpoint_, other.midpoint_);
        }

        CEREAL_SAVE(archive) {
            archive(size_front(), size_back());

            for (const auto& element : *this) {
                archive(element);
            }
        }

        CEREAL_LOAD(archive) {
            size_type size_front, size_back;
            archive(size_front, size_back);

            reserve(std::max(size_front, size_back) * 2);

            frontPtr_ -= size_front;
            backPtr_ += size_back;

            auto* element_ptr = frontPtr_;
            while (element_ptr != backPtr_) {
                // Most serialization methods expect a default constructed object instead of uninitialized memory
                value_type val;
                archive(val);

                std::allocator_traits<allocator_type>::construct(allocator_, element_ptr, std::move(val));
                ++element_ptr;
            }

            assert(this->size_front() == size_front);
            assert(this->size_back() == size_back);
            assert(this->size() == size_front + size_back);
        }

    private:
        /// Appends count copies of T of constructor argument args
        /// - Does not call destructor of overwritten elements
        template <typename... TArgs>
        void NoDestructAssign(const size_type count, TArgs&&... args) {
            resize(count * 2); // Only occupy indices > 0

            for (size_type i = 0; i < count; ++i) {
                UncheckedEmplaceBack(std::forward<TArgs>(args)...);
            }
        }

        /// Replaces the contents with copies of those in the range [first, last)
        /// - Does not call destructor of overwritten elements
        template <class InputIt>
        void NoDestructAssignItRange(const InputIt first, const InputIt last) {
            resize(std::distance(first, last) * 2); // Only occupy indices > 0

            for (auto it = first; it != last; ++it) {
                UncheckedEmplaceBack(*it);
            }
        }

        /// Replaces the contents with the elements from the initializer list
        /// - Does not call destructor of overwritten elements
        void NoDestructAssignInitList(std::initializer_list<T> init_list) {
            NoDestructAssignItRange(init_list.begin(), init_list.end());
        }

        /// Appends to front without checking for sufficient capacity
        template <typename... TArgs>
        reference UncheckedEmplaceFront(TArgs&&... args) {
            assert(frontPtr_ > data_); // Failed to allocate sufficient capacity
            --frontPtr_;
            std::allocator_traits<allocator_type>::construct(allocator_, frontPtr_, std::forward<TArgs>(args)...);

            return *frontPtr_;
        }

        /// Appends to end without checking for sufficient capacity
        template <typename... TArgs>
        reference UncheckedEmplaceBack(TArgs&&... args) {
            assert(backPtr_ < data_ + capacity_); // Failed to allocate sufficient capacity
            std::allocator_traits<allocator_type>::construct(allocator_, backPtr_, std::forward<TArgs>(args)...);
            ++backPtr_;

            return *backPtr_;
        }


        /*
        /// Creates a gap of count elements starting with element at element_offset from data_
        /// by moving elements to the front of the container
        /// \param element_offset Elements from data_
        void CreateElementGapFront(const size_type element_offset, const size_type count) {
            assert(count != 0);

            assert(false); // This reads into uninitialized memory because of element_offset after reserve
            reserve((std::max(size_front(), size_back()) + count) * 2);

            auto current_element = frontPtr_;
            const auto* end      = data_ + element_offset;

            while (current_element <= end) {
                std::allocator_traits<allocator_type>::construct(
                    allocator_, current_element - count, std::move(*current_element));
                std::allocator_traits<allocator_type>::destroy(allocator_, current_element);
                ++current_element;
            }
            frontPtr_ -= count;
        }

        /// Creates a gap of element size count starting with element at element_offset from data_
        /// by moving elements to the back of the container
        /// \param element_offset Elements from data_
        void CreateElementGapBack(const size_type element_offset, const size_type count) {
            assert(count != 0);

            assert(false); // This reads into uninitialized memory because of element_offset after reserve
            reserve((std::max(size_front(), size_back()) + count) * 2);

            auto current_element = backPtr_ - 1;
            const auto* start    = data_ + element_offset;

            while (current_element >= start) {
                std::allocator_traits<allocator_type>::construct(
                    allocator_, current_element + count, std::move(*current_element));
                std::allocator_traits<allocator_type>::destroy(allocator_, current_element);
                --current_element;
            }
            backPtr_ += count;
        }

        /// Remove count elements starting at start (start removed if count >= 1)
        void RemoveElements(const pointer start, size_type count) {
            assert(count != 0);
            auto* current_element   = start;
            const auto* end_pointer = backPtr_ - count;

            while (current_element != end_pointer) {
                std::allocator_traits<allocator_type>::destroy(allocator_, current_element);
                std::allocator_traits<allocator_type>::construct(
                    allocator_, current_element, std::move(*(current_element + count)));
                ++current_element;
            }
            while (current_element != backPtr_) {
                std::allocator_traits<allocator_type>::destroy(allocator_, current_element);
                ++current_element;
            }
            backPtr_ -= count;
        }
        */

        /// To be called from constructor for initial resize
        /// Copies data from other DVector
        /// - Does not check if desired capacity is smaller
        /// - Does not delete data_ (assumes it is nullptr)
        /// - Assumes resizing up and new_capacity is odd
        void CopyFromOther(const DVector& old) {
            const auto new_capacity = old.capacity();
            std::unique_ptr<value_type[]> new_data(allocator_.allocate(new_capacity));

            auto* new_midpoint = new_data.get() + new_capacity / 2;

            // Elements ahead of midpoint
            const auto midpoint_lead = old.midpoint_ - old.frontPtr_;

            // Where to start copying
            const auto* old_start = old.midpoint_ - midpoint_lead;
            auto* new_start       = new_midpoint - midpoint_lead;

            const auto amount_to_copy = old.size();
            for (size_type i = 0; i < amount_to_copy; ++i) {
                std::allocator_traits<allocator_type>::construct(allocator_, new_start, *old_start);
                ++new_start;
                ++old_start;
            }

            frontPtr_ = new_midpoint - (old.midpoint_ - old.frontPtr_);
            backPtr_  = new_midpoint + (old.backPtr_ - old.midpoint_);
            midpoint_ = new_midpoint;

            data_ = new_data.release();

            capacity_ = new_capacity;
        }

        void ResizeUp() {
            // Add 2 to guarantee capacity for front and back
            resize(LossyCast<std::size_t>(SafeCast<float>(capacity_) * kScalingFactor) + 2);
        }

        /// \return true if pointer points to a valid element
        J_NODISCARD bool ElementPointerValid(const_pointer pointer) const noexcept {
            return pointer >= frontPtr_ && pointer < backPtr_;
        }

        // !! Constructors, swap must be updated if members are changed

        allocator_type allocator_;

        size_type capacity_ = 0;

        value_type* data_ = nullptr;

        /// Points to 1 after the slot for front insertion
        value_type* frontPtr_ = nullptr;
        /// Points to the next slot for back insertion
        value_type* backPtr_ = nullptr;

        /// Pointer to where element 0 resides
        value_type* midpoint_ = nullptr;
    };

    // Deduction guides for DVector
    template <class InputIt, class Alloc = std::allocator<typename std::iterator_traits<InputIt>::value_type>>
    DVector(InputIt, InputIt, Alloc = Alloc()) -> DVector<typename std::iterator_traits<InputIt>::value_type, Alloc>;
} // namespace jactorio

#endif // JACTORIO_INCLUDE_CORE_DVECTOR_H
