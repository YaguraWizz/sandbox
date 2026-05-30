#include <gtest/gtest.h>
#include <string_view>
#include <bimap/bimap.hpp>

using namespace std::literals;

// Тест базовых операций добавления и поиска
TEST(BiMapTest, BasicAddAndFind) {
    BiMap bimap;
    
    // Успешное добавление
    EXPECT_TRUE(bimap.Add("Cat"sv, "Koshka"sv));
    
    // Проверка взаимно-однозначного поиска
    EXPECT_EQ(bimap.FindValue("Cat"sv), "Koshka"sv);
    EXPECT_EQ(bimap.FindKey("Koshka"sv), "Cat"sv);
    
    // Поиск несуществующих ключей/значений перекрестно
    EXPECT_FALSE(bimap.FindKey("Cat"sv).has_value());
    EXPECT_FALSE(bimap.FindValue("Koshka"sv).has_value());
}

// Тест запрета дублирования ключей и значений
TEST(BiMapTest, DuplicatePrevention) {
    BiMap bimap;
    ASSERT_TRUE(bimap.Add("Cat"sv, "Koshka"sv));
    
    // Пытаемся добавить тот же ключ с другим значением
    EXPECT_FALSE(bimap.Add("Cat"sv, "Kitty"sv));
    EXPECT_EQ(bimap.FindValue("Cat"sv), "Koshka"sv);
    EXPECT_FALSE(bimap.FindValue("Kitty"sv).has_value());

    // Пытаемся добавить новое имя на то же значение
    EXPECT_FALSE(bimap.Add("Kot"sv, "Koshka"sv));
    EXPECT_EQ(bimap.FindKey("Koshka"sv), "Cat"sv);
}

// Тест конструктора копирования (глубокое копирование Pimpl)
TEST(BiMapTest, CopyConstructor) {
    BiMap bimap;
    bimap.Add("Cat"sv, "Koshka"sv);
    
    BiMap bimap_copy(bimap);
    
    // Проверяем, что в копии данные есть
    EXPECT_EQ(bimap_copy.FindValue("Cat"sv), "Koshka"sv);
    EXPECT_EQ(bimap_copy.FindKey("Koshka"sv), "Cat"sv);
    
    // Модифицируем копию и проверяем изолированность
    bimap_copy.Add("Dog"sv, "Sobaka"sv);
    EXPECT_EQ(bimap_copy.FindValue("Dog"sv), "Sobaka"sv);
    EXPECT_FALSE(bimap.FindValue("Dog"sv).has_value()); // В оригинале этой пары быть не должно
}

// Тест оператора присваивания копированием
TEST(BiMapTest, CopyAssignment) {
    BiMap bimap_source;
    bimap_source.Add("Dog"sv, "Sobaka"sv);
    
    BiMap bimap_dest;
    bimap_dest = bimap_source;
    
    EXPECT_EQ(bimap_dest.FindValue("Dog"sv), "Sobaka"sv);
    EXPECT_EQ(bimap_dest.FindKey("Sobaka"sv), "Dog"sv);
}

// Тест конструктора перемещения
TEST(BiMapTest, MoveConstructor) {
    BiMap bimap;
    bimap.Add("Cat"sv, "Koshka"sv);
    
    BiMap moved_bimap(std::move(bimap));
    
    EXPECT_EQ(moved_bimap.FindValue("Cat"sv), "Koshka"sv);
    EXPECT_EQ(moved_bimap.FindKey("Koshka"sv), "Cat"sv);
}

// Тест оператора перемещающего присваивания
TEST(BiMapTest, MoveAssignment) {
    BiMap bimap_source;
    bimap_source.Add("Dog"sv, "Sobaka"sv);
    
    BiMap bimap_dest;
    bimap_dest = std::move(bimap_source);
    
    EXPECT_EQ(bimap_dest.FindValue("Dog"sv), "Sobaka"sv);
    EXPECT_EQ(bimap_dest.FindKey("Sobaka"sv), "Dog"sv);
}