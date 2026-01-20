#include <gtest/gtest.h>
#include "JsonValue.hpp"

using namespace json;

// Тесты для проверки типов
TEST(JsonValueTest, NullType) {
    JsonValue value(nullptr);
    EXPECT_TRUE(value.isNull());
    EXPECT_FALSE(value.isBool());
    EXPECT_FALSE(value.isNumber());
    EXPECT_FALSE(value.isString());
    EXPECT_FALSE(value.isArray());
    EXPECT_FALSE(value.isObject());
    EXPECT_EQ(value.typeName(), "null");
}

TEST(JsonValueTest, BoolType) {
    JsonValue valueTrue(true);
    JsonValue valueFalse(false);

    EXPECT_TRUE(valueTrue.isBool());
    EXPECT_TRUE(valueFalse.isBool());
    EXPECT_TRUE(valueTrue.asBool());
    EXPECT_FALSE(valueFalse.asBool());
    EXPECT_EQ(valueTrue.typeName(), "boolean");
}

TEST(JsonValueTest, NumberType) {
    JsonValue intValue(42);
    JsonValue doubleValue(3.14);

    EXPECT_TRUE(intValue.isNumber());
    EXPECT_TRUE(doubleValue.isNumber());
    EXPECT_DOUBLE_EQ(intValue.asNumber(), 42.0);
    EXPECT_DOUBLE_EQ(doubleValue.asNumber(), 3.14);
    EXPECT_EQ(intValue.typeName(), "number");
}

TEST(JsonValueTest, StringType) {
    JsonValue value("Hello, World!");

    EXPECT_TRUE(value.isString());
    EXPECT_EQ(value.asString(), "Hello, World!");
    EXPECT_EQ(value.typeName(), "string");
}

TEST(JsonValueTest, EmptyString) {
    JsonValue value("");

    EXPECT_TRUE(value.isString());
    EXPECT_EQ(value.asString(), "");
}

TEST(JsonValueTest, ArrayType) {
    JsonArray arr = {JsonValue(1), JsonValue(2), JsonValue(3)};
    JsonValue value(arr);

    EXPECT_TRUE(value.isArray());
    EXPECT_EQ(value.size(), 3);
    EXPECT_EQ(value.typeName(), "array");
}

TEST(JsonValueTest, EmptyArray) {
    JsonArray arr;
    JsonValue value(arr);

    EXPECT_TRUE(value.isArray());
    EXPECT_EQ(value.size(), 0);
}

TEST(JsonValueTest, ObjectType) {
    JsonObject obj = {
        {"name", JsonValue("Alice")},
        {"age", JsonValue(25)}
    };
    JsonValue value(obj);

    EXPECT_TRUE(value.isObject());
    EXPECT_EQ(value.size(), 2);
    EXPECT_EQ(value.typeName(), "object");
}

TEST(JsonValueTest, EmptyObject) {
    JsonObject obj;
    JsonValue value(obj);

    EXPECT_TRUE(value.isObject());
    EXPECT_EQ(value.size(), 0);
}

// Тесты для доступа к значениям
TEST(JsonValueTest, BoolAccess) {
    JsonValue value(true);
    EXPECT_TRUE(value.asBool());
}

TEST(JsonValueTest, NumberAccess) {
    JsonValue value(42.5);
    EXPECT_DOUBLE_EQ(value.asNumber(), 42.5);
}

TEST(JsonValueTest, StringAccess) {
    JsonValue value("test");
    EXPECT_EQ(value.asString(), "test");
}

// Тесты для исключений при неверном типе
TEST(JsonValueTest, WrongTypeAccessThrows) {
    JsonValue value(42);  // Number

    EXPECT_THROW(value.asBool(), JsonException);
    EXPECT_THROW(value.asString(), JsonException);
    EXPECT_THROW(value.asArray(), JsonException);
    EXPECT_THROW(value.asObject(), JsonException);
}

