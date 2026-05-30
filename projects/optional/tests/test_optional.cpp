#include <optional/optional.hpp> 
#include <gtest/gtest.h>
#include <memory>
#include <string>

struct C {
    C() noexcept {
        ++def_ctor;
    }
    C(const C& /*other*/) noexcept {
        ++copy_ctor;
    }
    C(C&& /*other*/) noexcept {
        ++move_ctor;
    }
    C& operator=(const C& other) noexcept {
        if (this != &other) {
            ++copy_assign;
        }
        return *this;
    }
    C& operator=(C&& /*other*/) noexcept {
        ++move_assign;
        return *this;
    }
    ~C() {
        ++dtor;
    }

    void Update() const& {
        ++const_lvalue_call_count;
    }

    void Update()& {
        ++lvalue_call_count;
    }

    void Update()&& {
        ++rvalue_call_count;
    }

    static size_t InstanceCount() {
        return def_ctor + copy_ctor + move_ctor - dtor;
    }

    static void Reset() {
        def_ctor = 0;
        copy_ctor = 0;
        move_ctor = 0;
        copy_assign = 0;
        move_assign = 0;
        dtor = 0;
        lvalue_call_count = 0;
        rvalue_call_count = 0;
        const_lvalue_call_count = 0;
    }

    inline static size_t def_ctor = 0;
    inline static size_t copy_ctor = 0;
    inline static size_t move_ctor = 0;
    inline static size_t copy_assign = 0;
    inline static size_t move_assign = 0;
    inline static size_t dtor = 0;

    inline static size_t lvalue_call_count = 0;
    inline static size_t rvalue_call_count = 0;
    inline static size_t const_lvalue_call_count = 0;
};

// Тестовый класс (Fixture) для удобного сброса счетчиков перед каждым тестом
class OptionalTest : public ::testing::Test {
protected:
    void SetUp() override {
        C::Reset();
    }
};

TEST_F(OptionalTest, Initialization) {
    {
        Optional<C> o;
        EXPECT_FALSE(o.HasValue());
        EXPECT_EQ(C::InstanceCount(), 0u);
    }
    EXPECT_EQ(C::InstanceCount(), 0u);

    C::Reset();
    {
        C c;
        Optional<C> o(c);
        EXPECT_TRUE(o.HasValue());
        EXPECT_EQ(C::def_ctor, 1u);
        EXPECT_EQ(C::copy_ctor, 1u);
        EXPECT_EQ(C::InstanceCount(), 2u);
    }
    EXPECT_EQ(C::InstanceCount(), 0u);

    C::Reset();
    {
        C c;
        Optional<C> o(std::move(c));
        EXPECT_TRUE(o.HasValue());
        EXPECT_EQ(C::def_ctor, 1u);
        EXPECT_EQ(C::move_ctor, 1u);
        EXPECT_EQ(C::copy_ctor, 0u);
        EXPECT_EQ(C::copy_assign, 0u);
        EXPECT_EQ(C::move_assign, 0u);
        EXPECT_EQ(C::InstanceCount(), 2u);
    }
    EXPECT_EQ(C::InstanceCount(), 0u);

    C::Reset();
    {
        C c;
        Optional<C> o1(c);
        const Optional<C> o2(o1);
        EXPECT_TRUE(o1.HasValue());
        EXPECT_TRUE(o2.HasValue());
        EXPECT_EQ(C::def_ctor, 1u);
        EXPECT_EQ(C::move_ctor, 0u);
        EXPECT_EQ(C::copy_ctor, 2u);
        EXPECT_EQ(C::copy_assign, 0u);
        EXPECT_EQ(C::move_assign, 0u);
        EXPECT_EQ(C::InstanceCount(), 3u);
    }
    EXPECT_EQ(C::InstanceCount(), 0u);

    C::Reset();
    {
        C c;
        Optional<C> o1(c);
        const Optional<C> o2(std::move(o1));
        EXPECT_EQ(C::def_ctor, 1u);
        EXPECT_EQ(C::copy_ctor, 1u);
        EXPECT_EQ(C::move_ctor, 1u);
        EXPECT_EQ(C::copy_assign, 0u);
        EXPECT_EQ(C::move_assign, 0u);
        EXPECT_EQ(C::InstanceCount(), 3u);
    }
    EXPECT_EQ(C::InstanceCount(), 0u);
}

TEST_F(OptionalTest, Assignment) {
    Optional<C> o1;
    Optional<C> o2;
    {  // Assign a value to empty
        C::Reset();
        C c;
        o1 = c;
        EXPECT_EQ(C::def_ctor, 1u);
        EXPECT_EQ(C::copy_ctor, 1u);
        EXPECT_EQ(C::dtor, 0u);
    }
    {  // Assign a non-empty to empty
        C::Reset();
        o2 = o1;
        EXPECT_EQ(C::copy_ctor, 1u);
        EXPECT_EQ(C::copy_assign, 0u);
        EXPECT_EQ(C::dtor, 0u);
    }
    {  // Assign non-empty to non-empty
        C::Reset();
        o2 = o1;
        EXPECT_EQ(C::copy_ctor, 0u);
        EXPECT_EQ(C::copy_assign, 1u);
        EXPECT_EQ(C::dtor, 0u);
    }
    {  // Assign empty to non-empty
        C::Reset();
        Optional<C> empty;
        o1 = empty;
        EXPECT_EQ(C::copy_ctor, 0u);
        EXPECT_EQ(C::dtor, 1u);
        EXPECT_FALSE(o1.HasValue());
    }
}

