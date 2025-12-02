#include <stdio.h>
#include <string.h>
#include "file_operations.h"

// Функция вывода справки по использованию
void print_help() {
    printf("=== Программа кодирования Хаффмана (Вариант 2) ===\n");
    printf("Алгоритм сжатия данных без потерь с сохранением дерева в файле\n\n");
    printf("Использование:\n");
    printf("  Кодирование:   huffman encode <входной_файл> <сжатый_файл>\n");
    printf("  Декодирование: huffman decode <сжатый_файл> <выходной_файл>\n");
    printf("\nПримеры:\n");
    printf("  huffman encode document.txt compressed.bin\n");
    printf("  huffman decode compressed.bin restored.txt\n");
    printf("\nОсобенности:\n");
    printf("  • Сжатый файл содержит дерево Хаффмана и закодированные данные\n");
    printf("  • Возможно многократное декодирование без потери информации\n");
    printf("  • Поддерживаются файлы любого типа и размера\n");
}

int main(int argc, char *argv[]) {
    printf("=== Программа кодирования Хаффмана ===\n");
    
    // Проверка количества аргументов
    if (argc != 4) {
        printf("Ошибка: неверное количество аргументов (ожидается 3, получено %d)\n\n", argc - 1);
        print_help();
        return 1;
    }
    
    // Обработка команды encode
    if (strcmp(argv[1], "encode") == 0) {
        printf("Режим: КОДИРОВАНИЕ\n");
        printf("Входной файл: %s\n", argv[2]);
        printf("Выходной файл: %s\n", argv[3]);
        printf("Начато кодирование...\n");
        
        encode_file(argv[2], argv[3]);
        
        printf("Кодирование завершено успешно!\n");
    }
    // Обработка команды decode
    else if (strcmp(argv[1], "decode") == 0) {
        printf("Режим: ДЕКОДИРОВАНИЕ\n");
        printf("Входной файл: %s\n", argv[2]);
        printf("Выходной файл: %s\n", argv[3]);
        printf("Начато декодирование...\n");
        
        decode_file(argv[2], argv[3]);
        
        printf("Декодирование завершено успешно!\n");
    }
    // Неизвестная команда
    else {
        printf("Ошибка: неизвестная команда '%s'\n\n", argv[1]);
        print_help();
        return 1;
    }
    
    return 0;
}