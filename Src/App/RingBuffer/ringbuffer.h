#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__

#include "stdint.h"
#include "stdlib.h"
#include "string.h"

#define MIN(x,y)  ((x) < (y) ? (x) : (y))


__packed typedef struct
{
    uint8_t* buffer;
    uint32_t length;
    uint32_t head;
    uint32_t tail;
} RingBuffer_t;

void RingBuffer_Init(RingBuffer_t* ringbuffer,uint8_t* buffer,uint32_t size);


uint32_t RingBuffer_read(RingBuffer_t* ringbuffer, uint8_t *indata,uint32_t amount);
uint32_t RingBuffer_write(RingBuffer_t* ringbuffer,const uint8_t *data,uint32_t length);

//uint32_t RingBuffer_free_space(RingBuffer_t* ringbuffer);
//uint32_t RingBuffer_used_space(RingBuffer_t* ringbuffer);

#define RingBuffer_starts_at(B)  ((B)->buffer + (B)->head)
#define RingBuffer_ends_at(B)    ((B)->buffer + (B)->tail)

#define RingBuffer_available_data(B)  (((B)->tail + 1) % (B)->length - (B)->head -1)
#define RingBuffer_available_space(B) ((B)->length - (B)->tail  -1)

#define RingBuffer_commit_read(B, A) ((B)->head = ((B)->head + (A)) % (B)->length)
#define RingBuffer_commit_write(B, A) ((B)->tail = ((B)->tail + (A)) % (B)->length)




#endif