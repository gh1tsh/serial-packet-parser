/*
* This is an independent project of an individual developer. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
/* LINTLIBRARY */
/**
  *********************************************************************************************************
  *
  *	This file is part of Devprodest Lib.
  *	
  *	  Devprodest Lib is free software: you can redistribute it and/or modify
  *	  it under the terms of the GNU General Public License as published by
  *	  the Free Software Foundation, either version 3 of the License, or
  *	  (at your option) any later version.
  *	
  *	  Devprodest Lib is distributed in the hope that it will be useful,
  *	  but WITHOUT ANY WARRANTY; without even the implied warranty of
  *	  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  *	  GNU General Public License for more details.
  *	
  *	  You should have received a copy of the GNU General Public License
  *	  along with Devprodest Lib.  If not, see <http://www.gnu.org/licenses/>.
  *	
  * @file ring_buffer.c
  * @brief Кольцевой буфер
  * @version 1.0
  * @authors Zaikin Denis (ZD)
  * @authors Potorochin Andrew (PA)
  * 
  * @copyright GNU General Public License
  *********************************************************************************************************
  */
#include "ring_buffer.h"
#include "stdint.h"

/** 
 * @defgroup CRC Функции вычисления циклической контрольной суммы
 * @{
 */

/**
@function RING_CRC16ccitt − Вычисление контрольной суммы.
@param RingBuffer_t *buf − Указатель на кольцевой буфер.
@param uint16_t lenght − Сколько элементов буфера участвуют в подсчете.
@param uint16_t position − С какого элемента начинать.
@return uint16_t Результат вычисления контрольной суммы.
 */
uint16_t
RING_CRC16ccitt(const RingBuffer_t *buf, uint16_t lenght, uint16_t position)
{
        return RING_CRC16ccitt_Intermediate(buf, lenght, 0xFFFF, position);
}

/**
@function RING_CRC16ccitt_Intermediate − Вычисление контрольной суммы с инициализацией.
@param RingBuffer_t *buf − Указатель на кольцевой буфер.
@param uint16_t lenght − Сколько элементов буфера участвуют в подсчете.
@param uint16_t tmpCrc − Начальное значение контрольной суммы.
@param uint16_t position − С какого элемента начинать.
@return uint16_t Результат вычисления контрольной суммы.
 */
uint16_t
RING_CRC16ccitt_Intermediate(const RingBuffer_t *buf, uint16_t lenght, uint16_t tmpCrc,
                             uint16_t position)
{
        uint16_t crc = tmpCrc;
        uint16_t crctab;
        uint8_t  byte;

        while (lenght--) {
                crctab = 0x0000;
                byte   = (RING_ShowSymbol(buf, lenght + position)) ^ (crc >> 8);
                if (byte & 0x01)
                        crctab ^= 0x1021;
                if (byte & 0x02)
                        crctab ^= 0x2042;
                if (byte & 0x04)
                        crctab ^= 0x4084;
                if (byte & 0x08)
                        crctab ^= 0x8108;
                if (byte & 0x10)
                        crctab ^= 0x1231;
                if (byte & 0x20)
                        crctab ^= 0x2462;
                if (byte & 0x40)
                        crctab ^= 0x48C4;
                if (byte & 0x80)
                        crctab ^= 0x9188;

                crc = (((crc & 0xFF) ^ (crctab >> 8)) << 8) | (crctab & 0xFF);
        }
        return crc;
}

/** @} */

/** 
 * @defgroup control Функции управления буфером
 * @{
 */


/**
@function RING_Init − Инициализация буфера.
@param RingBuffer_t *ring − Указатель на кольцевой буфер.
@param uint8_t *buf − Указатель на буфер хранения.
@param uint16_t size − Сколько элементов в буфере.
@return RING_ErrorStatus_t Результат инициализации @ref RING_ErrorStatus_t
 */
RING_ErrorStatus_t
RING_Init(RingBuffer_t *ring, uint8_t *buf, uint16_t size)
{
        ring->size   = size;
        ring->buffer = buf;
        RING_Clear(ring);

        return (ring->buffer ? RING_SUCCESS : RING_ERROR);
}

/**
@function RING_GetCount − Количество полезных данных в буфере.
@param RingBuffer_t *buf − Указатель на кольцевой буфер.
@return uint16_t Количество полезных данных в буфере.
 */
uint16_t
RING_GetCount(const RingBuffer_t *buf)
{
        uint16_t retval = 0;
        if (buf->idxIn < buf->idxOut)
                retval = buf->size + buf->idxIn - buf->idxOut;
        else
                retval = buf->idxIn - buf->idxOut;
        return retval;
}

/**
@function RING_Clear − Очищает буфер.
@param RingBuffer_t *buf − Указатель на кольцевой буфер.
 */
