#include "parser.h"

ParserError_t
PARSER_Init(Parser_t *t_parser, uint8_t t_sync_seq_size, uint8_t *t_sync_seq,
            void (*t_callback)(uint16_t t_payload_type, const uint8_t *t_payload,
                               uint16_t t_payload_size))
{
        if (t_sync_seq_size != 0) {
                t_parser->m_sync_seq_size = t_sync_seq_size;
        } else {
                return PARSER_INVALID_PARAM;
        }
        if (t_sync_seq != NULL) {
                t_parser->m_sync_seq = t_sync_seq;
        } else {
                return PARSER_INVALID_PARAM;
        }
        // Начальное состояние парсера - ожидание синхропоследовательности
        t_parser->m_state = PARSER_AWAIT_SYNC;
        t_parser->m_byte  = 0x00;

        // FIXME?: Инициализировать m_packet_header
        t_parser->m_packet_header_size                   = 0;
        t_parser->m_header_payload_size_field_size       = 0;
        t_parser->m_header_payload_type_field_offset     = 0;
        t_parser->m_header_payload_type_field_size       = 0;
        t_parser->m_header_payload_checksum_field_offset = 0;
        t_parser->m_header_payload_checksum_field_size   = 0;
        t_parser->m_header_header_checksum_field_offset  = 0;

        // FIXME?: Инициализировать m_packet_payload
        t_parser->m_packet_payload_size         = 0;
        t_parser->m_packet_payload_bytes_parsed = 0;

        if (t_callback != NULL) {
                t_parser->m_callback = t_callback;
        } else {
                return PARSER_INVALID_PARAM;
        }

        t_parser->m_sync_seq_hits = 0;

        t_parser->m_flag = 0;

        t_parser->m_payload_checksum_flag = 0;
        t_parser->m_header_checksum_flag  = 0;

        return PARSER_OK;
}

void
PARSER_Reset(Parser_t *parser)
{}

void
PARSER_PutByte(Parser_t *t_parser, uint8_t t_byte)
{
        t_parser->m_byte = t_byte;
}

/*
 * Источник: https://devcoons.com/crc8/
 */
uint8_t
crc8(const uint8_t *data, size_t data_size)
{
        uint8_t crc = 0x00;
        uint8_t extract;
        uint8_t sum;
        for (size_t i = 0; i < data_size; i++) {
                extract = *data;
                for (char tempI = 8; tempI; tempI--) {
                        sum = (crc ^ extract) & 0x01;
                        crc >>= 1;
                        if (sum)
                                crc ^= 0x8C;
                        extract >>= 1;
                }
                data++;
        }
        return crc;
}

/*
 * Источник: https://ru.wikibooks.org/wiki/Реализации_алгоритмов/Циклический_избыточный_код ->
 *           раздел сайта "Пример программы расчёта CRC-16 CCITT на языке Си"
 */
/**
 * Name  : CRC-16 CCITT
 * Poly  : 0x1021    x^16 + x^12 + x^5 + 1
 * Init  : 0xFFFF
 * Revert: false
 * XorOut: 0x0000
 * Check : 0x29B1 ("123456789")
 * MaxLen: 4095 байт (32767 бит) - обнаружение одинарных, двойных, тройных и всех нечетных ошибок
 */
uint16_t
crc16(const uint8_t *data, size_t data_size)
{
        uint16_t crc = 0xFFFF;
        size_t   i;

        while (data_size--) {
                crc ^= *data++ << 8;

                for (i = 0; i < 8; i++)
                        crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
        }
        return crc;
}

