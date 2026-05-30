#include <stdexcept>
#include <utility>
#include <type_traits>

// Исключение этого типа должно генерироваться при обращении к пустому Optional
class BadOptionalAccess : public std::exception {
public:
    const char* what() const noexcept override {
        return "Bad optional access";
    }
};

template <typename T>
class Optional {
public:
    Optional() = default;

    Optional(const T& value) {
        Construct(value);
    }

    Optional(T&& value) {
        Construct(std::move(value));
    }

    Optional(const Optional& other) {
        if (other.is_initialized_) {
            Construct(*other);
        }
    }

    Optional(Optional&& other) noexcept {
        if (other.is_initialized_) {
            Construct(std::move(*other));
        }
    }

    Optional& operator=(const T& value) {
        if (is_initialized_) {
            Get() = value;
        }
        else {
            Construct(value);
        }
        return *this;
    }

    Optional& operator=(T&& rhs) noexcept {
        if (is_initialized_) {
            Get() = std::move(rhs);
        }
        else {
            Construct(std::move(rhs));
        }
        return *this;
    }

    Optional& operator=(const Optional& rhs) {
        if (this != &rhs) {
            if (rhs.is_initialized_) {
                *this = *rhs;
            }
            else {
                Reset();
            }
        }
        return *this;
    }

    Optional& operator=(Optional&& rhs) noexcept {
        if (this != &rhs) {
            if (rhs.is_initialized_) {
                *this = std::move(*rhs);
            }
            else {
                Reset();
            }
        }
        return *this;
    }

    ~Optional() { Reset(); }

    bool HasValue() const { return is_initialized_; }

    T& operator*()& { return Get(); }
    const T& operator*() const& { return Get(); }
    T&& operator*()&& { return std::move(Get()); }

    T* operator->() { return &Get(); }
    const T* operator->() const { return &Get(); }

    T& Value()& {
        if (!is_initialized_) {
            throw BadOptionalAccess();
        }
        return Get();
    }

    const T& Value() const& {
        if (!is_initialized_) {
            throw BadOptionalAccess();
        }
        return Get();
    }

    T&& Value()&& {
        if (!is_initialized_) {
            throw BadOptionalAccess();
        }
        return std::move(Get());
    }

    void Reset() {
        if (is_initialized_) {
            Get().~T();
            is_initialized_ = false;
        }
    }

    template <typename... Args>
    void Emplace(Args&&... args) {
        Reset(); // Удаляем предыдущее значение, если оно было
        Construct(std::forward<Args>(args)...); // Создаём новое значение
    }

private:
    alignas(T) char data_[sizeof(T)];
    bool is_initialized_ = false;

    T& Get() { return *std::launder(reinterpret_cast<T*>(&data_)); }
    const T& Get() const { return *std::launder(reinterpret_cast<const T*>(&data_)); }

    template <typename... Args>
    void Construct(Args&&... args) {
        new (data_) T(std::forward<Args>(args)...);
        is_initialized_ = true;
    }
};
