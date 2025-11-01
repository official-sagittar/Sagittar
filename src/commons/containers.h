#pragma once

#include "commons/pch.h"

namespace sagittar::containers {

    template<typename T, std::size_t Capacity = 256>
    class ArrayList: public std::array<T, Capacity> {
       private:
        std::size_t current_size = 0;

       public:
        template<typename... Args>
        void emplace(size_t position, Args&&... args) {
            if (position > current_size || current_size >= Capacity)
            {
                throw std::out_of_range("Invalid position or ArrayList is full.");
            }

            // Shift elements to the right to make space for the new element
            for (size_t i = current_size; i > position; --i)
            {
                (*this)[i] = std::move((*this)[i - 1]);
            }

            // Construct the new element in place
            (*this)[position] = T(std::forward<Args>(args)...);
            ++current_size;
        }

        template<typename... Args>
        void emplace_back(Args&&... args) {
            if (current_size >= Capacity)
            {
                throw std::out_of_range("ArrayList is full: Cannot emplace_back.");
            }

            // Construct the new element at the back
            (*this)[current_size++] = T(std::forward<Args>(args)...);
        }

        void push(T t) {
            if (current_size >= Capacity)
            {
                throw std::out_of_range("ArrayList is full: Cannot push.");
            }
            (*this)[current_size++] = t;
        }

        size_t size() const { return current_size; }

        typedef typename std::array<T, Capacity>::iterator       iterator;
        typedef typename std::array<T, Capacity>::const_iterator const_iterator;

        iterator begin() { return std::array<T, Capacity>::begin(); }

        iterator end() { return std::array<T, Capacity>::begin() + current_size; }

        const_iterator begin() const { return std::array<T, Capacity>::begin(); }

        const_iterator end() const { return std::array<T, Capacity>::begin() + current_size; }
    };

}
