#include <gtest/gtest.h>
#include <sandbox/vector.hpp>

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std::literals;
using namespace sandbox;

namespace {
    inline const uint32_t DEFAULT_COOKIE = 0xdeadbeef;

    struct TestObj {
        TestObj() = default;
        TestObj(const TestObj& other) = default;
        TestObj& operator=(const TestObj& other) = default;
        TestObj(TestObj&& other) = default;
        TestObj& operator=(TestObj&& other) = default;
        ~TestObj() {
            cookie = 0;
        }
        [[nodiscard]] bool IsAlive() const noexcept {
            return cookie == DEFAULT_COOKIE;
        }
        uint32_t cookie = DEFAULT_COOKIE;
    };

    struct Obj {
        Obj() {
            if (default_construction_throw_countdown > 0) {
                if (--default_construction_throw_countdown == 0) {
                    throw std::runtime_error("Oops");
                }
            }
            ++num_default_constructed;
        }

        explicit Obj(int id)
            : id(id) {
            ++num_constructed_with_id;
        }

        Obj(int id, std::string name)
            : id(id)
            , name(std::move(name)) {
            ++num_constructed_with_id_and_name;
        }

        Obj(const Obj& other)
            : id(other.id) {
            if (other.throw_on_copy) {
                throw std::runtime_error("Oops");
            }
            ++num_copied;
        }

        Obj(Obj&& other) noexcept
            : id(other.id) {
            ++num_moved;
        }

        Obj& operator=(const Obj& other) {
            if (this != &other) {
                id = other.id;
                name = other.name;
                ++num_assigned;
            }
            return *this;
        }

        Obj& operator=(Obj&& other) noexcept {
            id = other.id;
            name = std::move(other.name);
            ++num_move_assigned;
            return *this;
        }

        ~Obj() {
            ++num_destroyed;
            id = 0;
        }

        static int GetAliveObjectCount() {
            return num_default_constructed + num_copied + num_moved + num_constructed_with_id
                + num_constructed_with_id_and_name - num_destroyed;
        }

        static void ResetCounters() {
            default_construction_throw_countdown = 0;
            num_default_constructed = 0;
            num_copied = 0;
            num_moved = 0;
            num_destroyed = 0;
            num_constructed_with_id = 0;
            num_constructed_with_id_and_name = 0;
            num_assigned = 0;
            num_move_assigned = 0;
        }

        bool throw_on_copy = false;
        int id = 0;
        std::string name;

        static inline int default_construction_throw_countdown = 0;
        static inline int num_default_constructed = 0;
        static inline int num_constructed_with_id = 0;
        static inline int num_constructed_with_id_and_name = 0;
        static inline int num_copied = 0;
        static inline int num_moved = 0;
        static inline int num_destroyed = 0;
        static inline int num_assigned = 0;
        static inline int num_move_assigned = 0;
    };

    struct C {
        C() noexcept { ++def_ctor; }
        C(const C&) noexcept { ++copy_ctor; }
        C(C&&) noexcept { ++move_ctor; }
        C& operator=(const C& other) noexcept {
            if (this != &other) {
                ++copy_assign;
            }
            return *this;
        }
        C& operator=(C&&) noexcept {
            ++move_assign;
            return *this;
        }
        ~C() { ++dtor; }

        static void Reset() {
            def_ctor = 0;
            copy_ctor = 0;
            move_ctor = 0;
            copy_assign = 0;
            move_assign = 0;
            dtor = 0;
        }

        static inline size_t def_ctor = 0;
        static inline size_t copy_ctor = 0;
        static inline size_t move_ctor = 0;
        static inline size_t copy_assign = 0;
        static inline size_t move_assign = 0;
        static inline size_t dtor = 0;
    };

}  // namespace


