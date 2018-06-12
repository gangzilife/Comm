#include "ringbuffer.h"


//static uint32_t RingBuffer_free_space(RingBuffer_t* ringbuffer)
//{
//    if(ringbuffer->tail == ringbuffer->head)
//        return ringbuffer->length - 1;
//    uint32_t tail_to_end = ringbuffer->length - 1 - ringbuffer->tail;
//    return ((tail_to_end + ringbuffer->head)%ringbuffer->length);
//}
//
//static uint32_t RingBuffer_used_space(RingBuffer_t* ringbuffer)
//{
//    if((ringbuffer->tail + 1 ) % ringbuffer->length == ringbuffer->head)
//        return ringbuffer->length;
//    uint32_t head_to_end = ringbuffer->length - ringbuffer->head;
//    return ((head_to_end + ringbuffer->tail)%ringbuffer->length);
//}


void RingBuffer_Init(RingBuffer_t* ringbuffer,uint8_t* buffer,uint32_t size)
{
    ringbuffer->length = size;
    ringbuffer->buffer = buffer;
    ringbuffer->head = 15;
    ringbuffer->tail = 20;
}



//uint32_t RingBuffer_write(RingBuffer_t* ringbuffer,const uint8_t *data,uint32_t length)
//{
//    uint32_t count;
//    uint32_t tail_to_end = ringbuffer->length - ringbuffer->tail ;
//    count = RingBuffer_free_space(ringbuffer);
//    if(count == 0)
//        return 0;
//    
//    count = MIN(count,length);
//    if(count <= tail_to_end)  //不会跨过头尾节点
//    {
//        memcpy(RingBuffer_ends_at(ringbuffer),data,count);
//        ringbuffer->tail = (ringbuffer->tail + count)%ringbuffer->length;
//    }
//    else
//    {   //跨过头尾节点
//        memcpy(RingBuffer_ends_at(ringbuffer),data,tail_to_end);
//        ringbuffer->tail = (ringbuffer->tail + tail_to_end)%ringbuffer->length;
//        memcpy(RingBuffer_ends_at(ringbuffer),data + tail_to_end , count - tail_to_end);
//        ringbuffer->tail = (ringbuffer->tail + count - tail_to_end)%ringbuffer->length;
//    }
//    return count;    
//}
//
//uint32_t RingBuffer_read(RingBuffer_t* ringbuffer,uint8_t *indata,uint32_t amount)
//{
//    uint32_t count;
//    uint32_t head_to_end = ringbuffer->length - ringbuffer->head;
//    count = RingBuffer_used_space(ringbuffer);
//    count = MIN(count,amount);
//    if(count == 0)
//        return 0;
//    if(count <= head_to_end)
//    {
//        memcpy(indata,RingBuffer_starts_at(ringbuffer),count);
//        ringbuffer->head = (ringbuffer->head + count)%ringbuffer->length;
//    }
//    else
//    {
//        memcpy(indata,RingBuffer_starts_at(ringbuffer),head_to_end);
//        ringbuffer->head = (ringbuffer->head + head_to_end)%ringbuffer->length;
//        memcpy(indata + head_to_end,RingBuffer_starts_at(ringbuffer),count - head_to_end);
//        ringbuffer->head = (ringbuffer->head + count - head_to_end)%ringbuffer->length;
//    }
//    return count;
//}




uint32_t RingBuffer_write(RingBuffer_t* ringbuffer,const uint8_t *data,uint32_t length)
{
    if (RingBuffer_available_data(ringbuffer) == 0) 
    {
          ringbuffer->head = ringbuffer->tail = 0;
    }
    
    uint32_t count = RingBuffer_available_space(ringbuffer);
    if( count < length)
        return 0;
           
    memcpy(RingBuffer_ends_at(ringbuffer), data, length);
    RingBuffer_commit_write(ringbuffer, length);
    return length;
}

uint32_t RingBuffer_read(RingBuffer_t* ringbuffer,uint8_t *indata,uint32_t amount)
{
    uint32_t count;
    count = RingBuffer_available_data(ringbuffer);
    if( count == 0)
        return 0;
    count = MIN(count,amount);
    memcpy(indata, RingBuffer_starts_at(ringbuffer), count);
    RingBuffer_commit_read(ringbuffer, count);
    if (ringbuffer->tail == ringbuffer->head) 
        ringbuffer->head = ringbuffer->tail = 0;

    return count;
}