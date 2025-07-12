#pragma once

#include <cstddef>
#include "freertos/FreeRTOS.h"
#include "freertos/message_buffer.h"
#include "base_channel.h"
#include <freertos/semphr.h>

class MBufChannel : public BaseChannel
{
public:
    // Dynamic allocation constructor
    explicit MBufChannel(
        size_t sizeBytes,
        bool threadSafeWrite = false,
        bool threadSafeRead = false)
        : mbuf_(xMessageBufferCreate(sizeBytes)), ownBuf_(true)
    {
        setupMutexes(threadSafeWrite, threadSafeRead);
    }

    // Static allocation constructor
    MBufChannel(
        uint8_t *storage,
        size_t sizeBytes,
        StaticMessageBuffer_t *control,
        bool threadSafeWrite = false,
        bool threadSafeRead = false)
        : mbuf_(xMessageBufferCreateStatic(sizeBytes, storage, control)), ownBuf_(false)
    {
        setupMutexes(threadSafeWrite, threadSafeRead);
    }

    ~MBufChannel() override
    {
        if (ownBuf_ && mbuf_)
            vMessageBufferDelete(mbuf_);
        if (mtxWrite_)
            vSemaphoreDelete(mtxWrite_);
        if (mtxRead_)
            vSemaphoreDelete(mtxRead_);
    }

    bool push(const void *buf, size_t len, TickType_t ticksToWait = 0) override
    {
        // May get blocked up to 2 * ticksToWait (mutex wait + buffer wait)
        LockGuard g(mtxWrite_, ticksToWait);
        if (!g.acquired())
            return false;
        return xMessageBufferSend(mbuf_, buf, len, ticksToWait) == len;
    }

    size_t pull(void *buf, size_t maxLen, TickType_t ticksToWait = 0) override
    {
        // May get blocked up to 2 * ticksToWait (mutex wait + buffer wait)
        LockGuard g(mtxRead_, ticksToWait);
        if (!g.acquired())
            return false;
        return xMessageBufferReceive(mbuf_, buf, maxLen, ticksToWait);
    }

    MessageBufferHandle_t handle() const { return mbuf_; }

private:
    void setupMutexes(bool threadSafeWrite, bool threadSafeRead)
    {
        if (threadSafeWrite)
            mtxWrite_ = xSemaphoreCreateMutex();
        if (threadSafeRead)
            mtxRead_ = xSemaphoreCreateMutex();
    }

    struct LockGuard
    {
        SemaphoreHandle_t m;
        BaseType_t taken;
        LockGuard(SemaphoreHandle_t h, TickType_t ticksToWait = portMAX_DELAY) : m(h)
        {
            taken = m ? xSemaphoreTake(m, ticksToWait) : pdTRUE;
        }
        ~LockGuard()
        {
            if (m && taken == pdTRUE)
                xSemaphoreGive(m);
        }
        bool acquired() const { return taken == pdTRUE; }
    };

    MessageBufferHandle_t mbuf_;
    bool ownBuf_;
    SemaphoreHandle_t mtxWrite_{nullptr};
    SemaphoreHandle_t mtxRead_{nullptr};
};