TEST(VectorTest, ReserveAndCapacityBasics) {
    Obj::ResetCounters();
    const size_t SIZE = 100500;
    const size_t INDEX = 10;
    const int MAGIC = 42;

    {
        Vector<int> v;
        EXPECT_EQ(v.Capacity(), 0u);
        EXPECT_EQ(v.Size(), 0u);

        v.Reserve(SIZE);
        EXPECT_EQ(v.Capacity(), SIZE);
        EXPECT_EQ(v.Size(), 0u);
    }
    {
        Vector<int> v(SIZE);
        const auto& cv(v);
        EXPECT_EQ(v.Capacity(), SIZE);
        EXPECT_EQ(v.Size(), SIZE);
        EXPECT_EQ(v[0], 0);
        EXPECT_EQ(&v[0], &cv[0]);
        v[INDEX] = MAGIC;
        EXPECT_EQ(v[INDEX], MAGIC);
        EXPECT_EQ(&v[100] - &v[0], 100);

        v.Reserve(SIZE * 2);
        EXPECT_EQ(v.Size(), SIZE);
        EXPECT_EQ(v.Capacity(), SIZE * 2);
        EXPECT_EQ(v[INDEX], MAGIC);
    }
    {
        Vector<int> v(SIZE);
        v[INDEX] = MAGIC;
        const auto v_copy(v);
        EXPECT_NE(&v[INDEX], &v_copy[INDEX]);
        EXPECT_EQ(v[INDEX], v_copy[INDEX]);
    }
    {
        Vector<Obj> v;
        v.Reserve(SIZE);
        EXPECT_EQ(Obj::GetAliveObjectCount(), 0);
    }
    {
        Vector<Obj> v(SIZE);
        EXPECT_EQ(Obj::GetAliveObjectCount(), static_cast<int>(SIZE));
        const int old_copy_count = Obj::num_copied;
        const int old_move_count = Obj::num_moved;
        v.Reserve(SIZE * 2);
        EXPECT_EQ(Obj::GetAliveObjectCount(), static_cast<int>(SIZE));
        EXPECT_EQ(Obj::num_copied, old_copy_count);
        EXPECT_EQ(Obj::num_moved, old_move_count + static_cast<int>(SIZE));
    }
    EXPECT_EQ(Obj::GetAliveObjectCount(), 0);
}

// --- A set of tests for exception safety ---

TEST(VectorTest, ExceptionSafetyAndRollbacks) {
    const size_t SIZE = 100;
    Obj::ResetCounters();
    {
        Obj::default_construction_throw_countdown = SIZE / 2;
        EXPECT_THROW(Vector<Obj> v(SIZE), std::runtime_error);
        EXPECT_EQ(Obj::num_default_constructed, static_cast<int>(SIZE / 2 - 1));
        EXPECT_EQ(Obj::GetAliveObjectCount(), 0);
    }
    Obj::ResetCounters();
    {
        Vector<Obj> v(SIZE);
        v[SIZE / 2].throw_on_copy = true;
        EXPECT_THROW(Vector<Obj> v_copy(v), std::runtime_error);
        EXPECT_EQ(Obj::num_copied, static_cast<int>(SIZE / 2));
        EXPECT_EQ(Obj::GetAliveObjectCount(), static_cast<int>(SIZE));
    }
    Obj::ResetCounters();
    {
        Vector<Obj> v(SIZE);
        v[SIZE - 1].throw_on_copy = true;
        EXPECT_NO_THROW(v.Reserve(SIZE * 2));
        EXPECT_EQ(v.Capacity(), SIZE * 2);
        EXPECT_EQ(v.Size(), SIZE);
        EXPECT_EQ(Obj::GetAliveObjectCount(), static_cast<int>(SIZE));
    }
}

// --- A set of tests for Move semantics and Copy/Move Assignment ---

