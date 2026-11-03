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

class IHSSPublicKey {
public:
    virtual ~IHSSPublicKey() = default;
    virtual size_t get_size_bytes() const = 0;
};

class IHSSPKEGenKey {
public:
    virtual ~IHSSPKEGenKey() = default;
    virtual size_t get_size_bytes() const = 0;
};

class IHSSHSSGenKey {
public:
    virtual ~IHSSHSSGenKey() = default;
    virtual size_t get_size_bytes() const = 0;
};

class IHSSEncryption {
public:
    virtual ~IHSSEncryption() = default;
    virtual size_t get_size_bytes() const = 0;
};

class IHSSOutputValue {
public:
    virtual ~IHSSOutputValue() = default;
    virtual size_t get_size_bytes() const = 0;
};

using MemoryHandle = std::shared_ptr<IHSSMemoryValue>;
using InputHandle = std::shared_ptr<IHSSInputValue>;
using EvalKeyHandle = std::shared_ptr<IHSSEvalKey>;
using ContextHandle = std::shared_ptr<IHSSContext>;
using PublicKeyHandle = std::shared_ptr<IHSSPublicKey>;
using PKEGenKeyHandle = std::shared_ptr<IHSSPKEGenKey>;
using HSSGenKeyHandle = std::shared_ptr<IHSSHSSGenKey>;
using EncryptionHandle = std::shared_ptr<IHSSEncryption>;
using OutputValueHandle = std::shared_ptr<IHSSOutputValue>;

#endif // TYPES_HPP
