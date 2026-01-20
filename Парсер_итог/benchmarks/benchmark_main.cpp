#include <iostream>

// Объявления функций бенчмарков
void runParserBenchmarks();
void runMemoryBenchmarks();

int main() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║          JSON Parser - Complete Benchmark Suite                   ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════════╝\n\n";

    // Запуск бенчмарков производительности
    runParserBenchmarks();

    // Запуск бенчмарков памяти
    runMemoryBenchmarks();

    std::cout << "\n✓ All benchmarks completed successfully!\n\n";

    return 0;
}