ParserError_t
PARSER_Parse(Parser_t *t_parser)
{
        if (t_parser->m_state == PARSER_AWAIT_SYNC) {
                if (t_parser->m_byte == t_parser->m_sync_seq[0]) {
                        t_parser->m_sync_seq_hits += 1;
                        if (t_parser->m_sync_seq_hits == t_parser->m_sync_seq_size) {
                                t_parser->m_state         = PARSING_HEADER_PAYLOAD_SIZE;
                                t_parser->m_sync_seq_hits = 0;
                        } else {
                                t_parser->m_state = PARSING_SYNC_SEQ;
                        }
                }
        } else if (t_parser->m_state == PARSING_SYNC_SEQ) {
                if (t_parser->m_byte == t_parser->m_sync_seq[t_parser->m_sync_seq_hits]) {
                        t_parser->m_sync_seq_hits += 1;
                        if (t_parser->m_sync_seq_hits == t_parser->m_sync_seq_size) {
                                t_parser->m_state         = PARSING_HEADER_PAYLOAD_SIZE;
                                t_parser->m_sync_seq_hits = 0;
                        }
                } else {
                        t_parser->m_state         = PARSER_AWAIT_SYNC;
                        t_parser->m_sync_seq_hits = 0;
                }
        } else if (t_parser->m_state == PARSING_HEADER_PAYLOAD_SIZE) {
                if (t_parser->m_flag == 0) {
                        if (t_parser->m_byte < 128) {
                                t_parser->m_packet_header[0]               = t_parser->m_byte;
                                t_parser->m_header_payload_size_field_size = 1;

                                t_parser->m_packet_payload_size = t_parser->m_packet_header[0];

                                t_parser->m_state = PARSING_HEADER_PAYLOAD_TYPE;
                        } else {
                                t_parser->m_packet_header[0] = t_parser->m_byte - 128;
                                t_parser->m_flag             = 1;
                        }
                } else {
                        t_parser->m_packet_header[1]               = t_parser->m_byte;
                        t_parser->m_header_payload_size_field_size = 2;
                        t_parser->m_flag                           = 0;

                        uint16_t payload_size           = 0x0;
                        uint16_t low_bits               = 0xFF & t_parser->m_packet_header[0];
                        uint16_t high_bits              = t_parser->m_packet_header[1];
                        uint16_t high_bits              = high_bits << 8;
                        payload_size                    = payload_size | low_bits | high_bits;
                        t_parser->m_packet_payload_size = payload_size;

                        t_parser->m_state = PARSING_HEADER_PAYLOAD_TYPE;
                }
        } else if (t_parser->m_state == PARSING_HEADER_PAYLOAD_TYPE) {
                if (t_parser->m_flag == 0) {
                        t_parser->m_header_payload_type_field_offset =
                            t_parser->m_header_payload_size_field_size;
                        if (t_parser->m_byte < 128) {
                                t_parser->m_packet_header
                                    [t_parser->m_header_payload_type_field_offset] =
                                    t_parser->m_byte;
                                t_parser->m_header_payload_type_field_size = 1;
                                t_parser->m_state = PARSING_HEADER_PAYLOAD_TYPE;
                        } else {
                                t_parser->m_packet_header
                                    [t_parser->m_header_payload_type_field_offset] =
                                    t_parser->m_byte - 128;
                                t_parser->m_flag = 1;
                        }
                } else {
                        t_parser
                            ->m_packet_header[t_parser->m_header_payload_type_field_offset + 1] =
                            t_parser->m_byte;
                        t_parser->m_header_payload_type_field_size = 2;
                        t_parser->m_flag                           = 0;
                        t_parser->m_state = PARSING_HEADER_PAYLOAD_CHECKSUM;
                }
        } else if (t_parser->m_state == PARSING_HEADER_PAYLOAD_CHECKSUM) {
                // FIXME: Добавить флаг наличия контрольной суммы полезных данных
                if ((t_parser->m_header_payload_size_field_size == 1 &&
                     t_parser->m_packet_header[0] != 0x0) ||
                    t_parser->m_header_payload_size_field_size == 2) {
                        t_parser->m_header_payload_checksum_field_offset =
                            t_parser->m_header_payload_type_field_offset +
                            t_parser->m_header_payload_type_field_size;

                        if (t_parser->m_header_payload_size_field_size == 1 &&
                            t_parser->m_packet_header[0] < 32) {
                                t_parser->m_packet_header
                                    [t_parser->m_header_payload_checksum_field_offset] =
                                    t_parser->m_byte;
                                t_parser->m_header_payload_checksum_field_size = 1;
                                t_parser->m_state = PARSING_HEADER_HEADER_CHECKSUM;
                        }

                        if ((t_parser->m_header_payload_size_field_size == 1 &&
                             t_parser->m_packet_header[0] >= 32) ||
                            t_parser->m_header_payload_size_field_size == 2) {
                                if (t_parser->m_flag) {
                                        t_parser->m_packet_header
                                            [t_parser->m_header_payload_checksum_field_offset + 1] =
                                            t_parser->m_byte;
                                        t_parser->m_flag                               = 0;
                                        t_parser->m_header_payload_checksum_field_size = 1;
                                        t_parser->m_state = PARSING_HEADER_HEADER_CHECKSUM;
                                } else {
                                        t_parser->m_packet_header
                                            [t_parser->m_header_payload_checksum_field_offset] =
                                            t_parser->m_byte;
                                        t_parser->m_flag = 1;
                                }
                        }
                }
        } else if (t_parser->m_state == PARSING_HEADER_HEADER_CHECKSUM) {
                if (t_parser->m_header_payload_checksum_field_offset == 0) {
                        // Если поле с контрольной суммой полезных данных отсутствует
                        t_parser->m_header_header_checksum_field_offset =
                            t_parser->m_header_payload_type_field_offset +
                            t_parser->m_header_payload_type_field_size;
                } else {
                        t_parser->m_header_header_checksum_field_offset =
                            t_parser->m_header_payload_checksum_field_offset +
                            t_parser->m_header_payload_checksum_field_size;
                }
                t_parser->m_packet_header[t_parser->m_header_header_checksum_field_offset] =
                    t_parser->m_byte;
                t_parser->m_state = PARSING_PAYLOAD;
                // FIXME: Добавить обновление размера заголовка пакета. В других секциях аналогично
                // нужно добавить.
        } else if (t_parser->m_state == PARSING_PAYLOAD) {
                if (t_parser->m_packet_payload_size != 0) {
                        t_parser->m_packet_payload[t_parser->m_packet_payload_bytes_parsed] =
                            t_parser->m_byte;
                        t_parser->m_packet_payload_bytes_parsed += 1;
                        if ((t_parser->m_packet_payload_bytes_parsed ==
                             t_parser->m_packet_payload_size) ||
                            (t_parser->m_packet_payload_bytes_parsed == MAX_PAYLOAD_SIZE)) {
                                t_parser->m_state = VERIFYING_CHECKSUM;
                        }
                } else {
                        t_parser->m_state = VERIFYING_CHECKSUM;
                }
        } else if (t_parser->m_state == VERIFYING_CHECKSUM) {
                uint8_t packet_header_payload_checksum_offset =
                    t_parser->m_header_payload_checksum_field_offset;
                if (t_parser->m_packet_payload_size < 32) {
                        uint8_t payload_checksum =
                            t_parser->m_packet_header[packet_header_payload_checksum_offset];
                        // Даже в случае, если размер полезных данных - 0, можем выполнить вызов
                        // функции рассчёта контрольной суммы, потому что обращение к массиву
                        // m_packet_payload не произойдёт.
                        uint8_t calculated_checksum =
                            crc8(t_parser->m_packet_payload, t_parser->m_packet_payload_size);

                } else {
                        uint8_t payload_checksum_low_bits =
                            t_parser->m_packet_header[packet_header_payload_checksum_offset];
                        uint8_t payload_checksum_high_bits =
                            t_parser->m_packet_header[packet_header_payload_checksum_offset + 1];
                        uint16_t packet_payload_checksum = payload_checksum_high_bits;
                        packet_payload_checksum          = packet_payload_checksum << 8;
                }

        } else if (t_parser->m_state == PARSING_COMPLETE) {
        } else if (t_parser->m_state == DISPATCH) {
        }

        return PARSER_OK;
}
