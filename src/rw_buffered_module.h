#pragma once

#include "base_module.h"
#include "base_channel.h"

// RWBufferedModule: supports Read and/or write buffering via channels
template <typename ModuleTypeT>
class RWBufferedModule : public BaseModule<ModuleTypeT>
{
public:
    // Set either or both; pass nullptr for unused buffer
    RWBufferedModule(const ModuleId<ModuleTypeT> &id,
                     BaseChannel *inBuf = nullptr,
                     BaseChannel *outBuf = nullptr)
        : BaseModule<ModuleTypeT>(id),
          inBuf_(inBuf),
          outBuf_(outBuf)
    {
    }

    // Optionally take ownership of buffers, or manage externally
    ~RWBufferedModule() override
    {
        if (inBuf_)
            delete inBuf_;
        if (outBuf_ && outBuf_ != inBuf_)
            delete outBuf_; // Avoid double-delete if same pointer
    }

    // Set/replace input buffer at runtime (old one deleted)
    void setInputBuffer(BaseChannel *inBuf)
    {
        if (inBuf_ && inBuf_ != outBuf_)
            delete inBuf_;
        inBuf_ = inBuf;
    }

    // Set/replace output buffer at runtime (old one deleted)
    void setOutputBuffer(BaseChannel *outBuf)
    {
        if (outBuf_ && outBuf_ != inBuf_)
            delete outBuf_;
        outBuf_ = outBuf;
    }

    // For external sources to send data in
    bool write(const void *buf, size_t len, TickType_t ticksToWait = 0) override
    {
        return inBuf_ ? inBuf_->push(buf, len, ticksToWait) : false;
    }

    // For polling or streaming data out
    bool read(void *buf, size_t maxLen, TickType_t ticksToWait = 0) override
    {
        return outBuf_ ? outBuf_->pull(buf, maxLen, ticksToWait) != 0 : false;
    }

protected:
    // For use inside the module
    size_t recv(void *buf, size_t maxLen, TickType_t ticksToWait = portMAX_DELAY)
    {
        return inBuf_ ? inBuf_->pull(buf, maxLen, ticksToWait) : 0;
    }

    bool send(const void *buf, size_t len, TickType_t ticksToWait = 0)
    {
        return outBuf_ ? outBuf_->push(buf, len, ticksToWait) : false;
    }

    BaseChannel *inBuf_{nullptr};
    BaseChannel *outBuf_{nullptr};
};
