#ifndef TYPES_HPP
#define TYPES_HPP

#include <memory>
#include <cstddef>

// Universal opaque base for any scheme's Memory Value
class IHSSMemoryValue {
public:
    virtual ~IHSSMemoryValue() = default;
    virtual size_t get_size_bytes() const = 0;
};

// Universal opaque base for any scheme's Input Value
class IHSSInputValue {
public:
    virtual ~IHSSInputValue() = default;
    virtual size_t get_size_bytes() const = 0;
};

class IHSSEvalKey {
public:
    virtual ~IHSSEvalKey() = default;
    virtual size_t get_size_bytes() const = 0;
};

class IHSSContext {
public:
    virtual ~IHSSContext() = default;
    virtual size_t get_size_bytes() const = 0;
};

using MemoryHandle = std::shared_ptr<IHSSMemoryValue>;
using InputHandle = std::shared_ptr<IHSSInputValue>;
using EvalKeyHandle = std::shared_ptr<IHSSEvalKey>;
using ContextHandle = std::shared_ptr<IHSSContext>;

#endif // TYPES_HPP
