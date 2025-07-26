#pragma once
#include "base_channel.h"
#include "freertos/queue.h"

class QueueChannel : public BaseChannel {
   public:
    QueueChannel(size_t itemSize, size_t length)
        : q_(xQueueCreate(length, itemSize)), itemSize_(itemSize) {}

    ~QueueChannel() override {
        vQueueDelete(q_);
    }

    bool push(const void *buf, size_t len, TickType_t to = 0) override {
        // Only accept correct size
        if (len != itemSize_) return false;
        return xQueueSend(q_, buf, to) == pdPASS;
    }

    size_t pull(void *buf, size_t len, TickType_t to = 0) override {
        if (len < itemSize_) return 0;
        return xQueueReceive(q_, buf, to) == pdPASS ? itemSize_ : 0;
    }

   private:
    QueueHandle_t q_;
    size_t itemSize_;
};
