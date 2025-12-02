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
        printf("ПИШУ ЛИСТ: символ '%c' (ASCII %d), биты: ", 
               (root->symbol >= 32 && root->symbol < 127) ? root->symbol : '.', 
               root->symbol);
        write_bit(stream, 1);
        for (int i = 7; i >= 0; i--) {
            int bit = (root->symbol >> i) & 1;
            printf("%d", bit);
            write_bit(stream, bit);
        }
        printf("\n");
    } else {
        printf("ПИШУ УЗЕЛ\n");
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
        printf("ЧИТАЮ ЛИСТ: биты символа: ");
        for (int i = 0; i < 8; i++) {
            bit = read_bit(stream);
            if (bit == -1) return NULL;
            printf("%d", bit);
            symbol = (symbol << 1) | bit;
        }
        printf(" -> символ '%c' (ASCII %d)\n", 
               (symbol >= 32 && symbol < 127) ? symbol : '.', symbol);
        return create_node(symbol, 0);
    } else {
        // УЗЕЛ
        printf("ЧИТАЮ УЗЕЛ\n");
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
        printf("Сброс битового буфера после чтения дерева\n");
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
    printf("БИТ %d: ", bit);
    
    if (bit == 0) {
        current = current->left;
        printf("влево -> ");
    } else {
        current = current->right;
        printf("вправо -> ");
    }
    
    if (current == NULL) {
        printf("ОШИБКА: NULL узел!\n");
        break;
    }
    
    if (current->left == NULL && current->right == NULL) {
        printf("СИМВОЛ '%c' (ASCII %d)\n", 
               (current->symbol >= 32 && current->symbol < 127) ? current->symbol : '.',
               current->symbol);
        fputc(current->symbol, output);
        decoded_bytes++;
        current = root;
    } else {
        printf("узел\n");
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