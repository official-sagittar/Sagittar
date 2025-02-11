#pragma once

#include "pch.h"

namespace sagittar {

    namespace containers {

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

        template<typename T, std::size_t Capacity = 256>
        class ArrayStack {
           private:
            std::array<T, Capacity> elements;
            std::size_t             current_size = 0;

           public:
            // Push an element onto the stack
            void push(const T& item) {
                if (current_size >= Capacity)
                {
                    throw std::overflow_error("Stack overflow: No more elements can be pushed.");
                }
                elements[current_size++] = item;
            }

            template<typename... Args>
            void emplace_back(Args&&... args) {
                if (current_size >= Capacity)
                {
                    throw std::out_of_range("Stack overflow: No more elements can be pushed.");
                }

                // Construct the new element at the back
                elements[current_size++] = T(std::forward<Args>(args)...);
            }

            // Pop the top element off the stack
            void pop() {
                if (current_size == 0)
                {
                    throw std::underflow_error("Stack underflow: No elements to pop.");
                }
                --current_size;
            }

            // Peek at the top element of the stack
            T top() const {
                if (current_size == 0)
                {
                    throw std::out_of_range("Stack is empty: No top element.");
                }
                return elements[current_size - 1];
            }

            // Peek at an element at a specific position (0-based index)
            T peek(size_t position) const {
                if (position >= current_size)
                {
                    throw std::out_of_range("Invalid position: Out of stack bounds.");
                }
                return elements[position];
            }

            // Check if the stack is empty
            bool is_empty() const { return current_size == 0; }

            // Get the current size of the stack
            size_t size() const { return current_size; }

            // Define iterator types using the underlying array iterators
            typedef typename std::array<T, Capacity>::iterator       iterator;
            typedef typename std::array<T, Capacity>::const_iterator const_iterator;

            // Begin iterator
            iterator begin() { return elements.begin(); }

            // End iterator (only up to the current size)
            iterator end() { return elements.begin() + current_size; }

            // Const begin iterator
            const_iterator begin() const { return elements.begin(); }

            // Const end iterator (only up to the current size)
            const_iterator end() const { return elements.begin() + current_size; }
        };

        template<typename... Types>
        class VariantMap {
           public:
            using ValueType = std::variant<Types...>;

            // Insert a key-value pair
            template<typename T>
            void insert(const std::string& key, T value) {
                static_assert((std::is_same_v<T, Types> || ...), "Type not supported");
                data_[key] = value;
            }

            // Get value as optional
            template<typename T>
            T get(const std::string& key, const T defaultValue) const {
                auto it = data_.find(key);
                if (it != data_.end())
                {
                    return std::get<T>(it->second);
                }
                return defaultValue;
            }

            // Check if key exists
            bool contains(const std::string& key) const { return data_.find(key) != data_.end(); }

            // Remove a key
            void erase(const std::string& key) { data_.erase(key); }

            // Print all key-value pairs
            void display() const {
                for (const auto& [key, value] : data_)
                {
                    std::visit([&](const auto& v) { std::cout << key << ": " << v << '\n'; },
                               value);
                }
            }

           private:
            std::map<std::string, ValueType> data_;
        };

    }

}
