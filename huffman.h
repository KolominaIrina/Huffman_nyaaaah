#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Узел дерева Хаффмана
typedef struct HuffmanNode {
    unsigned char symbol;      // Символ (для листьев)
    unsigned int frequency;    // Частота символа
    struct HuffmanNode *left;  // Левый потомок (0)
    struct HuffmanNode *right; // Правый потомок (1)
} HuffmanNode;

// Структура для хранения кода Хаффмана
typedef struct {
    unsigned char code[256];   // Битовый код (максимальная длина 256 бит)
    int code_length;           // Длина кода в битах
} HuffmanCode;

// Минимальная куча для построения дерева
typedef struct {
    HuffmanNode **nodes;       // Массив указателей на узлы
    int size;                  // Текущий размер кучи
    int capacity;              // Максимальная емкость
} MinHeap;

HuffmanNode* create_node(unsigned char symbol, unsigned int frequency);

// Функции для работы с деревом Хаффмана
MinHeap* create_min_heap(int capacity);
void insert_min_heap(MinHeap *heap, HuffmanNode *node);
HuffmanNode* extract_min(MinHeap *heap);
void build_min_heap(MinHeap *heap);

HuffmanNode* build_huffman_tree(unsigned int *frequencies);
void generate_codes(HuffmanNode *root, HuffmanCode *codes, unsigned char *buffer, int depth);
void free_huffman_tree(HuffmanNode *root);

#endif