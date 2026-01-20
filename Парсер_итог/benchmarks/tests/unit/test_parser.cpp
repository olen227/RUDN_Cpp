#include <gtest/gtest.h>
#include "Parser.hpp"

using namespace json;

// Тесты для парсинга примитивов
TEST(ParserTest, ParseNull) {
    auto value = Parser::parseString("null");
    EXPECT_TRUE(value.isNull());
}

TEST(ParserTest, ParseTrue) {
    auto value = Parser::parseString("true");
    EXPECT_TRUE(value.isBool());
    EXPECT_TRUE(value.asBool());
}

TEST(ParserTest, ParseFalse) {
    auto value = Parser::parseString("false");
    EXPECT_TRUE(value.isBool());
    EXPECT_FALSE(value.asBool());
}

TEST(ParserTest, ParseNumber) {
    auto value = Parser::parseString("42");
    EXPECT_TRUE(value.isNumber());
    EXPECT_DOUBLE_EQ(value.asNumber(), 42.0);
}

TEST(ParserTest, ParseNegativeNumber) {
    auto value = Parser::parseString("-42");
    EXPECT_TRUE(value.isNumber());
    EXPECT_DOUBLE_EQ(value.asNumber(), -42.0);
}

TEST(ParserTest, ParseFloat) {
    auto value = Parser::parseString("3.14159");
    EXPECT_TRUE(value.isNumber());
    EXPECT_DOUBLE_EQ(value.asNumber(), 3.14159);
}

TEST(ParserTest, ParseScientificNotation) {
    auto value = Parser::parseString("1.23e10");
    EXPECT_TRUE(value.isNumber());
    EXPECT_DOUBLE_EQ(value.asNumber(), 1.23e10);
}

TEST(ParserTest, ParseString) {
    auto value = Parser::parseString(R"("Hello, World!")");
    EXPECT_TRUE(value.isString());
    EXPECT_EQ(value.asString(), "Hello, World!");
}

TEST(ParserTest, ParseEmptyString) {
    auto value = Parser::parseString(R"("")");
    EXPECT_TRUE(value.isString());
    EXPECT_EQ(value.asString(), "");
}

TEST(ParserTest, ParseStringWithEscapes) {
    auto value = Parser::parseString(R"("Line1\nLine2\tTabbed")");
    EXPECT_TRUE(value.isString());
    EXPECT_EQ(value.asString(), "Line1\nLine2\tTabbed");
}

// Тесты для парсинга массивов
TEST(ParserTest, ParseEmptyArray) {
    auto value = Parser::parseString("[]");
    EXPECT_TRUE(value.isArray());
    EXPECT_EQ(value.size(), 0);
}

TEST(ParserTest, ParseSimpleArray) {
    auto value = Parser::parseString("[1, 2, 3, 4, 5]");
    EXPECT_TRUE(value.isArray());
    EXPECT_EQ(value.size(), 5);
    EXPECT_DOUBLE_EQ(value[0].asNumber(), 1.0);
    EXPECT_DOUBLE_EQ(value[4].asNumber(), 5.0);
}

TEST(ParserTest, ParseMixedArray) {
    auto value = Parser::parseString(R"([1, "two", true, null, 5.5])");
    EXPECT_TRUE(value.isArray());
    EXPECT_EQ(value.size(), 5);
    EXPECT_TRUE(value[0].isNumber());
    EXPECT_TRUE(value[1].isString());
    EXPECT_TRUE(value[2].isBool());
    EXPECT_TRUE(value[3].isNull());
    EXPECT_TRUE(value[4].isNumber());
}

TEST(ParserTest, ParseNestedArray) {
    auto value = Parser::parseString("[[1, 2], [3, 4], [5, 6]]");
    EXPECT_TRUE(value.isArray());
    EXPECT_EQ(value.size(), 3);
    EXPECT_EQ(value[0].size(), 2);
    EXPECT_DOUBLE_EQ(value[0][0].asNumber(), 1.0);
    EXPECT_DOUBLE_EQ(value[2][1].asNumber(), 6.0);
}

