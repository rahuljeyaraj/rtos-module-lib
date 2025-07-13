#pragma once

#include <cstddef>
#include "freertos/FreeRTOS.h"
#include "freertos/message_buffer.h"
#include "base_channel.h"
#include <freertos/semphr.h>
#include <algorithm>
#include <etl/algorithm.h>

class MBufChannel : public BaseChannel
{
public:
    // Dynamic allocation constructor
    explicit MBufChannel(
        size_t channelSizeBytes,
        size_t maxMsgSizeBytes = SIZE_MAX, // default: no max limit
        size_t minMsgSizeBytes = 1,        // default min message length
        bool threadSafeWrite = false,
        bool threadSafeRead = false)
        : mbuf_(xMessageBufferCreate(channelSizeBytes)),
          ownBuf_(true),
          maxMsgSizeBytes_(maxMsgSizeBytes),
          minMsgSizeBytes_(etl::max<size_t>(1, minMsgSizeBytes))

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

    // Issue: May get blocked up to 2 * ticksToWait (mutex wait + buffer wait)
    bool push(const void *buf, size_t len, TickType_t ticksToWait = 0) override
    {
        if (len < minMsgSizeBytes_ || len > maxMsgSizeBytes_)
            return false; // reject message size out of bounds
        LockGuard g(mtxWrite_, ticksToWait);
        if (!g.acquired())
            return false;
        return xMessageBufferSend(mbuf_, buf, len, ticksToWait) == len;
    }

    // Issue: May get blocked up to 2 * ticksToWait (mutex wait + buffer wait)
    size_t pull(void *buf, size_t maxLen, TickType_t ticksToWait = 0) override
    {
        LockGuard g(mtxRead_, ticksToWait);
        if (!g.acquired())
            return false;
        size_t len = xMessageBufferReceive(mbuf_, buf, maxLen, ticksToWait);
        if (len < minMsgSizeBytes_ || len > maxMsgSizeBytes_)
            return 0; // Drop invalid message (message is removed from buffer)
        return len;
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

    // TODO: see if maxMsgSizeBytes_ and minMsgSizeBytes_ need to moved to BaseChannel class
    size_t maxMsgSizeBytes_{SIZE_MAX};
    size_t minMsgSizeBytes_{1};
};
