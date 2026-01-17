#include "JsonValue.hpp"
#include <sstream>

namespace json {

// Вспомогательная функция для разбора пути
static std::vector<std::string> splitPath(const std::string& path) {
    std::vector<std::string> parts;
    std::string current;
    bool inBracket = false;

    for (size_t i = 0; i < path.size(); ++i) {
        char c = path[i];

        if (c == '[') {
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
            inBracket = true;
        } else if (c == ']') {
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
            inBracket = false;
        } else if (c == '.' && !inBracket) {
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }

    if (!current.empty()) {
        parts.push_back(current);
    }

    return parts;
}

// Проверка, является ли строка числом (индексом массива)
static bool isArrayIndex(const std::string& str) {
    if (str.empty()) return false;
    for (char c : str) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    return true;
}

std::optional<std::reference_wrapper<const JsonValue>> JsonValue::findByPath(const std::string& path) const {
    if (path.empty()) {
        return std::cref(*this);
    }

    std::vector<std::string> parts = splitPath(path);
    const JsonValue* current = this;

    for (const auto& part : parts) {
        if (current->isObject()) {
            const auto& obj = current->asObject();
            auto it = obj.find(part);
            if (it == obj.end()) {
                return std::nullopt;
            }
            current = &(it->second);
        } else if (current->isArray()) {
            if (!isArrayIndex(part)) {
                return std::nullopt;
            }
            size_t index = std::stoul(part);
            const auto& arr = current->asArray();
            if (index >= arr.size()) {
                return std::nullopt;
            }
            current = &arr[index];
        } else {
            return std::nullopt;
        }
    }

    return std::cref(*current);
}

std::optional<std::reference_wrapper<JsonValue>> JsonValue::findByPath(const std::string& path) {
    if (path.empty()) {
        return std::ref(*this);
    }

    std::vector<std::string> parts = splitPath(path);
    JsonValue* current = this;

    for (const auto& part : parts) {
        if (current->isObject()) {
            auto& obj = current->asObject();
            auto it = obj.find(part);
            if (it == obj.end()) {
                return std::nullopt;
            }
            current = &(it->second);
        } else if (current->isArray()) {
            if (!isArrayIndex(part)) {
                return std::nullopt;
            }
            size_t index = std::stoul(part);
            auto& arr = current->asArray();
            if (index >= arr.size()) {
                return std::nullopt;
            }
            current = &arr[index];
        } else {
            return std::nullopt;
        }
    }

    return std::ref(*current);
}

} // namespace json