TEST_F(OptionalTest, MoveAssignment) {
    {  // Assign a value to empty
        Optional<C> o1;
        C::Reset();
        C c;
        o1 = std::move(c);
        EXPECT_EQ(C::def_ctor, 1u);
        EXPECT_EQ(C::move_ctor, 1u);
        EXPECT_EQ(C::dtor, 0u);
    }
    {  // Assign a non-empty to empty
        Optional<C> o1;
        Optional<C> o2{ C{} };
        C::Reset();
        o1 = std::move(o2);
        EXPECT_EQ(C::move_ctor, 1u);
        EXPECT_EQ(C::move_assign, 0u);
        EXPECT_EQ(C::dtor, 0u);
    }
    {  // Assign non-empty to non-empty
        Optional<C> o1{ C{} };
        Optional<C> o2{ C{} };
        C::Reset();
        o2 = std::move(o1);
        EXPECT_EQ(C::copy_ctor, 0u);
        EXPECT_EQ(C::move_assign, 1u);
        EXPECT_EQ(C::dtor, 0u);
    }
    {  // Assign empty to non-empty
        Optional<C> o1{ C{} };
        C::Reset();
        Optional<C> empty;
        o1 = std::move(empty);
        EXPECT_EQ(C::copy_ctor, 0u);
        EXPECT_EQ(C::move_ctor, 0u);
        EXPECT_EQ(C::move_assign, 0u);
        EXPECT_EQ(C::dtor, 1u);
        EXPECT_FALSE(o1.HasValue());
    }
}

TEST_F(OptionalTest, ValueAccess) {
    using namespace std::literals;
    {
        Optional<std::string> o;
        o = "hello"s;
        EXPECT_TRUE(o.HasValue());
        EXPECT_EQ(o.Value(), "hello"s);
        EXPECT_EQ(&*o, &o.Value());
        EXPECT_EQ(o->length(), 5u);
    }
    {
        Optional<int> o;
        // Проверяем, что вызов .Value() у пустого optional выбрасывает BadOptionalAccess
        EXPECT_THROW([[maybe_unused]] int v = o.Value(), BadOptionalAccess);
    }
}

TEST_F(OptionalTest, ResetMethod) {
    Optional<C> o{ C() };
    EXPECT_TRUE(o.HasValue());
    o.Reset();
    EXPECT_FALSE(o.HasValue());
}

TEST_F(OptionalTest, Emplace) {
    struct S {
        S(int i, std::unique_ptr<int>&& p)
            : i(i)
            , p(std::move(p))  //
        {
        }
        int i;
        std::unique_ptr<int> p;
    };

    Optional<S> o;
    o.Emplace(1, std::make_unique<int>(2));
    EXPECT_TRUE(o.HasValue());
    EXPECT_EQ(o->i, 1);
    EXPECT_EQ(*(o->p), 2);

    o.Emplace(3, std::make_unique<int>(4));
    EXPECT_TRUE(o.HasValue());
    EXPECT_EQ(o->i, 3);
    EXPECT_EQ(*(o->p), 4);
}

TEST_F(OptionalTest, RefQualifiedMethodOverloading) {
    {
        C::Reset();
        C val = *Optional<C>(C{});
        EXPECT_EQ(C::copy_ctor, 0u);
        EXPECT_EQ(C::move_ctor, 2u);
        EXPECT_EQ(C::def_ctor, 1u);
        EXPECT_EQ(C::copy_assign, 0u);
        EXPECT_EQ(C::move_assign, 0u);
    }
    {
        C::Reset();
        C val = Optional<C>(C{}).Value();
        EXPECT_EQ(C::copy_ctor, 0u);
        EXPECT_EQ(C::move_ctor, 2u);
        EXPECT_EQ(C::def_ctor, 1u);
        EXPECT_EQ(C::copy_assign, 0u);
        EXPECT_EQ(C::move_assign, 0u);
    }
    {
        C::Reset();
        Optional<C> opt(C{});
        (*opt).Update();
        EXPECT_EQ(C::lvalue_call_count, 1u);
        EXPECT_EQ(C::rvalue_call_count, 0u);
        (*std::move(opt)).Update();
        EXPECT_EQ(C::lvalue_call_count, 1u);
        EXPECT_EQ(C::rvalue_call_count, 1u);
    }
    {
        C::Reset();
        const Optional<C> opt(C{});
        (*opt).Update();
        EXPECT_EQ(C::const_lvalue_call_count, 1u);
    }
    {
        C::Reset();
        Optional<C> opt(C{});
        opt.Value().Update();
        EXPECT_EQ(C::lvalue_call_count, 1u);
        EXPECT_EQ(C::rvalue_call_count, 0u);
        std::move(opt).Value().Update();
        // В оригинальном тесте проверялся только lvalue_call_count после rvalue вызова.
        // Оставляем логику оригинального ассерта.
        EXPECT_EQ(C::lvalue_call_count, 1u); 
    }
    {
        C::Reset();
        const Optional<C> opt(C{});
        opt.Value().Update();
        EXPECT_EQ(C::const_lvalue_call_count, 1u);
    }
}
