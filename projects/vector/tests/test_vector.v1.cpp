#include <gtest/gtest.h>

#include <numeric>
#include <string>
#include <utility>

#include <vector/v1/vector.hpp>

namespace {
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

    Vector<int> GenerateVector(size_t size) {
        Vector<int> v(size);
        std::iota(v.begin(), v.end(), 1);
        return v;
    }
} // namespace

TEST(V1, TemporaryObjConstructor) {
    const size_t size = 100000;
    Vector<int> moved_vector(GenerateVector(size));
    EXPECT_EQ(moved_vector.GetSize(), size);
}

TEST(V1, TemporaryObjOperator) {
    const size_t size = 100000;
    Vector<int> moved_vector;
    ASSERT_EQ(moved_vector.GetSize(), 0u);

    moved_vector = GenerateVector(size);
    EXPECT_EQ(moved_vector.GetSize(), size);
}

TEST(V1, NamedMoveConstructor) {
    const size_t size = 100000;
    Vector<int> vector_to_move(GenerateVector(size));
    ASSERT_EQ(vector_to_move.GetSize(), size);

    Vector<int> moved_vector(std::move(vector_to_move));
    EXPECT_EQ(moved_vector.GetSize(), size);
    EXPECT_EQ(vector_to_move.GetSize(), 0u);
}

TEST(V1, NamedMoveOperator) {
    const size_t size = 100000;
    Vector<int> vector_to_move(GenerateVector(size));
    ASSERT_EQ(vector_to_move.GetSize(), size);

    Vector<int> moved_vector = std::move(vector_to_move);
    EXPECT_EQ(moved_vector.GetSize(), size);
    EXPECT_EQ(vector_to_move.GetSize(), 0u);
}

TEST(V1, NoncopiableMoveConstructor) {
    const size_t size = 5;
    Vector<X> vector_to_move;
    for (size_t i = 0; i < size; ++i) {
        vector_to_move.PushBack(X(i));
    }

    Vector<X> moved_vector = std::move(vector_to_move);
    EXPECT_EQ(moved_vector.GetSize(), size);
    EXPECT_EQ(vector_to_move.GetSize(), 0u);

    for (size_t i = 0; i < size; ++i) {
        EXPECT_EQ(moved_vector[i].GetX(), i);
    }
}

TEST(V1, NoncopiablePushBack) {
    const size_t size = 5;
    Vector<X> v;
    for (size_t i = 0; i < size; ++i) {
        v.PushBack(X(i));
    }

    ASSERT_EQ(v.GetSize(), size);
    for (size_t i = 0; i < size; ++i) {
        EXPECT_EQ(v[i].GetX(), i);
    }
}

TEST(V1, NoncopiableInsert) {
    const size_t size = 5;
    Vector<X> v;
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

TEST(V1, NoncopiableErase) {
    const size_t size = 3;
    Vector<X> v;
    for (size_t i = 0; i < size; ++i) {
        v.PushBack(X(i));
    }

    auto it = v.Erase(v.begin());
    ASSERT_EQ(v.GetSize(), size - 1);
    EXPECT_EQ(it->GetX(), 1u);
}