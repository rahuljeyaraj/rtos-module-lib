#pragma once
#include <cstddef>
#include "freertos/FreeRTOS.h"

class BaseChannel
{
public:
    virtual ~BaseChannel() = default;

    virtual bool push(const void *buf,
                      size_t len,
                      TickType_t timeout = 0) = 0;

    virtual size_t pull(void *buf,
                        size_t len,
                        TickType_t timeout = 0) = 0;
};