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
  * @file ring_buffer.h
  * @brief Кольцевой буфер
  * @version 1.0
  * @authors Zaikin Denis (ZD)
  * @authors Potorochin Andrew (PA)
  * 
  * @copyright GNU General Public License
  *********************************************************************************************************
  */
#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

/// Тип описывающий кольцовой буфер
typedef struct
{
        uint8_t *buffer;    ///< Указатель на буфер-хранилище
        uint16_t idxIn;     ///< Номер ячейки для записи
        uint16_t idxOut;    ///< Номер ячейки для чтения
        uint16_t size;      ///< Размер Буфера-хранилища
} RingBuffer_t;

/// Возможные состояния выполнения функции инициализации
typedef enum
{
        RING_ERROR   = 0,             ///< Значение если произошла ошибка
        RING_SUCCESS = !RING_ERROR    ///< Значение если всё прошло удачно
} RING_ErrorStatus_t;

/**
@function RING_CRC16ccitt − Вычисление контрольной суммы.
@param RingBuffer_t *buf − Указатель на кольцевой буфер.
@param uint16_t lenght − Сколько элементов буфера участвуют в подсчете.
@param uint16_t position − С какого элемента начинать.
@return uint16_t Результат вычисления контрольной суммы.
 */
uint16_t
RING_CRC16ccitt(const RingBuffer_t *buf, uint16_t lenght, uint16_t position);

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
                             uint16_t position);

/**
@function RING_Init − Инициализация буфера.
@param RingBuffer_t *ring − Указатель на кольцевой буфер.
@param uint8_t *buf − Указатель на буфер хранения.
@param uint16_t size − Сколько элементов в буфере.
@return RING_ErrorStatus_t Результат инициализации @ref RING_ErrorStatus_t
 */
RING_ErrorStatus_t
RING_Init(RingBuffer_t *ring, uint8_t *buf, uint16_t size);

/**
@function RING_GetCount − Количество полезных данных в буфере.
@param RingBuffer_t *buf − Указатель на кольцевой буфер.
@return uint16_t Количество полезных данных в буфере.
 */
uint16_t
RING_GetCount(const RingBuffer_t *buf);

/**
@function RING_Clear − Очищает буфер.
@param RingBuffer_t *buf − Указатель на кольцевой буфер.
 */
void
RING_Clear(RingBuffer_t *buf);

/**
@function RING_Put − Загружает элемент в буфер.
@param uint8_t symbol − Элемент для загрузки в буфер.
@param RingBuffer_t *buf − Указатель на кольцевой буфер.
 */
void
RING_Put(RingBuffer_t *buf, uint8_t symbol);

/**
@function RING_Put16 − Загружает элемент в буфер.
@param uint16_t symbol − Элемент для загрузки в буфер.
@param RingBuffer_t *buf − Указатель на кольцевой буфер.
 */
void
RING_Put16(RingBuffer_t *buf, uint16_t symbol);

/**
@function RING_PutBuffr − Загружает элементы из массива-источника в кольцевой буфер.
@param RingBuffer_t *ringbuf − Указатель на кольцевой буфер.
@param uint8_t *src − Указатель на массив-источник.
@param uint16_t len − количество загружаемых элементов.
 */
void
RING_PutBuffr(RingBuffer_t *ringbuf, uint8_t *src, uint16_t len);

/**
@function RING_Pop − Получает из буфера байт.
@param RingBuffer_t *buf − Указатель на кольцевой буфер.
@return uint8_t Значение полученого элемента.
 */
uint8_t
RING_Pop(RingBuffer_t *buf);

/**
@function RING_Pop16 − Получает из буфера 16-битное число.
@param RingBuffer_t *buf − Указатель на кольцевой буфер.
@return uint16_t Значение полученого элемента.
 */
uint16_t
RING_Pop16(RingBuffer_t *buf);

/**
@function RING_Pop32 − Получает из буфера 32-битное число.
@param RingBuffer_t *buf − Указатель на кольцевой буфер.
@return uint32_t Значение полученого элемента.
 */
uint32_t
RING_Pop32(RingBuffer_t *buf);

/**
@function RING_PopBuffr − Заполняет элементами кольцевого буфера массив.
@param RingBuffer_t *ringbuf − Указатель на кольцевой буфер.
@param uint8_t *destination − Указатель на массив.
@param uint16_t len − количество получаемых элементов.
 */
void
RING_PopBuffr(RingBuffer_t *ringbuf, uint8_t *destination, uint16_t len);


/**
@function RING_PopString − Считывает нультерминальную строку из буфера в string
@param RingBuffer_t *ringbuf − Указатель на кольцевой буфер.
@param char *string − Указатель на массив.
 */
void
RING_PopString(RingBuffer_t *ringbuf, char *string);

/**
@function RING_ShowSymbol − Показывает содержимое элемента без его удаления из буфера.
@param RingBuffer_t *ringbuf − Указатель на кольцевой буфер.
@param uint16_t symbolNumbe − Номер элемента.
@return int32_t Значение полученого элемента. -1 если ошибка.
 */
int32_t
RING_ShowSymbol(const RingBuffer_t *buf, uint16_t symbolNumber);


#ifdef __cplusplus
}
#endif

#endif /* RING_BUFFER_H */
