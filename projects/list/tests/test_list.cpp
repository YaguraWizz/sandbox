#include <gtest/gtest.h>
#include <string>
#include <utility>
#include <algorithm>
#include <initializer_list>

#include <list/list.h>

using namespace std::string_literals;

// ==========================================
// Тест-кейс 0: Инициализация и размеры по умолчанию
// ==========================================
TEST(SingleLinkedListTest, DefaultConstructorAndEmptyState) {
    const SingleLinkedList<int> empty_int_list;
    EXPECT_EQ(empty_int_list.GetSize(), 0u);
    EXPECT_TRUE(empty_int_list.IsEmpty());

    const SingleLinkedList<std::string> empty_string_list;
    EXPECT_EQ(empty_string_list.GetSize(), 0u);
    EXPECT_TRUE(empty_string_list.IsEmpty());
}

// ==========================================
// Тест-кейс 1: Базовая работа с итераторами и PushFront
// ==========================================
TEST(SingleLinkedListTest, IteratorsAndPushFront) {
    // Проверка пустой коллекции
    {
        SingleLinkedList<int> list;
        const auto& const_list = list;

        EXPECT_EQ(list.begin(), list.end());
        EXPECT_EQ(const_list.begin(), const_list.end());
        EXPECT_EQ(list.cbegin(), list.cend());
        EXPECT_EQ(list.cbegin(), const_list.begin());
        EXPECT_EQ(list.cend(), const_list.end());
    }

    // Добавление элементов и модификация
    {
        SingleLinkedList<int> list;
        const auto& const_list = list;

        list.PushFront(1);
        EXPECT_EQ(list.GetSize(), 1u);
        EXPECT_FALSE(list.IsEmpty());

        EXPECT_NE(const_list.begin(), const_list.end());
        EXPECT_NE(const_list.cbegin(), const_list.cend());
        EXPECT_NE(list.begin(), list.end());
        EXPECT_EQ(const_list.begin(), const_list.cbegin());

        EXPECT_EQ(*list.cbegin(), 1);
        *list.begin() = -1;
        EXPECT_EQ(*list.cbegin(), -1);

        const auto old_begin = list.cbegin();
        list.PushFront(2);
        EXPECT_EQ(list.GetSize(), 2u);

        const auto new_begin = list.cbegin();
        EXPECT_NE(new_begin, old_begin);

        // Проверка префиксного инкремента
        {
            auto new_begin_copy(new_begin);
            EXPECT_EQ(++new_begin_copy, old_begin);
        }
        // Проверка постфиксного инкремента
        {
            auto new_begin_copy(new_begin);
            EXPECT_EQ(new_begin_copy++, new_begin);
            EXPECT_EQ(new_begin_copy, old_begin);
        }
        // Достижение конца списка
        {
            auto old_begin_copy(old_begin);
            EXPECT_EQ(++old_begin_copy, list.end());
        }
    }

    // Конвертация Итераторов
    {
        SingleLinkedList<int> list;
        list.PushFront(1);
        
        SingleLinkedList<int>::ConstIterator const_it(list.begin());
        EXPECT_EQ(const_it, list.cbegin());
        EXPECT_EQ(*const_it, *list.cbegin());

        SingleLinkedList<int>::ConstIterator const_it1;
        const_it1 = list.begin();
        EXPECT_EQ(const_it1, const_it);
    }

    // Оператор ->
    {
        SingleLinkedList<std::string> string_list;
        string_list.PushFront("one"s);
        EXPECT_EQ(string_list.cbegin()->length(), 3u);
        
        string_list.begin()->push_back('!');
        EXPECT_EQ(*string_list.begin(), "one!"s);
    }
}

// ==========================================
// Тест-кейс 2: Сравнения, Swap и Строгая безопасность исключений
// ==========================================
TEST(SingleLinkedListTest, EqualityComparisonAndSwap) {
    SingleLinkedList<int> list_1{2, 1};
    SingleLinkedList<int> list_2{3, 2, 1};
    SingleLinkedList<int> list_1_copy{2, 1};
    SingleLinkedList<int> empty_list;
    SingleLinkedList<int> another_empty_list;

    EXPECT_EQ(list_1, list_1);
    EXPECT_EQ(empty_list, empty_list);
    EXPECT_EQ(list_1, list_1_copy);
    EXPECT_NE(list_1, list_2);
    EXPECT_NE(list_2, list_1);
    EXPECT_EQ(empty_list, another_empty_list);
}

TEST(SingleLinkedListTest, SwapMethods) {
    SingleLinkedList<int> first{2, 1};
    SingleLinkedList<int> second{15, 11, 10};

    const auto old_first_begin = first.begin();
    const auto old_second_begin = second.begin();
    const auto old_first_size = first.GetSize();
    const auto old_second_size = second.GetSize();

    first.swap(second);

    EXPECT_EQ(second.begin(), old_first_begin);
    EXPECT_EQ(first.begin(), old_second_begin);
    EXPECT_EQ(second.GetSize(), old_first_size);
    EXPECT_EQ(first.GetSize(), old_second_size);

    using std::swap;
    swap(first, second);

    EXPECT_EQ(first.begin(), old_first_begin);
    EXPECT_EQ(second.begin(), old_second_begin);
    EXPECT_EQ(first.GetSize(), old_first_size);
    EXPECT_EQ(second.GetSize(), old_second_size);
}