TEST(ParserTest, ParseDeepNestedArray) {
    auto value = Parser::parseString("[[[1, 2], [3, 4]], [[5, 6], [7, 8]]]");
    EXPECT_TRUE(value.isArray());
    EXPECT_EQ(value.size(), 2);
    EXPECT_DOUBLE_EQ(value[0][0][0].asNumber(), 1.0);
    EXPECT_DOUBLE_EQ(value[1][1][1].asNumber(), 8.0);
}

// Тесты для парсинга объектов
TEST(ParserTest, ParseEmptyObject) {
    auto value = Parser::parseString("{}");
    EXPECT_TRUE(value.isObject());
    EXPECT_EQ(value.size(), 0);
}

TEST(ParserTest, ParseSimpleObject) {
    auto value = Parser::parseString(R"({"name": "Alice", "age": 25})");
    EXPECT_TRUE(value.isObject());
    EXPECT_EQ(value.size(), 2);
    EXPECT_EQ(value.at("name").asString(), "Alice");
    EXPECT_DOUBLE_EQ(value.at("age").asNumber(), 25.0);
}

TEST(ParserTest, ParseObjectWithDifferentTypes) {
    auto value = Parser::parseString(R"({
        "string": "value",
        "number": 42,
        "float": 3.14,
        "bool": true,
        "null": null
    })");
    EXPECT_TRUE(value.isObject());
    EXPECT_EQ(value.size(), 5);
    EXPECT_EQ(value.at("string").asString(), "value");
    EXPECT_DOUBLE_EQ(value.at("number").asNumber(), 42.0);
    EXPECT_DOUBLE_EQ(value.at("float").asNumber(), 3.14);
    EXPECT_TRUE(value.at("bool").asBool());
    EXPECT_TRUE(value.at("null").isNull());
}

TEST(ParserTest, ParseNestedObject) {
    auto value = Parser::parseString(R"({
        "person": {
            "name": "Bob",
            "address": {
                "city": "New York",
                "zip": "10001"
            }
        }
    })");
    EXPECT_TRUE(value.isObject());
    EXPECT_EQ(value.at("person").at("name").asString(), "Bob");
    EXPECT_EQ(value.at("person").at("address").at("city").asString(), "New York");
}

TEST(ParserTest, ParseObjectWithArray) {
    auto value = Parser::parseString(R"({
        "users": [
            {"name": "Alice", "age": 25},
            {"name": "Bob", "age": 30}
        ]
    })");
    EXPECT_TRUE(value.isObject());
    EXPECT_TRUE(value.at("users").isArray());
    EXPECT_EQ(value.at("users").size(), 2);
    EXPECT_EQ(value.at("users")[0].at("name").asString(), "Alice");
    EXPECT_DOUBLE_EQ(value.at("users")[1].at("age").asNumber(), 30.0);
}

// Тесты для обработки пробелов и форматирования
TEST(ParserTest, ParseWithWhitespace) {
    auto value = Parser::parseString("  {  \"key\"  :  \"value\"  }  ");
    EXPECT_TRUE(value.isObject());
    EXPECT_EQ(value.at("key").asString(), "value");
}

TEST(ParserTest, ParseMultilineJSON) {
    auto value = Parser::parseString(R"(
        {
            "name": "Charlie",
            "age": 35,
            "hobbies": [
                "reading",
                "coding",
                "gaming"
            ]
        }
    )");
    EXPECT_TRUE(value.isObject());
    EXPECT_EQ(value.at("name").asString(), "Charlie");
    EXPECT_EQ(value.at("hobbies").size(), 3);
}

// Тесты для обработки ошибок (могут бросать LexerException или ParserException)
TEST(ParserTest, ParseInvalidJSON_MissingQuote) {
    EXPECT_THROW(Parser::parseString(R"({"key: "value"})"), std::runtime_error);
}

TEST(ParserTest, ParseInvalidJSON_MissingColon) {
    EXPECT_THROW(Parser::parseString(R"({"key" "value"})"), std::runtime_error);
}

TEST(ParserTest, ParseInvalidJSON_MissingComma) {
    EXPECT_THROW(Parser::parseString(R"({"key1": "value1" "key2": "value2"})"), std::runtime_error);
}

TEST(ParserTest, ParseInvalidJSON_TrailingComma) {
    // Trailing commas are technically invalid in JSON
    EXPECT_THROW(Parser::parseString(R"({"key": "value",})"), std::runtime_error);
}

