#include "file_operations.h"
#include <stdlib.h> 
#include <string.h>

// Подсчет частот символов в файле
void calculate_frequencies(const char *filename, unsigned int *frequencies) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Не удалось открыть файл для подсчета частот");
        exit(1);
    }
    
    // Инициализация массива частот нулями
    for (int i = 0; i < 256; i++) {
        frequencies[i] = 0;
    }
    
    // Подсчет частот каждого символа
    int c;
    while ((c = fgetc(file)) != EOF) {
        frequencies[(unsigned char)c]++;
    }
    
    fclose(file);
}

// Рекурсивная запись дерева Хаффмана в файл
void write_tree_recursive(BitStream *stream, HuffmanNode *root) {
    if (root == NULL) return;
    
    if (root->left == NULL && root->right == NULL) {
        write_bit(stream, 1);
        for (int i = 7; i >= 0; i--) {
            int bit = (root->symbol >> i) & 1;
            write_bit(stream, bit);
        }
    } else {
        write_bit(stream, 0);
        write_tree_recursive(stream, root->left);
        write_tree_recursive(stream, root->right);
    }
}

// Запись дерева в заголовок сжатого файла
void write_tree_header(BitStream *stream, HuffmanNode *root) {
    write_tree_recursive(stream, root);
    flush_bits(stream); // Сбрасываем буфер после записи дерева
}

// Рекурсивное чтение дерева Хаффмана из файла
HuffmanNode* read_tree_recursive(BitStream *stream) {
    int bit = read_bit(stream);
    if (bit == -1) return NULL;
    
    if (bit == 1) {
        // ЛИСТ
        unsigned char symbol = 0;
        for (int i = 0; i < 8; i++) {
            bit = read_bit(stream);
            if (bit == -1) return NULL;
            symbol = (symbol << 1) | bit;
        }
        return create_node(symbol, 0);
    } else {
        // УЗЕЛ
        HuffmanNode *node = create_node(0, 0);
        node->left = read_tree_recursive(stream);
        node->right = read_tree_recursive(stream);
        return node;
    }
}

// Чтение дерева из заголовка сжатого файла
HuffmanNode* read_tree_header(BitStream *stream) {
    HuffmanNode *root = read_tree_recursive(stream);
    
    // сброс битового буфера после чтения дерева
    if (stream != NULL && stream->bit_count > 0) {
        stream->bit_count = 0;
        stream->buffer = 0;
    }
    
    return root;
}

// Кодирование файла алгоритмом Хаффмана
void encode_file(const char *input_file, const char *output_file) {
    printf("1. Подсчет частот символов...\n");
    unsigned int frequencies[256];
    calculate_frequencies(input_file, frequencies);
    
    // Подсчет уникальных символов для статистики
    int unique_symbols = 0;
    for (int i = 0; i < 256; i++) {
        if (frequencies[i] > 0) unique_symbols++;
    }
    printf("   Найдено уникальных символов: %d\n", unique_symbols);
    
    printf("2. Построение дерева Хаффмана...\n");
    HuffmanNode *root = build_huffman_tree(frequencies);
    
    printf("3. Генерация кодов...\n");
    HuffmanCode codes[256];
    unsigned char buffer[256];
    generate_codes(root, codes, buffer, 0);

    print_table(frequencies, codes);
    
    // Открытие файлов
    FILE *input = fopen(input_file, "rb");
    BitStream *output = open_bit_stream(output_file, "wb");
    
    if (!input || !output) {
        perror("Ошибка открытия файлов");
        exit(1);
    }
    
    printf("4. Запись дерева в заголовок...\n");

    long original_size;
    {
    FILE *src = fopen(input_file, "rb");
    fseek(src, 0, SEEK_END);
    original_size = ftell(src);
    fclose(src);
    }
fwrite(&original_size, sizeof(long), 1, output->file);
printf("   Размер исходного файла: %ld байт (записан в заголовок)\n", original_size);

    write_tree_header(output, root);

    flush_bits(output);
    
    printf("5. Кодирование данных...\n");
    int c;
    long encoded_bits = 0;
    while ((c = fgetc(input)) != EOF) {
        unsigned char symbol = (unsigned char)c;
        for (int i = 0; i < codes[symbol].code_length; i++) {
            write_bit(output, codes[symbol].code[i]);
            encoded_bits++;
        }
    }
    
    flush_bits(output);
    // Закрытие файлов и освобождение памяти
    fclose(input);
    close_bit_stream(output);
    free_huffman_tree(root);
    
    // Вывод статистики
    FILE *test = fopen(input_file, "rb");
    fseek(test, 0, SEEK_END);
    long input_size = ftell(test);
    fclose(test);
    
    printf("Кодирование завершено успешно!\n");
    printf("Статистика:\n");
    printf("  Размер исходного файла: %ld байт\n", input_size);
    printf("  Уникальных символов: %d\n", unique_symbols);
    printf("  Закодировано бит: %ld\n", encoded_bits);

    print_compression_ratio(input_file, output_file);

    printf("  Результат: %s -> %s\n", input_file, output_file);
}