TEST(VectorTest, CopyAndMoveSemantics) {
    const size_t MEDIUM_SIZE = 100;
    const size_t LARGE_SIZE = 250;
    const int ID = 42;
    {
        Obj::ResetCounters();
        Vector<int> v(MEDIUM_SIZE);
        {
            auto v_copy(std::move(v));
            EXPECT_EQ(v_copy.Size(), MEDIUM_SIZE);
            EXPECT_EQ(v_copy.Capacity(), MEDIUM_SIZE);
        }
        EXPECT_EQ(Obj::GetAliveObjectCount(), 0);
    }
    {
        Obj::ResetCounters();
        {
            Vector<Obj> v(MEDIUM_SIZE);
            v[MEDIUM_SIZE / 2].id = ID;
            EXPECT_EQ(Obj::num_default_constructed, static_cast<int>(MEDIUM_SIZE));
            Vector<Obj> moved_from_v(std::move(v));
            EXPECT_EQ(moved_from_v.Size(), MEDIUM_SIZE);
            EXPECT_EQ(moved_from_v[MEDIUM_SIZE / 2].id == ID, true);
        }
        EXPECT_EQ(Obj::GetAliveObjectCount(), 0);
        EXPECT_EQ(Obj::num_moved, 0);
        EXPECT_EQ(Obj::num_copied, 0);
        EXPECT_EQ(Obj::num_default_constructed, static_cast<int>(MEDIUM_SIZE));
    }
    {
        Obj::ResetCounters();
        Vector<Obj> v_medium(MEDIUM_SIZE);
        v_medium[MEDIUM_SIZE / 2].id = ID;
        Vector<Obj> v_large(LARGE_SIZE);
        v_large = v_medium;
        EXPECT_EQ(v_large.Size(), MEDIUM_SIZE);
        EXPECT_EQ(v_large.Capacity(), LARGE_SIZE);
        EXPECT_EQ(v_large[MEDIUM_SIZE / 2].id, ID);
        EXPECT_EQ(Obj::GetAliveObjectCount(), static_cast<int>(MEDIUM_SIZE + MEDIUM_SIZE));
    }
    {
        Obj::ResetCounters();
        Vector<Obj> v(MEDIUM_SIZE);
        {
            Vector<Obj> v_large(LARGE_SIZE);
            v_large[LARGE_SIZE - 1].id = ID;
            v = v_large;
            EXPECT_EQ(v.Size(), LARGE_SIZE);
            EXPECT_EQ(v_large.Capacity(), LARGE_SIZE);
            EXPECT_EQ(v_large[LARGE_SIZE - 1].id, ID);
            EXPECT_EQ(Obj::GetAliveObjectCount(), static_cast<int>(LARGE_SIZE + LARGE_SIZE));
        }
        EXPECT_EQ(Obj::GetAliveObjectCount(), static_cast<int>(LARGE_SIZE));
    }
    EXPECT_EQ(Obj::GetAliveObjectCount(), 0);
    {
        Obj::ResetCounters();
        Vector<Obj> v(MEDIUM_SIZE);
        v[MEDIUM_SIZE - 1].id = ID;
        Vector<Obj> v_small(MEDIUM_SIZE / 2);
        v_small.Reserve(MEDIUM_SIZE + 1);
        const size_t num_copies = Obj::num_copied;
        v_small = v;
        EXPECT_EQ(v_small.Size(), v.Size());
        EXPECT_EQ(v_small.Capacity(), MEDIUM_SIZE + 1);
        v_small[MEDIUM_SIZE - 1].id = ID;
        EXPECT_EQ(static_cast<size_t>(Obj::num_copied) - num_copies, MEDIUM_SIZE - (MEDIUM_SIZE / 2));
    }
}

// --- Test suite for PushBack, PopBack and Resize ---

