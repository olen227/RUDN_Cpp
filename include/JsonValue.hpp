#ifndef JSON_VALUE_HPP
#define JSON_VALUE_HPP

#include <string>
#include <vector>
#include <map>
#include <variant>
#include <memory>
#include <stdexcept>
#include <optional>

namespace json {

// Предварительное объявление
class JsonValue;

// Типы данных JSON
using JsonNull = std::nullptr_t;
using JsonBool = bool;
using JsonNumber = double;
using JsonString = std::string;
using JsonArray = std::vector<JsonValue>;
using JsonObject = std::map<std::string, JsonValue>;

// Исключение для ошибок JSON
class JsonException : public std::runtime_error {
public:
    explicit JsonException(const std::string& message)
        : std::runtime_error(message) {}
};

// Основной класс для представления JSON-значения
class JsonValue {
public:
    // Вариант для хранения всех возможных типов JSON
    using ValueType = std::variant<
        JsonNull,
        JsonBool,
        JsonNumber,
        JsonString,
        JsonArray,
        JsonObject
    >;

private:
    ValueType m_value;

public:
    // Конструкторы
    JsonValue() : m_value(nullptr) {}
    JsonValue(std::nullptr_t) : m_value(nullptr) {}
    JsonValue(bool value) : m_value(value) {}
    JsonValue(int value) : m_value(static_cast<double>(value)) {}
    JsonValue(double value) : m_value(value) {}
    JsonValue(const char* value) : m_value(std::string(value)) {}
    JsonValue(const std::string& value) : m_value(value) {}
    JsonValue(std::string&& value) : m_value(std::move(value)) {}
    JsonValue(const JsonArray& value) : m_value(value) {}
    JsonValue(JsonArray&& value) : m_value(std::move(value)) {}
    JsonValue(const JsonObject& value) : m_value(value) {}
    JsonValue(JsonObject&& value) : m_value(std::move(value)) {}

    // Проверки типа
    bool isNull() const { return std::holds_alternative<JsonNull>(m_value); }
    bool isBool() const { return std::holds_alternative<JsonBool>(m_value); }
    bool isNumber() const { return std::holds_alternative<JsonNumber>(m_value); }
    bool isString() const { return std::holds_alternative<JsonString>(m_value); }
    bool isArray() const { return std::holds_alternative<JsonArray>(m_value); }
    bool isObject() const { return std::holds_alternative<JsonObject>(m_value); }

    // Получение значений с проверкой типа
    bool asBool() const {
        if (!isBool()) throw JsonException("Значение не является булевым");
        return std::get<JsonBool>(m_value);
    }

    double asNumber() const {
        if (!isNumber()) throw JsonException("Значение не является числом");
        return std::get<JsonNumber>(m_value);
    }

    const std::string& asString() const {
        if (!isString()) throw JsonException("Значение не является строкой");
        return std::get<JsonString>(m_value);
    }

    std::string& asString() {
        if (!isString()) throw JsonException("Значение не является строкой");
        return std::get<JsonString>(m_value);
    }

    const JsonArray& asArray() const {
        if (!isArray()) throw JsonException("Значение не является массивом");
        return std::get<JsonArray>(m_value);
    }

    JsonArray& asArray() {
        if (!isArray()) throw JsonException("Значение не является массивом");
        return std::get<JsonArray>(m_value);
    }

    const JsonObject& asObject() const {
        if (!isObject()) throw JsonException("Значение не является объектом");
        return std::get<JsonObject>(m_value);
    }

    JsonObject& asObject() {
        if (!isObject()) throw JsonException("Значение не является объектом");
        return std::get<JsonObject>(m_value);
    }

    // Доступ к элементам массива
    JsonValue& operator[](size_t index) {
        if (!isArray()) throw JsonException("Значение не является массивом");
        auto& arr = std::get<JsonArray>(m_value);
        if (index >= arr.size()) throw JsonException("Индекс выходит за границы массива");
        return arr[index];
    }

    const JsonValue& operator[](size_t index) const {
        if (!isArray()) throw JsonException("Значение не является массивом");
        const auto& arr = std::get<JsonArray>(m_value);
        if (index >= arr.size()) throw JsonException("Индекс выходит за границы массива");
        return arr[index];
    }

    // Доступ к элементам объекта
    JsonValue& operator[](const std::string& key) {
        if (!isObject()) throw JsonException("Значение не является объектом");
        return std::get<JsonObject>(m_value)[key];
    }

    const JsonValue& at(const std::string& key) const {
        if (!isObject()) throw JsonException("Значение не является объектом");
        const auto& obj = std::get<JsonObject>(m_value);
        auto it = obj.find(key);
        if (it == obj.end()) throw JsonException("Ключ не найден: " + key);
        return it->second;
    }

    // Проверка наличия ключа
    bool contains(const std::string& key) const {
        if (!isObject()) return false;
        const auto& obj = std::get<JsonObject>(m_value);
        return obj.find(key) != obj.end();
    }

    // Размер (для массивов и объектов)
    size_t size() const {
        if (isArray()) return std::get<JsonArray>(m_value).size();
        if (isObject()) return std::get<JsonObject>(m_value).size();
        throw JsonException("Размер доступен только для массивов и объектов");
    }

    // Добавление элемента в массив
    void push_back(const JsonValue& value) {
        if (!isArray()) throw JsonException("push_back доступен только для массивов");
        std::get<JsonArray>(m_value).push_back(value);
    }

    // Удаление элемента из объекта по ключу
    bool erase(const std::string& key) {
        if (!isObject()) throw JsonException("erase по ключу доступен только для объектов");
        return std::get<JsonObject>(m_value).erase(key) > 0;
    }

    // Удаление элемента из массива по индексу
    void erase(size_t index) {
        if (!isArray()) throw JsonException("erase по индексу доступен только для массивов");
        auto& arr = std::get<JsonArray>(m_value);
        if (index >= arr.size()) throw JsonException("Индекс выходит за границы массива");
        arr.erase(arr.begin() + index);
    }

    // Получение названия типа
    std::string typeName() const {
        if (isNull()) return "null";
        if (isBool()) return "boolean";
        if (isNumber()) return "number";
        if (isString()) return "string";
        if (isArray()) return "array";
        if (isObject()) return "object";
        return "unknown";
    }

    // Поиск по пути (например, "user.address.city" или "items[0].name")
    std::optional<std::reference_wrapper<const JsonValue>> findByPath(const std::string& path) const;
    std::optional<std::reference_wrapper<JsonValue>> findByPath(const std::string& path);

    // Доступ к внутреннему variant (для сериализации)
    const ValueType& getValue() const { return m_value; }
    ValueType& getValue() { return m_value; }
};

} // namespace json

#endif // JSON_VALUE_HPP