// Тесты для работы с массивами
TEST(JsonValueTest, ArrayAccess) {
    JsonArray arr = {JsonValue(1), JsonValue(2), JsonValue(3)};
    JsonValue value(arr);

    EXPECT_EQ(value[0].asNumber(), 1.0);
    EXPECT_EQ(value[1].asNumber(), 2.0);
    EXPECT_EQ(value[2].asNumber(), 3.0);
}

TEST(JsonValueTest, ArrayModification) {
    JsonArray arr;
    JsonValue value(arr);

    value.push_back(JsonValue(10));
    value.push_back(JsonValue(20));
    value.push_back(JsonValue(30));

    EXPECT_EQ(value.size(), 3);
    EXPECT_EQ(value[0].asNumber(), 10.0);
    EXPECT_EQ(value[1].asNumber(), 20.0);
    EXPECT_EQ(value[2].asNumber(), 30.0);
}

TEST(JsonValueTest, ArrayErase) {
    JsonArray arr = {JsonValue(1), JsonValue(2), JsonValue(3)};
    JsonValue value(arr);

    value.erase(1);  // Удаляем элемент с индексом 1

    EXPECT_EQ(value.size(), 2);
    EXPECT_EQ(value[0].asNumber(), 1.0);
    EXPECT_EQ(value[1].asNumber(), 3.0);
}

TEST(JsonValueTest, ArrayIndexOutOfBounds) {
    JsonArray arr = {JsonValue(1), JsonValue(2)};
    JsonValue value(arr);

    EXPECT_THROW(value[5], JsonException);
}

// Тесты для работы с объектами
TEST(JsonValueTest, ObjectAccess) {
    JsonObject obj = {
        {"name", JsonValue("Alice")},
        {"age", JsonValue(25)}
    };
    JsonValue value(obj);

    EXPECT_EQ(value.at("name").asString(), "Alice");
    EXPECT_EQ(value.at("age").asNumber(), 25.0);
}

TEST(JsonValueTest, ObjectModification) {
    JsonObject obj;
    JsonValue value(obj);

    value["name"] = JsonValue("Bob");
    value["age"] = JsonValue(30);

    EXPECT_EQ(value.at("name").asString(), "Bob");
    EXPECT_EQ(value.at("age").asNumber(), 30.0);
    EXPECT_EQ(value.size(), 2);
}

TEST(JsonValueTest, ObjectContains) {
    JsonObject obj = {
        {"name", JsonValue("Charlie")},
        {"age", JsonValue(35)}
    };
    JsonValue value(obj);

    EXPECT_TRUE(value.contains("name"));
    EXPECT_TRUE(value.contains("age"));
    EXPECT_FALSE(value.contains("city"));
}

TEST(JsonValueTest, ObjectErase) {
    JsonObject obj = {
        {"name", JsonValue("David")},
        {"age", JsonValue(40)},
        {"city", JsonValue("New York")}
    };
    JsonValue value(obj);

    EXPECT_TRUE(value.erase("age"));
    EXPECT_FALSE(value.contains("age"));
    EXPECT_EQ(value.size(), 2);
}

TEST(JsonValueTest, ObjectKeyNotFound) {
    JsonObject obj = {{"name", JsonValue("Eve")}};
    JsonValue value(obj);

    EXPECT_THROW(value.at("nonexistent"), JsonException);
}

// Тесты для вложенных структур
TEST(JsonValueTest, NestedArray) {
    JsonArray inner1 = {JsonValue(1), JsonValue(2)};
    JsonArray inner2 = {JsonValue(3), JsonValue(4)};
    JsonArray outer = {JsonValue(inner1), JsonValue(inner2)};
    JsonValue value(outer);

    EXPECT_EQ(value.size(), 2);
    EXPECT_EQ(value[0].size(), 2);
    EXPECT_EQ(value[1].size(), 2);
    EXPECT_EQ(value[0][0].asNumber(), 1.0);
    EXPECT_EQ(value[1][1].asNumber(), 4.0);
}

