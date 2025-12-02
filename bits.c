#include "bits.h"
#include <stdlib.h>
#include <string.h>

// Открытие битового потока
BitStream* open_bit_stream(const char *filename, const char *mode) {
    BitStream *stream = (BitStream*)malloc(sizeof(BitStream));
    stream->file = fopen(filename, mode);
    if (stream->file == NULL) {
        free(stream);
        return NULL;
    }
    stream->buffer = 0;
    stream->bit_count = 0;
    stream->mode = (strcmp(mode, "wb") == 0 || strcmp(mode, "ab") == 0) ? 1 : 0;
    return stream;
}

// Закрытие битового потока с записью оставшихся битов
void close_bit_stream(BitStream *stream) {
    if (stream->mode == 1 && stream->bit_count > 0) {
        // Дописываем оставшиеся биты, заполняя нулями
        //stream->buffer <<= (8 - stream->bit_count);
        //fputc(stream->buffer, stream->file);
    }
    fclose(stream->file);
    free(stream);
}

// Запись одного бита в поток
void write_bit(BitStream *stream, int bit) {
    stream->buffer = (stream->buffer << 1) | (bit & 1);
    stream->bit_count++;
    
    // Когда накопилось 8 бит - записываем байт в файл
    if (stream->bit_count == 8) {
        fputc(stream->buffer, stream->file);
        stream->buffer = 0;
        stream->bit_count = 0;
    }
}

// Чтение одного бита из потока
int read_bit(BitStream *stream) {
    // Если буфер пуст - читаем новый байт
    if (stream->bit_count == 0) {
        int c = fgetc(stream->file);
        if (c == EOF) return -1;  // Конец файла
        stream->buffer = (unsigned char)c;
        stream->bit_count = 8;
    }
    
    // Извлекаем старший бит из буфера
    int bit = (stream->buffer >> (stream->bit_count - 1)) & 1;
    stream->bit_count--;
    return bit;
}

// Принудительная запись битов из буфера
void flush_bits(BitStream *stream) {
    if (stream->mode == 1 && stream->bit_count > 0) {
        stream->buffer <<= (8 - stream->bit_count);
        fputc(stream->buffer, stream->file);
        stream->buffer = 0;
        stream->bit_count = 0;
    }
}