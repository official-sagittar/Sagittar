#pragma once

#include "commons/pch.h"

namespace sagittar::containers {

    template<typename T, std::size_t Capacity = 256>
    class ArrayList: public std::array<T, Capacity> {
       private:
        std::size_t m_current_size = 0;

       public:
        template<typename... Args>
        void emplace(size_t position, Args&&... args) {
            if (position > m_current_size || m_current_size >= Capacity)
            {
                throw std::out_of_range("Invalid position or ArrayList is full.");
            }

            // Shift elements to the right to make space for the new element
            for (size_t i = m_current_size; i > position; --i)
            {
                (*this)[i] = std::move((*this)[i - 1]);
            }

            // Construct the new element in place
            (*this)[position] = T(std::forward<Args>(args)...);
            ++m_current_size;
        }

        template<typename... Args>
        void emplaceBack(Args&&... args) {
            if (m_current_size >= Capacity)
            {
                throw std::out_of_range("ArrayList is full: Cannot emplace_back.");
            }

            // Construct the new element at the back
            (*this)[m_current_size++] = T(std::forward<Args>(args)...);
        }

        void push(T t) {
            if (m_current_size >= Capacity)
            {
                throw std::out_of_range("ArrayList is full: Cannot push.");
            }
            (*this)[m_current_size++] = t;
        }

        size_t size() const { return m_current_size; }

        typedef typename std::array<T, Capacity>::iterator       Iterator_t;
        typedef typename std::array<T, Capacity>::const_iterator ConstIterator_t;

        Iterator_t begin() { return std::array<T, Capacity>::begin(); }

        Iterator_t end() { return std::array<T, Capacity>::begin() + m_current_size; }

        ConstIterator_t begin() const { return std::array<T, Capacity>::begin(); }

        ConstIterator_t end() const { return std::array<T, Capacity>::begin() + m_current_size; }
    };

}
