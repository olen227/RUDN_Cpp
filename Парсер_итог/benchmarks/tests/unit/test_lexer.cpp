#include <gtest/gtest.h>
#include "Lexer.hpp"

using namespace json;

// Тесты для базовой токенизации
TEST(LexerTest, EmptyInput) {
    Lexer lexer("");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens.size(), 1);  // Только EOF
    EXPECT_EQ(tokens[0].type, TokenType::EndOfFile);
}

TEST(LexerTest, SingleTokens) {
    Lexer lexer("{}[],:true false null");
    auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 10);  // 9 токенов + EOF
    EXPECT_EQ(tokens[0].type, TokenType::LeftBrace);
    EXPECT_EQ(tokens[1].type, TokenType::RightBrace);
    EXPECT_EQ(tokens[2].type, TokenType::LeftBracket);
    EXPECT_EQ(tokens[3].type, TokenType::RightBracket);
    EXPECT_EQ(tokens[4].type, TokenType::Comma);
    EXPECT_EQ(tokens[5].type, TokenType::Colon);
    EXPECT_EQ(tokens[6].type, TokenType::True);
    EXPECT_EQ(tokens[7].type, TokenType::False);
    EXPECT_EQ(tokens[8].type, TokenType::Null);
}

// Тесты для строк
TEST(LexerTest, SimpleString) {
    Lexer lexer(R"("hello world")");
    auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 2);  // String + EOF
    EXPECT_EQ(tokens[0].type, TokenType::String);
    EXPECT_EQ(tokens[0].value, "hello world");
}

TEST(LexerTest, EmptyString) {
    Lexer lexer(R"("")");
    auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0].type, TokenType::String);
    EXPECT_EQ(tokens[0].value, "");
}

TEST(LexerTest, StringWithEscapeSequences) {
    Lexer lexer(R"("hello\nworld\t\"quote\"\\backslash")");
    auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0].type, TokenType::String);
    EXPECT_EQ(tokens[0].value, "hello\nworld\t\"quote\"\\backslash");
}

TEST(LexerTest, StringWithUnicodeEscape) {
    Lexer lexer(R"("\u0048\u0065\u006c\u006c\u006f")");  // "Hello"
    auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0].type, TokenType::String);
    EXPECT_EQ(tokens[0].value, "Hello");
}

TEST(LexerTest, StringWithCyrillicUnicode) {
    Lexer lexer(R"("\u041f\u0440\u0438\u0432\u0435\u0442")");  // "Привет"
    auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0].type, TokenType::String);
}

// Тесты для чисел
TEST(LexerTest, PositiveInteger) {
    Lexer lexer("42");
    auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0].type, TokenType::Number);
    EXPECT_EQ(tokens[0].value, "42");
}

TEST(LexerTest, NegativeInteger) {
    Lexer lexer("-42");
    auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0].type, TokenType::Number);
    EXPECT_EQ(tokens[0].value, "-42");
}

TEST(LexerTest, FloatingPoint) {
    Lexer lexer("3.14159");
    auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0].type, TokenType::Number);
    EXPECT_EQ(tokens[0].value, "3.14159");
}

TEST(LexerTest, NegativeFloatingPoint) {
    Lexer lexer("-2.71828");
    auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0].type, TokenType::Number);
    EXPECT_EQ(tokens[0].value, "-2.71828");
}

TEST(LexerTest, ScientificNotation) {
    Lexer lexer("1.23e10");
    auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0].type, TokenType::Number);
    EXPECT_EQ(tokens[0].value, "1.23e10");
}

TEST(LexerTest, ScientificNotationNegativeExponent) {
    Lexer lexer("1.23e-10");
    auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0].type, TokenType::Number);
    EXPECT_EQ(tokens[0].value, "1.23e-10");
}

TEST(LexerTest, Zero) {
    Lexer lexer("0");
    auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0].type, TokenType::Number);
    EXPECT_EQ(tokens[0].value, "0");
}

