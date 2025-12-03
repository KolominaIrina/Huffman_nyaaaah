#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

#include "huffman.h"
#include "bits.h"

// Функции для работы с файлами
void calculate_frequencies(const char *filename, unsigned int *frequencies);
void write_tree_header(BitStream *stream, HuffmanNode *root);
HuffmanNode* read_tree_header(BitStream *stream);
void encode_file(const char *input_file, const char *output_file);
void decode_file(const char *input_file, const char *output_file);

// Функции для вывода информации по исполнению программы
void print_table(unsigned int *frequencies, HuffmanCode *codes);
void print_compression_ratio(const char *input_file, const char *output_file);

#endif