TEST(ParserTest, ParseInvalidJSON_UnmatchedBrace) {
    EXPECT_THROW(Parser::parseString(R"({"key": "value")"), std::runtime_error);
}

TEST(ParserTest, ParseInvalidJSON_UnmatchedBracket) {
    EXPECT_THROW(Parser::parseString(R"([1, 2, 3)"), std::runtime_error);
}

TEST(ParserTest, ParseInvalidJSON_InvalidNumber) {
    EXPECT_THROW(Parser::parseString("12.34.56"), std::runtime_error);
}

TEST(ParserTest, ParseInvalidJSON_InvalidKeyword) {
    EXPECT_THROW(Parser::parseString("tru"), std::runtime_error);
}

TEST(ParserTest, ParseEmptyInput) {
    EXPECT_THROW(Parser::parseString(""), std::runtime_error);
}

TEST(ParserTest, ParseWhitespaceOnly) {
    EXPECT_THROW(Parser::parseString("   \n\t  "), std::runtime_error);
}

// Тесты для сложных структур
TEST(ParserTest, ParseComplexStructure) {
    auto value = Parser::parseString(R"({
        "company": "TechCorp",
        "employees": [
            {
                "id": 1,
                "name": "Alice",
                "skills": ["C++", "Python", "JavaScript"],
                "active": true
            },
            {
                "id": 2,
                "name": "Bob",
                "skills": ["Java", "Go"],
                "active": false
            }
        ],
        "metadata": {
            "version": 1.0,
            "lastUpdate": "2024-01-01"
        }
    })");

    EXPECT_TRUE(value.isObject());
    EXPECT_EQ(value.at("company").asString(), "TechCorp");
    EXPECT_EQ(value.at("employees").size(), 2);
    EXPECT_EQ(value.at("employees")[0].at("skills").size(), 3);
    EXPECT_EQ(value.at("employees")[0].at("skills")[0].asString(), "C++");
    EXPECT_TRUE(value.at("employees")[0].at("active").asBool());
    EXPECT_DOUBLE_EQ(value.at("metadata").at("version").asNumber(), 1.0);
}

// Тесты для граничных случаев
TEST(ParserTest, ParseArrayWith1000Elements) {
    std::string json = "[";
    for (int i = 0; i < 1000; ++i) {
        if (i > 0) json += ",";
        json += std::to_string(i);
    }
    json += "]";

    auto value = Parser::parseString(json);
    EXPECT_TRUE(value.isArray());
    EXPECT_EQ(value.size(), 1000);
    EXPECT_DOUBLE_EQ(value[0].asNumber(), 0.0);
    EXPECT_DOUBLE_EQ(value[999].asNumber(), 999.0);
}

TEST(ParserTest, ParseLongString) {
    std::string longStr(10000, 'x');
    std::string json = "\"" + longStr + "\"";

    auto value = Parser::parseString(json);
    EXPECT_TRUE(value.isString());
    EXPECT_EQ(value.asString().length(), 10000);
}

// Тесты для Unicode
TEST(ParserTest, ParseUnicodeString) {
    auto value = Parser::parseString(R"("\u0048\u0065\u006c\u006c\u006f")");
    EXPECT_TRUE(value.isString());
    EXPECT_EQ(value.asString(), "Hello");
}

TEST(ParserTest, ParseCyrillicUnicode) {
    auto value = Parser::parseString(R"("\u041f\u0440\u0438\u0432\u0435\u0442")");
    EXPECT_TRUE(value.isString());
}

// Тесты для специальных числовых значений
TEST(ParserTest, ParseZero) {
    auto value = Parser::parseString("0");
    EXPECT_TRUE(value.isNumber());
    EXPECT_DOUBLE_EQ(value.asNumber(), 0.0);
}

TEST(ParserTest, ParseNegativeZero) {
    auto value = Parser::parseString("-0");
    EXPECT_TRUE(value.isNumber());
}

TEST(ParserTest, ParseVeryLargeNumber) {
    auto value = Parser::parseString("1.7976931348623157e308");
    EXPECT_TRUE(value.isNumber());
}

TEST(ParserTest, ParseVerySmallNumber) {
    auto value = Parser::parseString("2.2250738585072014e-308");
    EXPECT_TRUE(value.isNumber());
}
