#include <iostream>
using namespace std;

int main() {
    int N;
    cout << "Введите количество дней для анализа: ";
    
    if (!(cin >> N)) {
        cerr << "Ошибка: введено не число\n";
        return 1;
    }
    
    if (N <= 0) {
        cerr << "Ошибка: количество дней должно быть положительным\n";
        return 1;
    }
    
    if (N > 1000) {
        cerr << "Ошибка: слишком много дней (максимум 10000)\n";
        return 1;
    }
    
    double* temperatures = new double[N];
    
    cout << "Введите температуры для каждого дня:\n";
    for (int i = 0; i < N; i++) {
        cout << "День " << (i + 1) << ": ";
        if (!(cin >> temperatures[i])) {
            cerr << "Ошибка: введено не число\n";
            delete[] temperatures;
            return 1;
        }
        
        if (temperatures[i] < -100 || temperatures[i] > 100) {
            cerr << "Предупреждение: необычная температура для Земли\n";
        }
    }
    
    double sum = 0;
    for (int i = 0; i < N; i++) {
        sum += temperatures[i];
    }
    double average = sum / N;
    
    double minTemp = temperatures[0];
    double maxTemp = temperatures[0];
    
    for (int i = 1; i < N; i++) {
        if (temperatures[i] < minTemp) {
            minTemp = temperatures[i];
        }
        if (temperatures[i] > maxTemp) {
            maxTemp = temperatures[i];
        }
    }
    
    int daysBelow = 0;
    for (int i = 0; i < N; i++) {
        if (temperatures[i] < average) {
            daysBelow++;
        }
    }
    
    cout << "\n=== РЕЗУЛЬТАТЫ АНАЛИЗА ===\n";
    cout << "Средняя температура: " << average << " градусов\n";
    cout << "Минимальная температура: " << minTemp << " градусов\n";
    cout << "Максимальная температура: " << maxTemp << " градусов\n";
    cout << "Дней с температурой ниже средней: " << daysBelow << "\n";
    
    delete[] temperatures;

}
