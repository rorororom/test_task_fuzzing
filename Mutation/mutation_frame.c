/**
 * @file custom_mutator_mp3.c
 * @brief Кастомный мутатор для MP3 файлов для использования с AFL++.
 *
 * Этот мутатор выполняет следующие действия:
 * 1. Ищет заголовки MP3 фреймов в данных.
 * 2. Применяет мутации к заголовкам и содержимому MP3 фреймов.
 * 3. Случайно переставляет фреймы, сохраняя их целостность.
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "/home/subkhankulov/Рабочий стол/romashka/AFLplusplus/include/afl-fuzz.h"

// Максимальный размер файла
#define MAX_FILE 1048576 * 10

// Вероятность мутации
#define MUTATION_RATE 0.5

/**
 * @brief Структура данных кастомного мутатора.
 */
typedef struct my_mutator {
    afl_state_t *afl;       /**< Указатель на состояние AFL. */
    uint8_t *mutated_out;   /**< Буфер для мутации данных. */
    size_t data_size;       /**< Размер данных. */
} my_mutator_t;

#define CHECK(ptr, message)     \
    if (ptr == NULL) {              \
        perror(message);            \
        return 0;                   \
    }

/**
 * @brief Инициализация кастомного мутатора.
 * 
 * @param afl  Указатель на состояние AFL.
 * @param seed Начальное значение для генератора случайных чисел.
 * @return     Указатель на структуру мутатора.
 */
my_mutator_t* afl_custom_init(afl_state_t* afl, unsigned int seed) {
    srand(seed);

    my_mutator_t* data = calloc(1, sizeof(my_mutator_t));
    CHECK(data, "init calloc");
    CHECK((data->mutated_out = (uint8_t *)malloc(MAX_FILE)), "init malloc");

    data->afl = afl;
    data->data_size = 0;

    return data;
}

/**
 * @brief Функция для нахождения следующего заголовка MP3 фрейма.
 * 
 * @param data  Указатель на данные.
 * @param size  Размер данных.
 * @param start Начальная позиция для поиска.
 * @return      Позиция следующего заголовка MP3 фрейма или размер данных, если заголовок не найден.
 */
size_t find_next_frame(uint8_t* data, size_t size, size_t start) {
    for (size_t i = start; i < size - 1; i++) {
        if (data[i] == 0xFF && (data[i + 1] & 0xE0) == 0xE0) {
            return i;
        }
    }
    return size;
}

/**
 * @brief Основная функция мутации.
 * 
 * @param data         Указатель на структуру мутатора.
 * @param buf          Входные данные.
 * @param buf_size     Размер входных данных.
 * @param out_buf      Указатель на выходные данные.
 * @param add_buf      Дополнительные данные.
 * @param add_buf_size Размер дополнительных данных.
 * @param max_size     Максимальный размер выходных данных.
 * @return             Размер мутированных данных.
 */
size_t afl_custom_fuzz(my_mutator_t* data, uint8_t* buf, size_t buf_size, uint8_t** out_buf, uint8_t* add_buf, size_t add_buf_size, size_t max_size) {
    size_t mutated_size = buf_size <= max_size ? buf_size : max_size;
    memcpy(data->mutated_out, buf, mutated_size);

    size_t pos = 0;
    while (pos < mutated_size) {
        size_t frame_start = find_next_frame(data->mutated_out, mutated_size, pos);
        if (frame_start == mutated_size) break;  
        size_t next_frame_start = find_next_frame(data->mutated_out, mutated_size, frame_start + 1);
        
        for (size_t i = frame_start; i < next_frame_start && i < mutated_size; i++) {
            if ((rand() / (double)RAND_MAX) < MUTATION_RATE) {
                data->mutated_out[i] ^= 0xFF; 
            }
        }

        if ((rand() / (double)RAND_MAX) < MUTATION_RATE) {
            size_t other_frame_start = find_next_frame(data->mutated_out, mutated_size, next_frame_start + 1);
            if (other_frame_start < mutated_size) {
                size_t other_frame_end = find_next_frame(data->mutated_out, mutated_size, other_frame_start + 1);
                size_t frame_size = next_frame_start - frame_start;
                size_t other_frame_size = other_frame_end - other_frame_start;
                if (frame_size == other_frame_size) {
                    for (size_t i = 0; i < frame_size; i++) {
                        uint8_t temp = data->mutated_out[frame_start + i];
                        data->mutated_out[frame_start + i] = data->mutated_out[other_frame_start + i];
                        data->mutated_out[other_frame_start + i] = temp;
                    }
                }
            }
        }

        pos = next_frame_start;
    }

    *out_buf = data->mutated_out;
    return mutated_size;
}

/**
 * @brief Деинициализация кастомного мутатора.
 * 
 * @param data Указатель на структуру мутатора.
 */
void afl_custom_deinit(my_mutator_t *data) {
    free(data->mutated_out);
    free(data);
}
