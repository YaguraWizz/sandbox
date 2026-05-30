#include <gtest/gtest.h>

#include <key_invoke/key_invoke.hpp>
#include <nlohmann/json.hpp>

namespace js = nlohmann;
using namespace key_invoke;

namespace app_types {
    struct UserProfile {
        int id{0};
        std::string username;
        std::vector<std::string> roles;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UserProfile, id, username, roles)
} // namespace app_types

// ----------------------
// Helpers: custom extract
// ----------------------
namespace key_invoke {
    template <typename T>
    static T extract_value(value_extractor_tag<T>, const nlohmann::json& jv, std::string_view key) {
        if (!jv.is_object() || !jv.contains(key.data())) {
            throw key_not_found_error(key);
        }
        const nlohmann::json& value_from_json = jv.at(key.data());
        try {
            return value_from_json.get<T>();
        } catch (const nlohmann::json::exception& ex) {
            throw type_mismatch_error(key, typeid(T).name(), ex.what());
        }
    }
} // namespace key_invoke

// ----------------------
// Test: free function
// ----------------------
static void greet_user(int id, const std::string& name, double score) {
    (void)id;
    (void)name;
    (void)score;
}

TEST(KeyInvoke, FreeFunction_AllFromJson) {
    js::json j = {{"user_id", 123}, {"user_name", "Alice"}, {"score_val", 99.5}};

    EXPECT_NO_THROW({ invoke(j, greet_user, KeyArg("user_id"), KeyArg("user_name"), KeyArg("score_val")); });
}

TEST(KeyInvoke, FreeFunction_MixedArgs) {
    js::json j = {{"user_name", "Bob"}};

    EXPECT_NO_THROW({ invoke(j, greet_user, 456, KeyArg("user_name"), 75.0); });
}

TEST(KeyInvoke, MissingKey_Throws) {
    js::json j = {{"user_id", 123}, {"score_val", 50.0}};

    EXPECT_THROW(
        { invoke(j, greet_user, KeyArg("user_id"), KeyArg("user_name"), KeyArg("score_val")); }, std::exception);
}

// ----------------------
// Test: member function
// ----------------------
class Processor {
public:
    void calculate_sum(int a, int b, double factor) {
        (void)a;
        (void)b;
        (void)factor;
    }
};

TEST(KeyInvoke, MemberFunction_AllFromJson) {
    Processor p;
    js::json j = {{"val_a", 10}, {"val_b", 20}, {"multiplier", 2.5}};

    EXPECT_NO_THROW(
        { invoke(j, &Processor::calculate_sum, p, KeyArg("val_a"), KeyArg("val_b"), KeyArg("multiplier")); });
}

TEST(KeyInvoke, MemberFunction_MissingKey) {
    Processor p;
    js::json j = {{"val_a", 10}};

    EXPECT_THROW({ invoke(j, &Processor::calculate_sum, p, KeyArg("val_a"), KeyArg("val_b"), 1.0); }, std::exception);
}

// ----------------------
// Test: lambda
// ----------------------
TEST(KeyInvoke, Lambda_Invoke) {
    js::json j = {{"value_a", 10}, {"is_active", true}};

    auto lambda = [](int a, double b, bool c) {
        (void)a;
        (void)b;
        (void)c;
    };

    EXPECT_NO_THROW({ invoke(j, lambda, KeyArg("value_a"), 20.5, KeyArg("is_active")); });
}

// ----------------------
// Test: type mismatch
// ----------------------
TEST(KeyInvoke, TypeMismatch_Throws) {
    js::json j = {{"user_id", 111}, {"user_name", "Charlie"}, {"score_val", "fifty"}};

    EXPECT_THROW(
        { invoke(j, greet_user, KeyArg("user_id"), KeyArg("user_name"), KeyArg("score_val")); }, std::exception);
}

// ----------------------
// Test: user type extraction
// ----------------------
TEST(KeyInvoke, UserType_MissingNestedKey) {
    js::json j = {{"user_profile_data", {{"id", 102}, {"roles", {"guest"}}}}};

    auto fn = [](app_types::UserProfile, bool, std::string) {};

    EXPECT_THROW({ invoke(j, fn, KeyArg("user_profile_data"), false, "OFFLINE"); }, std::exception);
}


