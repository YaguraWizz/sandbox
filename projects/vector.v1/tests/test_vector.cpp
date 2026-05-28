#include <gtest/gtest.h>

#include <numeric>
#include <string>
#include <utility>

#include <sandbox/vector.hpp>

using namespace std;
using namespace sandbox;

class X {
public:
    X() : X(5) {}
    X(size_t num) : x_(num) {}
    X(const X& other) = delete;
    X& operator=(const X& other) = delete;
    X(X&& other) noexcept { x_ = std::exchange(other.x_, 0); }
    X& operator=(X&& other) noexcept {
        x_ = std::exchange(other.x_, 0);
        return *this;
    }
    size_t GetX() const { return x_; }

private:
    size_t x_;
};

SimpleVector<int> GenerateVector(size_t size) {
    SimpleVector<int> v(size);
    iota(v.begin(), v.end(), 1);
    return v;
}


TEST(SimpleVectorTest, TemporaryObjConstructor) {
    const size_t size = 100000;
    SimpleVector<int> moved_vector(GenerateVector(size));
    EXPECT_EQ(moved_vector.GetSize(), size);
}

TEST(SimpleVectorTest, TemporaryObjOperator) {
    const size_t size = 100000;
    SimpleVector<int> moved_vector;
    ASSERT_EQ(moved_vector.GetSize(), 0u);

    moved_vector = GenerateVector(size);
    EXPECT_EQ(moved_vector.GetSize(), size);
}

TEST(SimpleVectorTest, NamedMoveConstructor) {
    const size_t size = 100000;
    SimpleVector<int> vector_to_move(GenerateVector(size));
    ASSERT_EQ(vector_to_move.GetSize(), size);

    SimpleVector<int> moved_vector(std::move(vector_to_move));
    EXPECT_EQ(moved_vector.GetSize(), size);
    EXPECT_EQ(vector_to_move.GetSize(), 0u);
}

TEST(SimpleVectorTest, NamedMoveOperator) {
    const size_t size = 100000;
    SimpleVector<int> vector_to_move(GenerateVector(size));
    ASSERT_EQ(vector_to_move.GetSize(), size);

    SimpleVector<int> moved_vector = std::move(vector_to_move);
    EXPECT_EQ(moved_vector.GetSize(), size);
    EXPECT_EQ(vector_to_move.GetSize(), 0u);
}

TEST(SimpleVectorTest, NoncopiableMoveConstructor) {
    const size_t size = 5;
    SimpleVector<X> vector_to_move;
    for (size_t i = 0; i < size; ++i) {
        vector_to_move.PushBack(X(i));
    }

    SimpleVector<X> moved_vector = std::move(vector_to_move);
    EXPECT_EQ(moved_vector.GetSize(), size);
    EXPECT_EQ(vector_to_move.GetSize(), 0u);

    for (size_t i = 0; i < size; ++i) {
        EXPECT_EQ(moved_vector[i].GetX(), i);
    }
}

TEST(SimpleVectorTest, NoncopiablePushBack) {
    const size_t size = 5;
    SimpleVector<X> v;
    for (size_t i = 0; i < size; ++i) {
        v.PushBack(X(i));
    }

    ASSERT_EQ(v.GetSize(), size);
    for (size_t i = 0; i < size; ++i) {
        EXPECT_EQ(v[i].GetX(), i);
    }
}

TEST(SimpleVectorTest, NoncopiableInsert) {
    const size_t size = 5;
    SimpleVector<X> v;
    for (size_t i = 0; i < size; ++i) {
        v.PushBack(X(i));
    }

    // beginning
    auto it_begin = v.Insert(v.begin(), X(size + 1));
    ASSERT_EQ(v.GetSize(), size + 1);
    EXPECT_EQ(v.begin()->GetX(), size + 1);
    EXPECT_EQ(it_begin, v.begin());

    // end
    auto it_end = v.Insert(v.end(), X(size + 2));
    ASSERT_EQ(v.GetSize(), size + 2);
    EXPECT_EQ((v.end() - 1)->GetX(), size + 2);
    EXPECT_EQ(it_end, v.end() - 1);

    // middle
    auto it_mid = v.Insert(v.begin() + 3, X(size + 3));
    ASSERT_EQ(v.GetSize(), size + 3);
    EXPECT_EQ((v.begin() + 3)->GetX(), size + 3);
    EXPECT_EQ(it_mid, v.begin() + 3);
}

TEST(SimpleVectorTest, NoncopiableErase) {
    const size_t size = 3;
    SimpleVector<X> v;
    for (size_t i = 0; i < size; ++i) {
        v.PushBack(X(i));
    }

    auto it = v.Erase(v.begin());
    ASSERT_EQ(v.GetSize(), size - 1);
    EXPECT_EQ(it->GetX(), 1u);
}