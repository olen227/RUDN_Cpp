#include "Serializer.hpp"
#include <sstream>
#include <fstream>
#include <iomanip>
#include <algorithm>

namespace json {

Serializer::Serializer(const Options& options)
    : m_options(options) {}

std::string Serializer::indent(int depth) const {
    if (!m_options.prettyPrint) return "";
    return std::string(depth * m_options.indentSize, ' ');
}

std::string Serializer::newline() const {
    return m_options.prettyPrint ? "\n" : "";
}

std::string Serializer::escapeString(const std::string& str) const {
    std::ostringstream result;

    for (unsigned char c : str) {
        switch (c) {
            case '"':  result << "\\\""; break;
            case '\\': result << "\\\\"; break;
            case '\b': result << "\\b";  break;
            case '\f': result << "\\f";  break;
            case '\n': result << "\\n";  break;
            case '\r': result << "\\r";  break;
            case '\t': result << "\\t";  break;
            default:
                if (c < 0x20) {
                    // Управляющие символы
                    result << "\\u" << std::hex << std::setfill('0')
                           << std::setw(4) << static_cast<int>(c);
                } else if (m_options.escapeUnicode && c > 0x7F) {
                    // Unicode символы (если требуется экранирование)
                    result << "\\u" << std::hex << std::setfill('0')
                           << std::setw(4) << static_cast<int>(c);
                } else {
                    result << c;
                }
                break;
        }
    }

    return result.str();
}

void Serializer::serializeString(const std::string& str, std::ostream& os) const {
    os << '"' << escapeString(str) << '"';
}

void Serializer::serializeValue(const JsonValue& value, std::ostream& os, int depth) const {
    if (value.isNull()) {
        os << "null";
    } else if (value.isBool()) {
        os << (value.asBool() ? "true" : "false");
    } else if (value.isNumber()) {
        double num = value.asNumber();
        // Проверка на целое число
        if (num == static_cast<long long>(num) &&
            num >= -9007199254740992.0 && num <= 9007199254740992.0) {
            os << static_cast<long long>(num);
        } else {
            os << std::setprecision(17) << num;
        }
    } else if (value.isString()) {
        serializeString(value.asString(), os);
    } else if (value.isArray()) {
        serializeArray(value.asArray(), os, depth);
    } else if (value.isObject()) {
        serializeObject(value.asObject(), os, depth);
    }
}

void Serializer::serializeArray(const JsonArray& arr, std::ostream& os, int depth) const {
    if (arr.empty()) {
        os << "[]";
        return;
    }

    os << "[" << newline();

    for (size_t i = 0; i < arr.size(); ++i) {
        os << indent(depth + 1);
        serializeValue(arr[i], os, depth + 1);

        if (i < arr.size() - 1) {
            os << ",";
        }
        os << newline();
    }

    os << indent(depth) << "]";
}

void Serializer::serializeObject(const JsonObject& obj, std::ostream& os, int depth) const {
    if (obj.empty()) {
        os << "{}";
        return;
    }

    os << "{" << newline();

    // Опционально сортируем ключи
    std::vector<std::string> keys;
    keys.reserve(obj.size());
    for (const auto& [key, _] : obj) {
        keys.push_back(key);
    }

    if (m_options.sortKeys) {
        std::sort(keys.begin(), keys.end());
    }

    for (size_t i = 0; i < keys.size(); ++i) {
        const std::string& key = keys[i];
        const JsonValue& value = obj.at(key);

        os << indent(depth + 1);
        serializeString(key, os);
        os << ":" << (m_options.prettyPrint ? " " : "");
        serializeValue(value, os, depth + 1);

        if (i < keys.size() - 1) {
            os << ",";
        }
        os << newline();
    }

    os << indent(depth) << "}";
}

std::string Serializer::serialize(const JsonValue& value) const {
    std::ostringstream os;
    serialize(value, os);
    return os.str();
}

void Serializer::serialize(const JsonValue& value, std::ostream& os) const {
    serializeValue(value, os, 0);
}

bool Serializer::saveToFile(const JsonValue& value, const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    serialize(value, file);
    file << "\n"; // Добавляем перевод строки в конце файла
    return file.good();
}

// Статические методы
std::string Serializer::toString(const JsonValue& value, bool pretty) {
    Serializer serializer(pretty ? Options::pretty() : Options::compact());
    return serializer.serialize(value);
}

bool Serializer::toFile(const JsonValue& value, const std::string& filename, bool pretty) {
    Serializer serializer(pretty ? Options::pretty() : Options::compact());
    return serializer.saveToFile(value, filename);
}

} // namespace json
