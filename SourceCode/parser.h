#ifndef PARSER_H
#define PARSER_H

#include "ring_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define MAX_PAYLOAD_SIZE 1000    // Максимальный размер полезных данных

typedef enum
{
        PARSER_OK = 0,        /**< Операция выполнена успешно */
        PARSER_INVALID_PARAM, /**< Передан недопустимый параметр */
} ParserError_t;

typedef enum
{
        PARSER_AWAIT_SYNC,              // Парсер ожидает синхропоследовательность
        PARSING_SYNC_SEQ,               // Парсер выполняет разбор синхропоследовательности
        PARSING_HEADER_PAYLOAD_SIZE,    // Парсер выполняет разбор размера полезных данных в заголовке
        PARSING_HEADER_PAYLOAD_TYPE,    // Парсер выполняет разбор типа полезных данных в заголовке
        PARSING_HEADER_PAYLOAD_CHECKSUM,    // Парсер выполняет разбор контрольной суммы полезных данных в заголовке
        PARSING_HEADER_HEADER_CHECKSUM,    // Парсер выполняет разбор контрольной суммы заголовка
        PARSING_PAYLOAD,                   // Парсер выполняет разбор тела пакета
        VERIFYING_CHECKSUM,                // Парсер проверяет контрольные суммы
        PARSING_COMPLETE,                  // Парсер завершил разбор пакета
        DISPATCH                           // Парсер передаёт разобранный пакет
} ParserState_t;

typedef struct
{
        size_t   m_sync_seq_size;    // Размер синхропоследовательности
        uint8_t *m_sync_seq;         // Синхропоследовательность

        ParserState_t m_state;    // Состояние парсера
        uint8_t       m_byte;     // Байт, обрабатываемый парсером в данный момент

        uint8_t m_packet_header[7];      // Заголовок пакета
        uint8_t m_packet_header_size;    // Размер заголовка пакета
        // Размер поля заголовка пакета в байтах, в котором хранится размер полезных данных пакета
        // (может иметь значения 1 или 2)
        uint8_t m_header_payload_size_field_size;
        // Смещение в заголовке пакета на начало поля, в котором записан тип полезных данных пакета
        uint8_t m_header_payload_type_field_offset;
        // Размер поля заголовка пакета в байтах, в котором хранится тип полезных данных пакета
        // (может иметь значения 1 или 2)
        uint8_t m_header_payload_type_field_size;
        // Смещение в заголовке пакета на начало поля, в котором записана контрольная полезных
        // данных пакета
        uint8_t m_header_payload_checksum_field_offset;
        // Размер поля заголовка пакета в байтах, в котором хранится контрольная сумма полезных
        // данных пакета (может иметь значения 0, 1 или 2)
        uint8_t m_header_payload_checksum_field_size;
        // Смещение в заголовке пакета на начало поля, в котором записана контрольная сумма
        // заголовка пакета
        uint8_t m_header_header_checksum_field_offset;

        // Полезные данные (тело) пакета
        uint8_t  m_packet_payload[MAX_PAYLOAD_SIZE];
        // Размер полезных данных пакета
        uint16_t m_packet_payload_size;
        // Сколько байт полезных данных было считано на данный момент
        uint16_t m_packet_payload_bytes_parsed;

        // Коллбек для передачи валидного пакета вышестоящему коду
        void (*m_callback)(uint16_t t_payload_type, const uint8_t *t_payload,
                           uint16_t t_payload_size);

        // Количество совпадений синхропоследовательности
        //
        // При m_sync_seq_hits == m_sync_seq_size завершаем парсинг синхропоследовательности и
        // переходим в состояние PARSING_HEADER
        uint8_t m_sync_seq_hits;

        // Флаг, указывающий на необходимость обработать ещё один байт для поля.
        uint8_t m_flag;

        /*
         * Флаг, который хранит результат проверки контрольной суммы полезных данных.
         *
         * Если установлен в 0, то контрольные суммы не совпадают.
         * 
         * Если не 0, то контрольные суммы совпадают.
         */
        uint8_t m_payload_checksum_flag;

        /*
         * Флаг, который хранит результат проверки контрольной суммы заголовка.
         *
         * Если установлен в 0, то контрольные суммы не совпадают.
         * 
         * Если не 0, то контрольные суммы совпадают.
         */
        uint8_t m_header_checksum_flag;
} Parser_t;

/**
 * @brief Инициализирует парсер
 * 
 * @param parser указатель на парсер
 * @param ring указатель на кольцевой буфер
 */
ParserError_t
PARSER_Init(Parser_t *t_parser, uint8_t t_sync_seq_size, uint8_t *t_sync_seq,
            void (*callback)(uint16_t t_payload_type, const uint8_t *t_payload,
                             uint16_t t_payload_size));


/**
 * Сбрасывает состояние парсера.
 * 
 * Очищает m_buffer.
 * 
 * Перезаписывает все значения m_packet_header на 0x00.
 * 
 * Устанавливает в 0 значения m_header_payload_size_field_size, m_header_payload_type_field_offset,
 * m_header_payload_type_field_size, m_header_payload_checksum_field_offset,
 * m_header_payload_checksum_field_size, m_header_header_checksum_field_offset.
 * 
 * Перезаписывает все значения m_packet_payload на 0x00.
 */
void
PARSER_Reset(Parser_t *parser);

/**
 * @brief Помещает байт в парсер для дальнейшей обработки
 */
void
PARSER_PutByte(Parser_t *t_parser, uint8_t t_byte);

/**
 * @brief: Функция для расчёта восьмибитной контрольной суммы
 */
uint8_t
crc8(const uint8_t *data, size_t size);

/**
 * @brief: Функция для расчёта шестнадцатибитной контрольной суммы
 */
uint16_t
crc16(const uint8_t *data, size_t data_size);

ParserError_t
PARSER_Parse(Parser_t *t_parser);

#ifdef __cplusplus
}
#endif

#endif    // PARSER_H
