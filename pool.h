#pragma once

#include <vector>

template<typename T>
class Pool final {
    struct PoolFreed {
        PoolFreed* nextFree;
    };

    PoolFreed* freed = nullptr;
    int block = -1;
    unsigned blockSize;
    unsigned used;
    std::vector<T*> blocks;

public:
    /**
     * @brief Construct a memory pool
     * @param blockSize The number of elements in a pool block
     */
    Pool(unsigned blockSize) : blockSize(blockSize), used(blockSize - 1) {}
    ~Pool() {
        for (unsigned i = 0, blockCount = (unsigned)blocks.size(); i < blockCount; ++i)
            std::free(blocks[i]);
    }

    /**
     * @brief Allocate an instance from this pool
     * @return The allocated object, or nullptr if the pool is full
     */
    template <typename... Args>
    T* allocate(Args&&... args) {
        if (freed) {
            const auto recycle = freed;

            freed = freed->nextFree;

            new (recycle) T(std::forward<Args>(args)...);

            return (T*)recycle;
        }

        if (++used == blockSize) {
            used = 0;

            if (++block == (unsigned)blocks.size())
                blocks.push_back((T*)malloc(sizeof(T) * blockSize));
        }

        new (blocks[block] + used) T(std::forward<Args>(args)...);

        return blocks[block] + used;
    }

    /**
     * @brief Free a previously allocated instance of this pool
     * @param instance The instance to free
     */
    void free(T* instance) {
        const auto nextFree = freed;

        instance->~T();

        freed = (PoolFreed*)instance;
        freed->nextFree = nextFree;
    }
};