#pragma once

#include <cstdint>
#include <exception>
#include <type_traits>
#include <iostream>

namespace bmstu {
class bad_optional_access : public std::exception {
 public:
  using exception::exception;
  const char *what() const noexcept override {
    return "Bad optional access";
  }
};

template<typename T>
class optional {
 public:
  using value_type = T;
  using pointer = T *;
  using const_pointer = const T *;
  using reference = T &;
  using const_reference = const T &;
  using rvalue_reference = T &&;

  optional() = default;

  optional(const optional &other) {
    if (is_initialized_ && other.is_initialized_) {
      value() = other.value();
    } else if (!is_initialized_ && other.is_initialized_) {
      pointer val = new(data_) value_type(other.value());
      is_initialized_ = true;
      (void *) val;
    } else if (is_initialized_ && !other.is_initialized_) {
      reset();
    }
  }

  optional(optional &&other)  noexcept {
    if (is_initialized_ && other.is_initialized_) {
      value() = std::move(other.value());
    } else if (!is_initialized_ && other.is_initialized_) {
      pointer val = new(data_) value_type(std::move(other.value()));
      is_initialized_ = true;
      (void *) val;
    } else if (is_initialized_ && !other.is_initialized_) {
      reset();
    }
  }

  explicit optional(const value_type &value) { // конструктор от значения
    is_initialized_ = true;
    pointer val = new(data_) T{value};
    (void *) val;
  }

  explicit optional(value_type &&value) {
    is_initialized_ = true;
    pointer val = new(data_) T(std::move(value));
    (void) val;
  }

  optional &operator=(const value_type &val) { // a = std::vector {1,2,3,45}; optional = a;
    if (is_initialized_) {
      value() = val;
    } else {
      pointer ptr = new(data_) value_type(val);
      is_initialized_ = true;
    }
    return *this;
  }

  optional &operator=(value_type &&val) { // optional = std::vector {1,2,3,45}
    if (is_initialized_) {
      value() = std::move(val);
    } else {
      pointer ptr = new(data_) value_type(std::move(val));
      is_initialized_ = true;
    }
    return *this;
  }

  optional &operator=(const optional &other) {
    if (this == &other) {
      return *this;
    }

    if (is_initialized_ && other.is_initialized_) {
      value() = other.value();
    } else if (!is_initialized_ && other.is_initialized_) {
      pointer val = new(data_) value_type(other.value());
      is_initialized_ = true;
      (void) val;
    } else if (is_initialized_ && !other.is_initialized_) {
      reset();
    }
    return *this;
  }

  optional &operator=(optional &&other) { // optional = std::vector {1,2,3,4};
    if (is_initialized_ && other.is_initialized_) {
      value() = std::move(other.value());
    } else if (!is_initialized_ && other.is_initialized_) {
      pointer val = new(data_) value_type(std::move(other.value()));
      is_initialized_ = true;
      (void *) val;
    } else if (is_initialized_ && !other.is_initialized_) {
      reset();
    }
    return *this;
  }

  /*
   * Оператор `&` после списка параметров функции в C++ указывает на тип ссылки, который может быть использован для вызова этой функции. В данном случае, `&` означает, что оператор `*` может быть вызван только для lvalue.
   * В контексте класса `optional`, это означает, что вы не можете получить значение из временного объекта `optional`, который не существует после выражения, в котором он был создан. Это может быть полезно для предотвращения ошибок, когда значение извлекается из `optional`, который уже был уничтожен.
   *
   * Вот пример:
   *
   * T value = *optional<T>(); // Ошибка: нельзя вызвать operator* для rvalue
   *
   * В этом примере `optional<T>()` является rvalue, и поэтому оператор `*` не может быть вызван. Если бы `&` не было в определении оператора, этот код был бы допустимым, но мог бы привести к неопределенному поведению, так как значение извлекается из временного объекта `optional`, который уничтожается сразу после этого выражения.
   */

  reference operator*() &{
    return *(reinterpret_cast<pointer>(data_));
  }

  const_reference operator*() const &{
    return *(reinterpret_cast<const_pointer>(data_));
  }

  pointer operator->() {
    return reinterpret_cast<pointer>(data_);
  }

  const_pointer operator->() const {
    return reinterpret_cast<const_pointer>(data_);
  }

  /*
   * Оператор rvalue_reference operator*() && предназначен для извлечения значения
   * из объекта optional, который является rvalue. Это может быть полезно, когда вы
   * хотите извлечь значение из временного объекта optional без копирования.
   */
  rvalue_reference operator*() &&{
    if (!is_initialized_) {
      throw bad_optional_access();
    }
    return std::move(*(reinterpret_cast<pointer>(data_)));
  }

  reference value() &{
    if (!is_initialized_) {
      throw bad_optional_access();
    }
    return *reinterpret_cast<pointer>(data_);
  }

  const_reference value() const &{
    if (!is_initialized_) {
      throw bad_optional_access();
    }
    return *reinterpret_cast<const_pointer>(data_);
  }

  template<typename ...Args>
  void emplace(Args &&...args) {
    if (is_initialized_) { reset(); }

    is_initialized_ = true;
    pointer val = new(data_)T(std::forward<Args>(args)...);
    (void) (&val);
  }

  void reset() {
    if (is_initialized_) {
      reinterpret_cast<pointer>(data_)->~T();
      is_initialized_ = false;
    }
  }

  ~optional() {
    if (is_initialized_) {
      pointer ptr = reinterpret_cast<pointer>(data_);
      ptr->~T();
    }
  }

  bool has_value() {
    return is_initialized_;
  }
 private:
  alignas(T) uint8_t data_[sizeof(T)];
  bool is_initialized_ = false;
};
}
