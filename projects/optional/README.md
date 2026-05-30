# 📦 sandbox-optional

Учебная реализация контейнера `Optional<T>` на стандарте C++20. Проект демонстрирует работу с сырой памятью, управление жизненным циклом объектов через explicit destructor/placement new, реализацию семантики перемещения (Move Semantics) и перегрузку методов по ref-квалификаторам (`&`, `const &`, `&&`).

## 🛠 Технологический стек & Специфика

* **Стандарт:** C++20 (MSVC / GCC / Clang)
* **Система сборки:** CMake (с автоматическим подтягиванием зависимостей через `FetchContent`)
* **Тестирование:** Google Test (GTest) + автоматическое обнаружение через `gtest_discover_tests`
* **Архитектурные особенности:**
  * **Zero-allocation:** Хранение объекта внутри класса без выделения памяти в куче (`alignas(T) char data_`).
  * **Strict Aliasing & Laundering:** Безопасный доступ к переиспользованной памяти через `std::launder`.
  * **Perfect Forwarding:** Реализация метода `Emplace` с использованием Variadic Templates и `std::forward`.
  * **Ref-qualified Overloading:** Полная поддержка rvalue-оптимизаций при доступе к значению через `operator*()` и `.Value()`.

## 🚀 Сборка и запуск

Сборка проекта выполняется стандартными командами:

```bash
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```
