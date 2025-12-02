#ifndef BITS_H
#define BITS_H

#include <stdio.h>

// Структура для битового ввода/вывода
typedef struct {
    FILE *file;           // Файловый поток
    unsigned char buffer; // Буфер для накопления битов
    int bit_count;        // Количество битов в буфере
    int mode;             // Режим: 0 - чтение, 1 - запись
} BitStream;

// Функции для работы с битовыми потоками
BitStream* open_bit_stream(const char *filename, const char *mode);
void close_bit_stream(BitStream *stream);
void write_bit(BitStream *stream, int bit);
int read_bit(BitStream *stream);
void flush_bits(BitStream *stream);

#endif