TEST(JsonValueTest, NestedObject) {
    JsonObject address = {
        {"city", JsonValue("London")},
        {"zip", JsonValue("12345")}
    };
    JsonObject person = {
        {"name", JsonValue("Frank")},
        {"address", JsonValue(address)}
    };
    JsonValue value(person);

    EXPECT_EQ(value.at("name").asString(), "Frank");
    EXPECT_EQ(value.at("address").at("city").asString(), "London");
    EXPECT_EQ(value.at("address").at("zip").asString(), "12345");
}

TEST(JsonValueTest, ComplexNestedStructure) {
    JsonObject user1 = {
        {"name", JsonValue("Alice")},
        {"age", JsonValue(25)}
    };
    JsonObject user2 = {
        {"name", JsonValue("Bob")},
        {"age", JsonValue(30)}
    };
    JsonArray users = {JsonValue(user1), JsonValue(user2)};
    JsonObject root = {
        {"users", JsonValue(users)},
        {"count", JsonValue(2)}
    };
    JsonValue value(root);

    EXPECT_EQ(value.at("count").asNumber(), 2.0);
    EXPECT_EQ(value.at("users").size(), 2);
    EXPECT_EQ(value.at("users")[0].at("name").asString(), "Alice");
    EXPECT_EQ(value.at("users")[1].at("age").asNumber(), 30.0);
}

// Тесты для edge cases
TEST(JsonValueTest, DefaultConstructor) {
    JsonValue value;
    EXPECT_TRUE(value.isNull());
}

TEST(JsonValueTest, CopyConstructor) {
    JsonValue original(42);
    JsonValue copy = original;

    EXPECT_TRUE(copy.isNumber());
    EXPECT_EQ(copy.asNumber(), 42.0);
}

TEST(JsonValueTest, MoveConstructor) {
    JsonValue original("Hello");
    JsonValue moved = std::move(original);

    EXPECT_TRUE(moved.isString());
    EXPECT_EQ(moved.asString(), "Hello");
}

TEST(JsonValueTest, LargeArray) {
    JsonArray arr;
    for (int i = 0; i < 1000; ++i) {
        arr.push_back(JsonValue(i));
    }
    JsonValue value(arr);

    EXPECT_EQ(value.size(), 1000);
    EXPECT_EQ(value[0].asNumber(), 0.0);
    EXPECT_EQ(value[999].asNumber(), 999.0);
}

TEST(JsonValueTest, LargeObject) {
    JsonObject obj;
    for (int i = 0; i < 1000; ++i) {
        obj["key" + std::to_string(i)] = JsonValue(i);
    }
    JsonValue value(obj);

    EXPECT_EQ(value.size(), 1000);
    EXPECT_EQ(value.at("key0").asNumber(), 0.0);
    EXPECT_EQ(value.at("key999").asNumber(), 999.0);
}

// Тесты для специальных значений чисел
TEST(JsonValueTest, NegativeZero) {
    JsonValue value(-0.0);
    EXPECT_DOUBLE_EQ(value.asNumber(), 0.0);
}

TEST(JsonValueTest, VeryLargeNumber) {
    JsonValue value(1.7976931348623157e308);  // Близко к max double
    EXPECT_TRUE(value.isNumber());
}

TEST(JsonValueTest, VerySmallNumber) {
    JsonValue value(2.2250738585072014e-308);  // Близко к min positive double
    EXPECT_TRUE(value.isNumber());
}

// Тесты для Unicode строк
TEST(JsonValueTest, UnicodeString) {
    JsonValue value("Привет мир! 你好世界! こんにちは世界！");
    EXPECT_TRUE(value.isString());
    EXPECT_EQ(value.asString(), "Привет мир! 你好世界! こんにちは世界！");
}
