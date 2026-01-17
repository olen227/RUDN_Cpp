#ifndef PROGRESSBAR_HPP
#define PROGRESSBAR_HPP

#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace json {

class ProgressBar {
private:
    size_t m_total;
    size_t m_current;
    int m_barWidth;
    std::string m_description;
    std::chrono::high_resolution_clock::time_point m_startTime;
    bool m_finished;

    std::string formatSize(size_t bytes) const {
        const char* units[] = {"Б", "КБ", "МБ", "ГБ", "ТБ"};
        int unitIndex = 0;
        double size = static_cast<double>(bytes);

        while (size >= 1024.0 && unitIndex < 4) {
            size /= 1024.0;
            unitIndex++;
        }

        std::ostringstream oss;
        if (unitIndex == 0) {
            oss << bytes << " " << units[unitIndex];
        } else {
            oss << std::fixed << std::setprecision(2) << size << " " << units[unitIndex];
        }
        return oss.str();
    }

    std::string formatTime(double seconds) const {
        if (seconds < 60) {
            return std::to_string(static_cast<int>(seconds)) + "с";
        } else if (seconds < 3600) {
            int mins = static_cast<int>(seconds / 60);
            int secs = static_cast<int>(seconds) % 60;
            return std::to_string(mins) + "м " + std::to_string(secs) + "с";
        } else {
            int hours = static_cast<int>(seconds / 3600);
            int mins = static_cast<int>(seconds / 60) % 60;
            return std::to_string(hours) + "ч " + std::to_string(mins) + "м";
        }
    }

public:
    ProgressBar(size_t total, const std::string& description = "", int barWidth = 50)
        : m_total(total), m_current(0), m_barWidth(barWidth),
          m_description(description), m_finished(false) {
        m_startTime = std::chrono::high_resolution_clock::now();
    }

    void update(size_t current) {
        if (m_finished) return;

        m_current = current;
        if (m_current > m_total) m_current = m_total;

        // Вычисляем процент
        int percent = m_total > 0 ? static_cast<int>((m_current * 100) / m_total) : 0;

        // Вычисляем время
        auto now = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double>(now - m_startTime).count();

        // Вычисляем скорость и оставшееся время
        double speed = elapsed > 0 ? m_current / elapsed : 0;
        double eta = (speed > 0 && m_current < m_total) ? (m_total - m_current) / speed : 0;

        // Рисуем прогресс-бар
        std::cout << "\r";

        if (!m_description.empty()) {
            std::cout << m_description << ": ";
        }

        std::cout << "[";
        int pos = m_barWidth * percent / 100;
        for (int i = 0; i < m_barWidth; ++i) {
            if (i < pos) std::cout << "█";
            else if (i == pos) std::cout << "▓";
            else std::cout << "░";
        }
        std::cout << "] " << percent << "% ";

        // Показываем размер
        if (m_total >= 1024) {
            std::cout << "(" << formatSize(m_current) << " / " << formatSize(m_total) << ") ";
        } else {
            std::cout << "(" << m_current << " / " << m_total << ") ";
        }

        // Показываем скорость
        if (speed > 0 && m_total >= 1024) {
            std::cout << formatSize(static_cast<size_t>(speed)) << "/с ";
        }

        // Показываем ETA
        if (eta > 0 && m_current < m_total) {
            std::cout << "ETA: " << formatTime(eta);
        } else if (m_current >= m_total) {
            std::cout << "Готово! (" << formatTime(elapsed) << ")";
        }

        std::cout.flush();
    }

    void finish() {
        if (m_finished) return;
        m_finished = true;
        update(m_total);
        std::cout << std::endl;
    }

    void increment(size_t amount = 1) {
        update(m_current + amount);
    }

    void setDescription(const std::string& description) {
        m_description = description;
    }

    size_t getCurrent() const { return m_current; }
    size_t getTotal() const { return m_total; }
    bool isFinished() const { return m_finished; }
};

} // namespace json

#endif // PROGRESSBAR_HPP