TEST(SingleLinkedListTest, InitializerListAndLexicographicalOperators) {
    SingleLinkedList<int> list{ 1, 2, 3, 4, 5 };
    EXPECT_EQ(list.GetSize(), 5u);
    EXPECT_FALSE(list.IsEmpty());
    
    int expected[] = { 1, 2, 3, 4, 5 };
    EXPECT_TRUE(std::equal(list.begin(), list.end(), std::begin(expected)));

    using IntList = SingleLinkedList<int>;
    EXPECT_LT(IntList({ 1, 2, 3 }), IntList({ 1, 2, 3, 1 }));
    EXPECT_LE(IntList({ 1, 2, 3 }), IntList({ 1, 2, 3 }));
    EXPECT_GT(IntList({ 1, 2, 4 }), IntList({ 1, 2, 3 }));
    EXPECT_GE(IntList({ 1, 2, 3 }), IntList({ 1, 2, 3 }));
}

TEST(SingleLinkedListTest, CopySemantics) {
    const SingleLinkedList<int> empty_list{};
    {
        auto list_copy(empty_list);
        EXPECT_TRUE(list_copy.IsEmpty());
    }

    SingleLinkedList<int> non_empty_list{ 1, 2, 3, 4 };
    {
        auto list_copy(non_empty_list);
        EXPECT_NE(non_empty_list.begin(), list_copy.begin());
        EXPECT_EQ(list_copy, non_empty_list);
    }

    {
        const SingleLinkedList<int> source_list{ 1, 2, 3, 4 };
        SingleLinkedList<int> receiver{ 5, 4, 3, 2, 1 };
        receiver = source_list;
        EXPECT_NE(receiver.begin(), source_list.begin());
        EXPECT_EQ(receiver, source_list);
    }
}

// Вспомогательная структура для контроля копирования
struct ThrowOnCopy {
    ThrowOnCopy() = default;
    explicit ThrowOnCopy(int& copy_counter) noexcept : countdown_ptr(&copy_counter) {}
    
    ThrowOnCopy(const ThrowOnCopy& other) : countdown_ptr(other.countdown_ptr) {
        if (countdown_ptr) {
            if (*countdown_ptr == 0) {
                throw std::bad_alloc();
            }
            else {
                --(*countdown_ptr);
            }
        }
    }
    ThrowOnCopy& operator=(const ThrowOnCopy& rhs) = delete;
    int* countdown_ptr = nullptr;
};

TEST(SingleLinkedListTest, CopyAssignmentExceptionSafety) {
    SingleLinkedList<ThrowOnCopy> src_list;
    src_list.PushFront(ThrowOnCopy{});
    src_list.PushFront(ThrowOnCopy{});
    auto thrower = src_list.begin();
    src_list.PushFront(ThrowOnCopy{});

    int copy_counter = 0;
    thrower->countdown_ptr = &copy_counter;

    SingleLinkedList<ThrowOnCopy> dst_list;
    dst_list.PushFront(ThrowOnCopy{});
    int dst_counter = 10;
    dst_list.begin()->countdown_ptr = &dst_counter;
    dst_list.PushFront(ThrowOnCopy{});

    // Проверяем транзакционность (strong exception safety): при исключении состояние dst_list не меняется
    EXPECT_THROW({
        dst_list = src_list;
    }, std::bad_alloc);

    EXPECT_EQ(dst_list.GetSize(), 2u);
    auto it = dst_list.begin();
    EXPECT_NE(it, dst_list.end());
    EXPECT_EQ(it->countdown_ptr, nullptr);
    ++it;
    EXPECT_NE(it, dst_list.end());
    EXPECT_EQ(it->countdown_ptr, &dst_counter);
    EXPECT_EQ(dst_counter, 10);
}

// ==========================================
// Тест-кейс 3: Модифицирующие операции (PopFront, InsertAfter, EraseAfter)
// ==========================================
struct DeletionSpy {
    ~DeletionSpy() {
        if (deletion_counter_ptr) {
            ++(*deletion_counter_ptr);
        }
    }
    int* deletion_counter_ptr = nullptr;
};

TEST(SingleLinkedListTest, PopFront) {
    SingleLinkedList<int> numbers{ 3, 14, 15, 92, 6 };
    numbers.PopFront();
    EXPECT_EQ(numbers, SingleLinkedList<int>({14, 15, 92, 6}));

    SingleLinkedList<DeletionSpy> list;
    list.PushFront(DeletionSpy{});
    int deletion_counter = 0;
    list.begin()->deletion_counter_ptr = &deletion_counter;
    
    EXPECT_EQ(deletion_counter, 0);
    list.PopFront();
    EXPECT_EQ(deletion_counter, 1);
}