void
RING_Clear(RingBuffer_t *buf)
{
        buf->idxIn  = 0;
        buf->idxOut = 0;
}

/** @} */

/** 
 * @defgroup put Функции загрузки данных в буфер
 * @{
 */

/**
@function RING_Put − Загружает элемент в буфер.
@param uint8_t symbol − Элемент для загрузки в буфер.
@param RingBuffer_t *buf − Указатель на кольцевой буфер.
 */
void
RING_Put(RingBuffer_t *buf, uint8_t symbol)
{
        buf->buffer[buf->idxIn++] = symbol;
        if (buf->idxIn >= buf->size)
                buf->idxIn = 0;
}

/**
@function RING_Put16 − Загружает элемент в буфер.
@param uint16_t symbol − Элемент для загрузки в буфер.
@param RingBuffer_t *buf − Указатель на кольцевой буфер.
 */
void
RING_Put16(RingBuffer_t *buf, uint16_t symbol)
{
        buf->buffer[buf->idxIn++] = symbol >> 8;
        if (buf->idxIn >= buf->size)
                buf->idxIn = 0;
        buf->buffer[buf->idxIn++] = symbol & 0xFF;
        if (buf->idxIn >= buf->size)
                buf->idxIn = 0;
}

/**
@function RING_PutBuffr − Загружает элементы из массива-источника в кольцевой буфер.
@param RingBuffer_t *ringbuf − Указатель на кольцевой буфер.
@param uint8_t *src − Указатель на массив-источник.
@param uint16_t len − количество загружаемых элементов.
 */
void
RING_PutBuffr(RingBuffer_t *ringbuf, uint8_t *src, uint16_t len)
{
        while (len--)
                RING_Put(ringbuf, *(src++));
}

/** @} */

/** 
 * @defgroup pop Функции выгрузки данных из буфера
 * @{
 */

/**
@function RING_Pop − Получает из буфера байт.
@param RingBuffer_t *buf − Указатель на кольцевой буфер.
@return uint8_t Значение полученого элемента.
 */
uint8_t
RING_Pop(RingBuffer_t *buf)
{
        uint8_t retval = buf->buffer[buf->idxOut++];
        if (buf->idxOut >= buf->size)
                buf->idxOut = 0;
        return retval;
}

/**
@function RING_Pop16 − Получает из буфера 16-битное число.
@param RingBuffer_t *buf − Указатель на кольцевой буфер.
@return uint16_t Значение полученого элемента.
 */
uint16_t
RING_Pop16(RingBuffer_t *buf)
{
        uint16_t retval = RING_Pop(buf) << 8;
        retval += RING_Pop(buf);
        return retval;
}

/**
@function RING_Pop32 − Получает из буфера 32-битное число.
@param RingBuffer_t *buf − Указатель на кольцевой буфер.
@return uint32_t Значение полученого элемента.
 */
uint32_t
RING_Pop32(RingBuffer_t *buf)
{
        uint32_t retval = RING_Pop(buf) << 8;
        retval += RING_Pop(buf) << 8;
        retval += RING_Pop(buf) << 8;
        retval += RING_Pop(buf);
        return retval;
}

/**
@function RING_PopBuffr − Заполняет элементами кольцевого буфера массив.
@param RingBuffer_t *ringbuf − Указатель на кольцевой буфер.
@param uint8_t *destination − Указатель на массив.
@param uint16_t len − количество получаемых элементов.
 */
void
RING_PopBuffr(RingBuffer_t *ringbuf, uint8_t *destination, uint16_t len)
{
        while (len--)
                *(destination++) = RING_Pop(ringbuf);
}

/**
@function RING_PopString − Считывает нультерминальную строку из буфера в string
@param RingBuffer_t *ringbuf − Указатель на кольцевой буфер.
@param char *string − Указатель на массив.
 */
void
RING_PopString(RingBuffer_t *ringbuf, char *string)
{
        while (RING_ShowSymbol(ringbuf, 0) > 0)
                *(string++) = RING_Pop(ringbuf);
}

/** @} */

/** 
 * @defgroup other Другие функции
 * @ingroup pop
 * @{
 */

/**
@function RING_ShowSymbol − Показывает содержимое элемента без его удаления из буфера.
@param RingBuffer_t *ringbuf − Указатель на кольцевой буфер.
@param uint16_t symbolNumbe − Номер элемента.
@return int32_t Значение полученого элемента. -1 если ошибка.
 */
int32_t
RING_ShowSymbol(const RingBuffer_t *buf, uint16_t symbolNumber)
{
        uint32_t pointer = buf->idxOut + symbolNumber;
        int32_t  retval  = -1;
        if (symbolNumber < RING_GetCount(buf)) {
                if (pointer > buf->size)
                        pointer -= buf->size;
                retval = buf->buffer[pointer];
        }
        return retval;
}

/** @} */