// Декодирование файла
void decode_file(const char *input_file, const char *output_file) {
    printf("1. Чтение дерева Хаффмана из файла...\n");
    
    // Открытие файлов
    BitStream *input = open_bit_stream(input_file, "rb");
    FILE *output = fopen(output_file, "wb");
    
    if (!input || !output) {
        perror("Ошибка открытия файлов");
        exit(1);
    }

    long expected_bytes;
    fread(&expected_bytes, sizeof(long), 1, input->file);
    printf("   Ожидается символов: %ld\n", expected_bytes);
    
    // Чтение дерева из заголовка
    HuffmanNode *root = read_tree_header(input);
    if (root == NULL) {
        printf("Ошибка: не удалось прочитать дерево Хаффмана\n");
        exit(1);

    }
    
    printf("2. Декодирование данных...\n");
    HuffmanNode *current = root;
    int bit;
    long decoded_bytes = 0;
    
    // Декодирование: обход дерева по битам
    printf("=== НАЧАЛО ДЕКОДИРОВАНИЯ ДАННЫХ ===\n");
while ((bit = read_bit(input)) != -1 && decoded_bytes < expected_bytes) {
    
    if (bit == 0) {
        current = current->left;
    } else {
        current = current->right;
    }
    
    if (current == NULL) {
        printf("ОШИБКА: NULL узел!\n");
        break;
    }
    
    if (current->left == NULL && current->right == NULL) {
        
        fputc(current->symbol, output);
        decoded_bytes++;
        current = root;
    } 
}
printf("=== КОНЕЦ ДЕКОДИРОВАНИЯ ДАННЫХ ===\n");
    
    // Закрытие файлов и освобождение памяти
    close_bit_stream(input);
    fclose(output);
    free_huffman_tree(root);
    
    printf("Декодирование завершено успешно!\n");
    printf("  Декодировано байт: %ld\n", decoded_bytes);
    printf("  Результат: %s -> %s\n", input_file, output_file);
}

// Таблица для пользователя
   void print_table(unsigned int *frequencies, HuffmanCode *codes) {
    printf("\n");
    printf("┌──────────┬───────┬─────────────────┬──────────┬──────────┐\n");
    printf("│  Символ  │ ASCII │      Код        │  Длина   │  Частота │\n");
    printf("├──────────┼───────┼─────────────────┼──────────┼──────────┤\n");
    
    // Создаем массив для сортировки
    int indices[256], count = 0;
    for (int i = 0; i < 256; i++) {
        if (frequencies[i] > 0) indices[count++] = i;
    }
    
    // Сортировка по убыванию частоты
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (frequencies[indices[j]] > frequencies[indices[i]]) {
                int helper = indices[i];
                indices[i] = indices[j];
                indices[j] = helper;
            }
        }
    }
    
    // Вывод таблицы
    for (int idx = 0; idx < count; idx++) {
        int i = indices[idx];
        char ch = (i >= 32 && i < 127) ? (char)i : '.';
        
        printf("│   '%c'    │ %5d │ ", ch, i);
        
        // Вывод кода
        for (int j = 0; j < codes[i].code_length; j++) {
            printf("%d", codes[i].code[j]);
        }
        
        // Выравнивание пробелами
        for (int j = codes[i].code_length; j < 15; j++) printf(" ");
        
        printf("│ %8d │ %8d │\n", codes[i].code_length, frequencies[i]);
    }
    
    printf("└──────────┴───────┴─────────────────┴──────────┴──────────┘\n");
}

// Вывод коэффициента сжатия
void print_compression_ratio(const char *input_file, const char *output_file) {
    FILE *in = fopen(input_file, "rb");
    if (!in) return;
    fseek(in, 0, SEEK_END);
    long input_size = ftell(in);
    fclose(in);
    
    FILE *out = fopen(output_file, "rb");
    if (!out) return;
    fseek(out, 0, SEEK_END);
    long output_size = ftell(out);
    fclose(out);
    
    double ratio = (input_size > 0) ? 
                   100.0 * (1.0 - (double)output_size / input_size) : 0.0;
    
    printf("\nКоэффициент сжатия: %.2f%%\n", ratio);
    printf("(%ld байт -> %ld байт)\n", input_size, output_size);
}