TEST(VectorTest, PushBackPopBackAndResize) {
    const size_t ID = 42;
    const size_t SIZE = 100500;
    {
        Obj::ResetCounters();
        Vector<Obj> v;
        v.Resize(SIZE);
        EXPECT_EQ(v.Size(), SIZE);
        EXPECT_EQ(v.Capacity(), SIZE);
        EXPECT_EQ(Obj::num_default_constructed, static_cast<int>(SIZE));
    }
    EXPECT_EQ(Obj::GetAliveObjectCount(), 0);

    {
        const size_t NEW_SIZE = 10000;
        Obj::ResetCounters();
        Vector<Obj> v(SIZE);
        v.Resize(NEW_SIZE);
        EXPECT_EQ(v.Size(), NEW_SIZE);
        EXPECT_EQ(v.Capacity(), SIZE);
        EXPECT_EQ(Obj::num_destroyed, static_cast<int>(SIZE - NEW_SIZE));
    }
    EXPECT_EQ(Obj::GetAliveObjectCount(), 0);
    {
        Obj::ResetCounters();
        Vector<Obj> v(SIZE);
        Obj o{ ID };
        v.PushBack(o);
        EXPECT_EQ(v.Size(), SIZE + 1);
        EXPECT_EQ(v.Capacity(), SIZE * 2);
        EXPECT_EQ(v[SIZE].id, ID);
        EXPECT_EQ(Obj::num_default_constructed, static_cast<int>(SIZE));
        EXPECT_EQ(Obj::num_copied, 1);
        EXPECT_EQ(Obj::num_constructed_with_id, 1);
        EXPECT_EQ(Obj::num_moved, static_cast<int>(SIZE));
    }
    EXPECT_EQ(Obj::GetAliveObjectCount(), 0);
    {
        Obj::ResetCounters();
        Vector<Obj> v(SIZE);
        v.PushBack(Obj{ ID });
        EXPECT_EQ(v.Size(), SIZE + 1);
        EXPECT_EQ(v.Capacity(), SIZE * 2);
        EXPECT_EQ(v[SIZE].id, ID);
        EXPECT_EQ(Obj::num_default_constructed, static_cast<int>(SIZE));
        EXPECT_EQ(Obj::num_copied, 0);
        EXPECT_EQ(Obj::num_constructed_with_id, 1);
        EXPECT_EQ(Obj::num_moved, static_cast<int>(SIZE + 1));
    }
    {
        Obj::ResetCounters();
        Vector<Obj> v;
        v.PushBack(Obj{ ID });
        v.PopBack();
        EXPECT_EQ(v.Size(), 0u);
        EXPECT_EQ(v.Capacity(), 1u);
        EXPECT_EQ(Obj::GetAliveObjectCount(), 0);
    }
    {
        Vector<TestObj> v(1);
        ASSERT_EQ(v.Size(), v.Capacity());
        v.PushBack(v[0]);
        EXPECT_TRUE(v[0].IsAlive());
        EXPECT_TRUE(v[1].IsAlive());
    }
    {
        Vector<TestObj> v(1);
        ASSERT_EQ(v.Size(), v.Capacity());
        v.PushBack(std::move(v[0]));
        EXPECT_TRUE(v[0].IsAlive());
        EXPECT_TRUE(v[1].IsAlive());
    }
}

// --- Test suite for EmplaceBack ---

TEST(VectorTest, EmplaceBackMechanics) {
    const int ID = 42;
    {
        Obj::ResetCounters();
        Vector<Obj> v;
        auto& elem = v.EmplaceBack(ID, "Ivan"s);
        EXPECT_EQ(v.Capacity(), 1u);
        EXPECT_EQ(v.Size(), 1u);
        EXPECT_EQ(&elem, &v[0]);
        EXPECT_EQ(v[0].id, ID);
        EXPECT_EQ(v[0].name, "Ivan"s);
        EXPECT_EQ(Obj::num_constructed_with_id_and_name, 1);
        EXPECT_EQ(Obj::GetAliveObjectCount(), 1);
    }
    EXPECT_EQ(Obj::GetAliveObjectCount(), 0);
    {
        Vector<TestObj> v(1);
        ASSERT_EQ(v.Size(), v.Capacity());
        v.EmplaceBack(v[0]);
        EXPECT_TRUE(v[0].IsAlive());
        EXPECT_TRUE(v[1].IsAlive());
    }
}

// --- Test Suite for Iterators, Insert, Emplace and Erase ---