TEST(SingleLinkedListTest, BeforeBegin) {
    SingleLinkedList<int> empty_list;
    const auto& const_empty_list = empty_list;
    EXPECT_EQ(empty_list.before_begin(), empty_list.cbefore_begin());
    EXPECT_EQ(++empty_list.before_begin(), empty_list.begin());
    EXPECT_EQ(++empty_list.cbefore_begin(), const_empty_list.begin());

    SingleLinkedList<int> numbers{ 1, 2, 3, 4 };
    const auto& const_numbers = numbers;
    EXPECT_EQ(numbers.before_begin(), numbers.cbefore_begin());
    EXPECT_EQ(++numbers.before_begin(), numbers.begin());
    EXPECT_EQ(++numbers.cbefore_begin(), const_numbers.begin());
}

TEST(SingleLinkedListTest, InsertAfter) {
    // Вставка в начало пустого списка
    {
        SingleLinkedList<int> lst;
        const auto inserted_item_pos = lst.InsertAfter(lst.before_begin(), 123);
        EXPECT_EQ(lst, SingleLinkedList<int>({123}));
        EXPECT_EQ(inserted_item_pos, lst.begin());
        EXPECT_EQ(*inserted_item_pos, 123);
    }

    // Вставка в середину/начало непустого списка
    {
        SingleLinkedList<int> lst{ 1, 2, 3 };
        auto inserted_item_pos = lst.InsertAfter(lst.before_begin(), 123);

        EXPECT_EQ(inserted_item_pos, lst.begin());
        EXPECT_NE(inserted_item_pos, lst.end());
        EXPECT_EQ(*inserted_item_pos, 123);
        EXPECT_EQ(lst, SingleLinkedList<int>({123, 1, 2, 3}));

        inserted_item_pos = lst.InsertAfter(lst.begin(), 555);
        EXPECT_EQ(++SingleLinkedList<int>::Iterator(lst.begin()), inserted_item_pos);
        EXPECT_EQ(*inserted_item_pos, 555);
        EXPECT_EQ(lst, SingleLinkedList<int>({123, 555, 1, 2, 3}));
    }
}

TEST(SingleLinkedListTest, InsertAfterExceptionSafety) {
    bool exception_was_thrown = false;
    for (int max_copy_counter = 10; max_copy_counter >= 0; --max_copy_counter) {
        SingleLinkedList<ThrowOnCopy> list;
        list.PushFront(ThrowOnCopy{});
        list.PushFront(ThrowOnCopy{});
        list.PushFront(ThrowOnCopy{});
        
        try {
            int copy_counter = max_copy_counter;
            list.InsertAfter(list.cbegin(), ThrowOnCopy(copy_counter));
            EXPECT_EQ(list.GetSize(), 4u);
        }
        catch (const std::bad_alloc&) {
            exception_was_thrown = true;
            EXPECT_EQ(list.GetSize(), 3u);
            break;
        }
    }
    EXPECT_TRUE(exception_was_thrown);
}

TEST(SingleLinkedListTest, EraseAfter) {
    // Удаление первого элемента
    {
        SingleLinkedList<int> lst{ 1, 2, 3, 4 };
        const auto& const_lst = lst;
        const auto item_after_erased = lst.EraseAfter(const_lst.cbefore_begin());
        EXPECT_EQ(lst, SingleLinkedList<int>({2, 3, 4}));
        EXPECT_EQ(item_after_erased, lst.begin());
    }
    // Удаление из середины
    {
        SingleLinkedList<int> lst{ 1, 2, 3, 4 };
        const auto item_after_erased = lst.EraseAfter(lst.cbegin());
        EXPECT_EQ(lst, SingleLinkedList<int>({1, 3, 4}));
        EXPECT_EQ(item_after_erased, ++lst.begin());
    }
    // Удаление последнего элемента
    {
        SingleLinkedList<int> lst{ 1, 2, 3, 4 };
        const auto item_after_erased = lst.EraseAfter(++(++lst.cbegin()));
        EXPECT_EQ(lst, SingleLinkedList<int>({1, 2, 3}));
        EXPECT_EQ(item_after_erased, lst.end());
    }
    // Проверка вызова деструкторов при удалении
    {
        SingleLinkedList<DeletionSpy> list;
        list.PushFront(DeletionSpy{});
        list.PushFront(DeletionSpy{});
        list.PushFront(DeletionSpy{});
        
        auto after_begin = ++list.begin();
        int deletion_counter = 0;
        after_begin->deletion_counter_ptr = &deletion_counter;
        
        EXPECT_EQ(deletion_counter, 0u);
        list.EraseAfter(list.cbegin());
        EXPECT_EQ(deletion_counter, 1u);
    }
}