#include "huffman.h"

// Создание новой минимальной кучи
MinHeap* create_min_heap(int capacity) {
    MinHeap *heap = (MinHeap*)malloc(sizeof(MinHeap));
    heap->size = 0;
    heap->capacity = capacity;
    heap->nodes = (HuffmanNode**)malloc(capacity * sizeof(HuffmanNode*));
    return heap;
}

// Восстановление свойства минимальной кучи
void min_heapify(MinHeap *heap, int index) {
    int smallest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    // Сравниваем с левым потомком
    if (left < heap->size && 
        heap->nodes[left]->frequency < heap->nodes[smallest]->frequency) {
        smallest = left;
    }

    // Сравниваем с правым потомком
    if (right < heap->size && 
        heap->nodes[right]->frequency < heap->nodes[smallest]->frequency) {
        smallest = right;
    }

    // Если найден меньший элемент, меняем местами
    if (smallest != index) {
        HuffmanNode *temp = heap->nodes[index];
        heap->nodes[index] = heap->nodes[smallest];
        heap->nodes[smallest] = temp;
        min_heapify(heap, smallest);
    }
}

// Вставка узла в минимальную кучу
void insert_min_heap(MinHeap *heap, HuffmanNode *node) {
    if (heap->size == heap->capacity) {
        printf("Ошибка: куча переполнена!\n");
        return;
    }

    // Добавляем узел в конец
    heap->size++;
    int i = heap->size - 1;
    
    // Поднимаем узел до нужной позиции
    while (i > 0 && node->frequency < heap->nodes[(i-1)/2]->frequency) {
        heap->nodes[i] = heap->nodes[(i-1)/2];
        i = (i-1)/2;
    }
    
    heap->nodes[i] = node;
}

// Извлечение узла с минимальной частотой
HuffmanNode* extract_min(MinHeap *heap) {
    if (heap->size <= 0) return NULL;
    
    HuffmanNode *min = heap->nodes[0];
    heap->nodes[0] = heap->nodes[heap->size - 1];
    heap->size--;
    min_heapify(heap, 0);
    
    return min;
}

// Построение кучи из массива узлов
void build_min_heap(MinHeap *heap) {
    int n = heap->size - 1;
    for (int i = (n-1)/2; i >= 0; i--) {
        min_heapify(heap, i);
    }
}

// Создание нового узла дерева
HuffmanNode* create_node(unsigned char symbol, unsigned int frequency) {
    HuffmanNode *node = (HuffmanNode*)malloc(sizeof(HuffmanNode));
    node->symbol = symbol;
    node->frequency = frequency;
    node->left = node->right = NULL;
    return node;
}

// Построение дерева Хаффмана на основе частот символов
HuffmanNode* build_huffman_tree(unsigned int *frequencies) {
    // Создаем минимальную кучу
    MinHeap *heap = create_min_heap(256);
    
    // Добавляем все символы с ненулевой частотой
    for (int i = 0; i < 256; i++) {
        if (frequencies[i] > 0) {
            insert_min_heap(heap, create_node((unsigned char)i, frequencies[i]));
        }
    }
    
    // Построение дерева: объединяем узлы пока не останется один
    while (heap->size > 1) {
        // Извлекаем два узла с минимальной частотой
        HuffmanNode *left = extract_min(heap);
        HuffmanNode *right = extract_min(heap);
        
        // Создаем новый узел-родитель
        HuffmanNode *parent = create_node(0, left->frequency + right->frequency);
        parent->left = left;
        parent->right = right;
        
        // Добавляем новый узел обратно в кучу
        insert_min_heap(heap, parent);
    }
    
    // Последний оставшийся узел - корень дерева
    HuffmanNode *root = extract_min(heap);
    free(heap->nodes);
    free(heap);
    
    return root;
}

// Рекурсивная функция генерации кодов Хаффмана
void generate_codes_recursive(HuffmanNode *root, HuffmanCode *codes, unsigned char *buffer, int depth) {
    if (root == NULL) return;
    
    // Если достигли листа - сохраняем код
    if (root->left == NULL && root->right == NULL) {
        for (int i = 0; i < depth; i++) {
            codes[root->symbol].code[i] = buffer[i];
        }
        codes[root->symbol].code_length = depth;
        return;
    }
    
    // Левый потомок - добавляем бит 0
    if (root->left != NULL) {
        buffer[depth] = 0;
        generate_codes_recursive(root->left, codes, buffer, depth + 1);
    }
    
    // Правый потомок - добавляем бит 1
    if (root->right != NULL) {
        buffer[depth] = 1;
        generate_codes_recursive(root->right, codes, buffer, depth + 1);
    }
}

// Основная функция генерации кодов
void generate_codes(HuffmanNode *root, HuffmanCode *codes, unsigned char *buffer, int depth) {
    // Инициализируем все коды нулевой длиной
    for (int i = 0; i < 256; i++) {
        codes[i].code_length = 0;
    }
    generate_codes_recursive(root, codes, buffer, 0);
}

// Освобождение памяти дерева Хаффмана
void free_huffman_tree(HuffmanNode *root) {
    if (root == NULL) return;
    
    free_huffman_tree(root->left);
    free_huffman_tree(root->right);
    free(root);
}