TEST(VectorTest, IteratorsInsertEmplaceAndErase) {
    const size_t SIZE = 10;
    const int ID = 42;
    {
        Obj::ResetCounters();
        Vector<int> v(SIZE);
        const auto& cv(v);
        v.PushBack(1);
        EXPECT_EQ(&*v.begin(), &v[0]);
        *v.begin() = 2;
        EXPECT_EQ(v[0], 2);
        EXPECT_EQ(v.end() - v.begin(), static_cast<std::ptrdiff_t>(v.Size()));
        EXPECT_EQ(v.begin(), cv.begin());
        EXPECT_EQ(v.end(), cv.end());
        EXPECT_EQ(v.cbegin(), cv.begin());
        EXPECT_EQ(v.cend(), cv.end());
    }
    {
        Obj::ResetCounters();
        Vector<Obj> v(SIZE);
        Obj obj{ 1 };
        Vector<Obj>::iterator pos = v.Insert(v.cbegin() + 1, obj);
        EXPECT_EQ(v.Size(), SIZE + 1);
        EXPECT_EQ(v.Capacity(), SIZE * 2);
        EXPECT_EQ(&*pos, &v[1]);
        EXPECT_EQ(v[1].id, obj.id);
        EXPECT_EQ(Obj::num_copied, 1);
        EXPECT_EQ(Obj::num_default_constructed, static_cast<int>(SIZE));
        EXPECT_EQ(Obj::GetAliveObjectCount(), static_cast<int>(SIZE + 2));
    }
    {
        Obj::ResetCounters();
        Vector<Obj> v;
        auto* pos = v.Emplace(v.end(), Obj{ 1 });
        EXPECT_EQ(v.Size(), 1u);
        EXPECT_GE(v.Capacity(), v.Size());
        EXPECT_EQ(&*pos, &v[0]);
        EXPECT_EQ(Obj::num_moved, 1);
        EXPECT_EQ(Obj::num_constructed_with_id, 1);
        EXPECT_EQ(Obj::num_copied, 0);
        EXPECT_EQ(Obj::num_assigned, 0);
        EXPECT_EQ(Obj::num_move_assigned, 0);
        EXPECT_EQ(Obj::GetAliveObjectCount(), 1);
    }
    {
        Obj::ResetCounters();
        Vector<Obj> v;
        v.Reserve(SIZE);
        auto* pos = v.Emplace(v.end(), Obj{ 1 });
        EXPECT_EQ(v.Size(), 1u);
        EXPECT_GE(v.Capacity(), v.Size());
        EXPECT_EQ(&*pos, &v[0]);
        EXPECT_EQ(Obj::num_moved, 1);
        EXPECT_EQ(Obj::num_constructed_with_id, 1);
        EXPECT_EQ(Obj::num_copied, 0);
        EXPECT_EQ(Obj::num_assigned, 0);
        EXPECT_EQ(Obj::num_move_assigned, 0);
        EXPECT_EQ(Obj::GetAliveObjectCount(), 1);
    }
    {
        Obj::ResetCounters();
        Vector<Obj> v(SIZE);
        Vector<Obj>::iterator pos = v.Insert(v.cbegin() + 1, Obj{ 1 });
        EXPECT_EQ(v.Size(), SIZE + 1);
        EXPECT_EQ(v.Capacity(), SIZE * 2);
        EXPECT_EQ(&*pos, &v[1]);
        EXPECT_EQ(v[1].id, 1);
        EXPECT_EQ(Obj::num_copied, 0);
        EXPECT_EQ(Obj::num_default_constructed, static_cast<int>(SIZE));
        EXPECT_EQ(Obj::GetAliveObjectCount(), static_cast<int>(SIZE + 1));
    }
    {
        Vector<TestObj> v(SIZE);
        v.Insert(v.cbegin() + 2, v[0]);
        EXPECT_TRUE(std::all_of(v.begin(), v.end(), [](const TestObj& obj) { return obj.IsAlive(); }));
    }
    {
        Vector<TestObj> v(SIZE);
        v.Insert(v.cbegin() + 2, std::move(v[0]));
        EXPECT_TRUE(std::all_of(v.begin(), v.end(), [](const TestObj& obj) { return obj.IsAlive(); }));
    }
    {
        Vector<TestObj> v(SIZE);
        v.Emplace(v.cbegin() + 2, std::move(v[0]));
        EXPECT_TRUE(std::all_of(v.begin(), v.end(), [](const TestObj& obj) { return obj.IsAlive(); }));
    }
    {
        Obj::ResetCounters();
        Vector<Obj> v(SIZE);
        auto* pos = v.Emplace(v.cbegin() + 1, ID, "Ivan"s);
        EXPECT_EQ(v.Size(), SIZE + 1);
        EXPECT_EQ(v.Capacity(), SIZE * 2);
        EXPECT_EQ(&*pos, &v[1]);
        EXPECT_EQ(v[1].id, ID);
        EXPECT_EQ(v[1].name, "Ivan"s);
        EXPECT_EQ(Obj::num_copied, 0);
        EXPECT_EQ(Obj::num_default_constructed, static_cast<int>(SIZE));
        EXPECT_EQ(Obj::num_moved, static_cast<int>(SIZE));
        EXPECT_EQ(Obj::num_move_assigned, 0);
        EXPECT_EQ(Obj::num_assigned, 0);
        EXPECT_EQ(Obj::GetAliveObjectCount(), static_cast<int>(SIZE + 1));
    }
    {
        Obj::ResetCounters();
        Vector<Obj> v(SIZE);
        auto* pos = v.Emplace(v.cbegin() + v.Size(), ID, "Ivan"s);
        EXPECT_EQ(v.Size(), SIZE + 1);
        EXPECT_EQ(v.Capacity(), SIZE * 2);
        EXPECT_EQ(&*pos, &v[SIZE]);
        EXPECT_EQ(v[SIZE].id, ID);
        EXPECT_EQ(v[SIZE].name, "Ivan"s);
        EXPECT_EQ(Obj::num_copied, 0);
        EXPECT_EQ(Obj::num_default_constructed, static_cast<int>(SIZE));
        EXPECT_EQ(Obj::num_moved, static_cast<int>(SIZE));
        EXPECT_EQ(Obj::num_move_assigned, 0);
        EXPECT_EQ(Obj::num_assigned, 0);
        EXPECT_EQ(Obj::GetAliveObjectCount(), static_cast<int>(SIZE + 1));
    }
    {
        Obj::ResetCounters();
        Vector<Obj> v(SIZE);
        v.Reserve(SIZE * 2);
        const int old_num_moved = Obj::num_moved;
        EXPECT_EQ(v.Capacity(), SIZE * 2);
        auto* pos = v.Emplace(v.cbegin() + 3, ID, "Ivan"s);
        EXPECT_EQ(v.Size(), SIZE + 1);
        EXPECT_EQ(&*pos, &v[3]);
        EXPECT_EQ(v[3].id, ID);
        EXPECT_EQ(v[3].name, "Ivan");
        EXPECT_EQ(Obj::num_copied, 0);
        EXPECT_EQ(Obj::num_default_constructed, static_cast<int>(SIZE));
        EXPECT_EQ(Obj::num_constructed_with_id_and_name, 1);
        EXPECT_EQ(Obj::num_moved, old_num_moved + 1);
        EXPECT_EQ(Obj::num_move_assigned, static_cast<int>(SIZE - 3));
        EXPECT_EQ(Obj::num_assigned, 0);
    }
    {
        Obj::ResetCounters();
        Vector<Obj> v(SIZE);
        v[2].id = ID;
        auto* pos = v.Erase(v.cbegin() + 1);
        EXPECT_EQ((pos - v.begin()), 1);
        EXPECT_EQ(v.Size(), SIZE - 1);
        EXPECT_EQ(v.Capacity(), SIZE);
        EXPECT_EQ(pos->id, ID);
        EXPECT_EQ(Obj::num_copied, 0);
        EXPECT_EQ(Obj::num_assigned, 0);
        EXPECT_EQ(Obj::num_move_assigned, static_cast<int>(SIZE - 2));
        EXPECT_EQ(Obj::num_moved, 0);
        EXPECT_EQ(Obj::GetAliveObjectCount(), static_cast<int>(SIZE - 1));
    }
}