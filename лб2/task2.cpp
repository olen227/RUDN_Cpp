#include <iostream>
using namespace std;

int stringLength(char* str) {
    int length = 0;
    while (*str != '\0') {
        length++;
        str++;
    }
    return length;
}

void reverseString(char* str) {
    int length = stringLength(str);
    
    if (length == 0) return;
    
    char* left = str;
    char* right = str + length - 1;
    
    while (left < right) {
        char temp = *left;
        *left = *right;
        *right = temp;
        
        left++;
        right--;
    }
}

int countCharacter(char* str, char target) {
    int count = 0;
    while (*str != '\0') {
        if (*str == target) {
            count++;
        }
        str++;
    }
    return count;
}

int main() {
    const int MAX_SIZE = 1024;
    char input[MAX_SIZE];
    
    cout << "Введите строку: ";
    
    if (!cin.getline(input, MAX_SIZE)) {
        cerr << "Ошибка чтения строки\n";
        return 1;
    }
    
    int length = stringLength(input);
    
    if (length == 0) {
        cout << "Строка пустая\n";
        return 0;
    }
    
    cout << "\nДлина строки: " << length << "\n";
    
    char reversed[MAX_SIZE];
    int i = 0;
    while (input[i] != '\0' && i < MAX_SIZE - 1) {
        reversed[i] = input[i];
        i++;
    }
    reversed[i] = '\0';
    
    reverseString(reversed);
    cout << "Перевернутая строка: " << reversed << "\n";
    
    char targetChar;
    cout << "\nВведите символ для поиска: ";
    cin >> targetChar;
    
    int occurrences = countCharacter(input, targetChar);
    cout << "Символ '" << targetChar << "' встречается " 
         << occurrences << " раз\n";
    
    return 0;
}