// Тесты для сложных JSON структур
TEST(LexerTest, SimpleObject) {
    Lexer lexer(R"({"name":"John","age":30})");
    auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 10);  // { "name" : "John" , "age" : 30 } EOF
    EXPECT_EQ(tokens[0].type, TokenType::LeftBrace);
    EXPECT_EQ(tokens[1].type, TokenType::String);
    EXPECT_EQ(tokens[1].value, "name");
    EXPECT_EQ(tokens[2].type, TokenType::Colon);
    EXPECT_EQ(tokens[3].type, TokenType::String);
    EXPECT_EQ(tokens[3].value, "John");
    EXPECT_EQ(tokens[4].type, TokenType::Comma);
    EXPECT_EQ(tokens[5].type, TokenType::String);
    EXPECT_EQ(tokens[5].value, "age");
    EXPECT_EQ(tokens[6].type, TokenType::Colon);
    EXPECT_EQ(tokens[7].type, TokenType::Number);
    EXPECT_EQ(tokens[7].value, "30");
    EXPECT_EQ(tokens[8].type, TokenType::RightBrace);
}

TEST(LexerTest, SimpleArray) {
    Lexer lexer(R"([1, 2, 3, 4, 5])");
    auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 12);  // [ 1 , 2 , 3 , 4 , 5 ] EOF
    EXPECT_EQ(tokens[0].type, TokenType::LeftBracket);
    EXPECT_EQ(tokens[1].type, TokenType::Number);
    EXPECT_EQ(tokens[10].type, TokenType::RightBracket);
}

TEST(LexerTest, NestedStructure) {
    Lexer lexer(R"({"users":[{"name":"Alice","age":25},{"name":"Bob","age":30}]})");
    auto tokens = lexer.tokenize();

    EXPECT_GT(tokens.size(), 10);
    EXPECT_EQ(tokens[0].type, TokenType::LeftBrace);
    EXPECT_EQ(tokens[tokens.size() - 1].type, TokenType::EndOfFile);
}

// Тесты для обработки пробелов
TEST(LexerTest, WhitespaceHandling) {
    Lexer lexer("  {  \n  \"key\"  \t:  \r\n  \"value\"  }  ");
    auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 6);  // { "key" : "value" } EOF
    EXPECT_EQ(tokens[0].type, TokenType::LeftBrace);
    EXPECT_EQ(tokens[1].type, TokenType::String);
    EXPECT_EQ(tokens[2].type, TokenType::Colon);
    EXPECT_EQ(tokens[3].type, TokenType::String);
    EXPECT_EQ(tokens[4].type, TokenType::RightBrace);
}

// Тесты для ошибок
TEST(LexerTest, UnterminatedString) {
    Lexer lexer(R"("unterminated)");
    EXPECT_THROW(lexer.tokenize(), LexerException);
}

TEST(LexerTest, InvalidKeyword) {
    Lexer lexer("tru");  // Неполное "true"
    EXPECT_THROW(lexer.tokenize(), LexerException);
}

TEST(LexerTest, InvalidNumber) {
    Lexer lexer("12.34.56");  // Две точки в числе
    EXPECT_THROW(lexer.tokenize(), LexerException);
}

TEST(LexerTest, InvalidEscapeSequence) {
    Lexer lexer(R"("\x")");  // Неверная escape-последовательность
    EXPECT_THROW(lexer.tokenize(), LexerException);
}

// Тесты для позиции токенов
TEST(LexerTest, TokenPosition) {
    Lexer lexer("{\n  \"key\": 123\n}");
    auto tokens = lexer.tokenize();

    EXPECT_EQ(tokens[0].line, 1);  // {
    EXPECT_EQ(tokens[1].line, 2);  // "key"
    EXPECT_EQ(tokens[3].line, 2);  // 123
    EXPECT_EQ(tokens[4].line, 3);  // }
}

// Тесты для граничных случаев
TEST(LexerTest, MaxNumber) {
    Lexer lexer("1.7976931348623157e308");  // Близко к max double
    auto tokens = lexer.tokenize();
    EXPECT_EQ(tokens[0].type, TokenType::Number);
}

TEST(LexerTest, MinNumber) {
    Lexer lexer("-1.7976931348623157e308");
    auto tokens = lexer.tokenize();
    EXPECT_EQ(tokens[0].type, TokenType::Number);
}

TEST(LexerTest, VerySmallNumber) {
    Lexer lexer("2.2250738585072014e-308");
    auto tokens = lexer.tokenize();
    EXPECT_EQ(tokens[0].type, TokenType::Number);
}
