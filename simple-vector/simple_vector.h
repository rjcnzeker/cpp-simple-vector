#pragma once

#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <algorithm>
#include <iostream>

#include "array_ptr.h"

struct ReserveProxyObj {
    explicit ReserveProxyObj(size_t cap) : capacity_(cap) {}

    size_t capacity_;
};

ReserveProxyObj Reserve(int capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template<typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) : items_(size) {
        size_ = size;
        capacity_ = size;
        for (size_t i = 0; i < size; ++i) {
            items_[i] = Type();
        }
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) : items_(size) {
        size_ = size;
        capacity_ = size;
        std::fill(this->begin(), this->end(), value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) : items_(init.size()) {
        size_ = init.size();
        capacity_ = size_;
        size_t i = 0;
        for (const Type& item : init) {
            items_[i] = item;
            ++i;
        }
    }

    SimpleVector(const SimpleVector& other) {
        SimpleVector<Type> other_copy(other.size_);
        for (size_t i = 0; i < other.size_; ++i) {
            other_copy[i] = other[i];
        }
        this->swap(other_copy);
    }

    SimpleVector(ReserveProxyObj proxyObj) {
        Reserve(proxyObj.capacity_);
    }

    SimpleVector(SimpleVector<Type>&& other) :
            items_(std::move(other.items_)),
            size_(std::exchange(other.size_, 0)),
            capacity_(std::exchange(other.capacity_, 0)) {
    };

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("index => size");
        }
        return items_[index];
    }

    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("");
        }
        const Type& ref = items_[index];
        return ref;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        const Type& const_ref = items_[index];
        return const_ref;
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this == &rhs) {
            return *this;
        }
        SimpleVector<Type> other_copy(rhs.size_);
        for (size_t i = 0; i < rhs.size_; ++i) {
            other_copy[i] = rhs[i];
        }
        this->swap(other_copy);
        return *this;
    }

    SimpleVector& operator=(SimpleVector<Type>&& other) {
        if (this == &other) {
            return *this;
        }
        items_ = std::move(other.items_);
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
        return *this;
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
            return;
        }
        if (new_size < capacity_) {
            for (size_t i = size_; i < new_size; ++i) {
                this->PushBack(Type());
            }

        } else {
            SimpleVector<Type> new_array(size_);
            std::move(begin(), end(), new_array.begin());
            swap(new_array);
            for (size_t i = size_; i < new_size; ++i) {
                this->PushBack(Type());
            }
            return;
        }
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity <= capacity_) {
            return;
        }
        SimpleVector<Type> new_vector(new_capacity);
        new_vector.Resize(size_);
        std::copy(begin(), end(), new_vector.begin());
        swap(new_vector);

    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        if (capacity_ == size_) {
            ArrayPtr<Type> new_array(capacity_ == 0 ? 2 : capacity_ * 2);
            std::move(begin(), end(), new_array);
            items_.swap(new_array);
            capacity_ = (capacity_ == 0 ? 2 : capacity_ * 2);
        }
        items_[size_] = item;
        ++size_;
    }

    void PushBack(Type&& item) {
        if (capacity_ == size_) {
            ArrayPtr<Type> new_array(capacity_ == 0 ? 2 : capacity_ * 2);
            std::move(std::make_move_iterator(begin()), std::make_move_iterator(end()), new_array.Get());
            items_.swap(new_array);
            capacity_ = (capacity_ == 0 ? 2 : capacity_ * 2);
        }
        items_[size_] = std::exchange(item, 0);
        ++size_;
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        assert(size_ > 0);
        --size_;
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    Iterator Insert(ConstIterator pos, const Type& value) {
        if (capacity_ >= size_ + 1) {
            Iterator new_pos = begin() + (pos - cbegin());
            std::copy_backward(new_pos, end(), end() + 1);
            ++size_;
            *new_pos = value;
            return new_pos;
        } else {
            SimpleVector<Type> new_vector(capacity_*
            2);
            new_vector.Resize(size_ + 1);

            //Вставка первой половины старого вектора в новый
            std::move(cbegin(), pos, new_vector.begin());

            //Находим позицию для вставки в старом векторе
            Iterator old_pos = begin() + (pos - cbegin());
            //Находим позицию для вставки в новом векторе
            Iterator new_pos = new_vector.begin() + (old_pos - begin());
            *new_pos = value;

            //Копируем оставшуюся часть старого вектора в новый
            std::move_backward(pos, cend(), new_vector.end());
            swap(new_vector);

            return new_pos;
        }

    }

    Iterator Insert(Iterator pos, Type&& value) {
        if (capacity_ >= size_ + 1) {
            Iterator new_pos = begin() + (pos - cbegin());

            std::move_backward(new_pos, end(), end() + 1);
            ++size_;

            items_[pos - begin()] = std::exchange(value, 0);

            return std::move(new_pos);
        } else {
            SimpleVector<Type> new_vector(capacity_*
            2);
            new_vector.Resize(size_ + 1);

            std::move(begin(), pos, new_vector.begin());

            Iterator old_pos = begin() + (pos - begin());
            Iterator new_pos = new_vector.begin() + (old_pos - begin());

            items_[new_pos - begin()] = std::exchange(value, 0);

            std::move_backward(pos, end(), new_vector.end());
            swap(new_vector);

            return new_pos;
        }

    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        Iterator new_pos = begin() + (pos - cbegin());
        std::copy(new_pos + 1, end(), new_pos);
        --size_;
        return new_pos;
    }

    Iterator Erase(Iterator pos) {
        Iterator new_pos = begin() + (pos - cbegin());
        std::move(new_pos + 1, end(), new_pos);
        --size_;
        return new_pos;
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return items_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return items_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        const Type* begin = items_.Get();
        return begin;
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        const Type* end = items_.Get() + size_;
        return &end;
    }

private:

    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template<typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template<typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template<typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template<typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !std::lexicographical_compare(rhs.begin(), rhs.end(), lhs.begin(), lhs.end());
}

template<typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(rhs.begin(), rhs.end(), lhs.begin(), lhs.end());
}

template<typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}