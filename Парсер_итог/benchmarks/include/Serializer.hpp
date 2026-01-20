#ifndef SERIALIZER_HPP
#define SERIALIZER_HPP

#include "JsonValue.hpp"
#include <string>
#include <ostream>

namespace json {

// Класс для сериализации JSON в строку/файл
class Serializer {
public:
    // Опции форматирования
    struct Options {
        bool prettyPrint;
        int indentSize;
        bool sortKeys;
        bool escapeUnicode;

        // Конструктор по умолчанию
        Options() : prettyPrint(true), indentSize(2), sortKeys(false), escapeUnicode(false) {}

        static Options compact() {
            Options opts;
            opts.prettyPrint = false;
            return opts;
        }

        static Options pretty(int indent = 2) {
            Options opts;
            opts.prettyPrint = true;
            opts.indentSize = indent;
            return opts;
        }
    };

private:
    Options m_options;

    // Вспомогательные методы
    void serializeValue(const JsonValue& value, std::ostream& os, int depth) const;
    void serializeObject(const JsonObject& obj, std::ostream& os, int depth) const;
    void serializeArray(const JsonArray& arr, std::ostream& os, int depth) const;
    void serializeString(const std::string& str, std::ostream& os) const;

    // Escape строки для JSON
    std::string escapeString(const std::string& str) const;

    // Отступ
    std::string indent(int depth) const;
    std::string newline() const;

public:
    explicit Serializer(const Options& options = Options());

    // Сериализация в строку
    std::string serialize(const JsonValue& value) const;

    // Сериализация в поток
    void serialize(const JsonValue& value, std::ostream& os) const;

    // Сохранение в файл
    bool saveToFile(const JsonValue& value, const std::string& filename) const;

    // Статические методы для быстрой сериализации
    static std::string toString(const JsonValue& value, bool pretty = true);
    static bool toFile(const JsonValue& value, const std::string& filename, bool pretty = true);
};

} // namespace json

#endif // SERIALIZER_HPP
