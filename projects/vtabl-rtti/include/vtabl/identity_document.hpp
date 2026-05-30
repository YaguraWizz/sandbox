#pragma once

#include <iostream>
#include <string>
#include <string_view>

using namespace std::string_view_literals;

#define OVERIDE_FUNC_VTABLE(name, return_type, atribute, ...)  \
    public:                                                          \
    return_type name() atribute {                                    \
        if constexpr (std::is_void_v<return_type>) {                 \
            __VA_ARGS__;                                             \
        } else {                                                     \
            return __VA_ARGS__;                                      \
        }                                                            \
    }


struct IdentityDocument;
struct IdentityDocumentVTable {
    void (*PrintID)(const IdentityDocument*);
    void (*Delete)(IdentityDocument*);
    IdentityDocument* (*Copy)(const IdentityDocument*);

    template<typename TVTable, typename Concrete>
    void verify(const TVTable* _vtable, const Concrete* object_ptr) const {
        // Сравниваем адрес vtable и адрес самого объекта
        if (reinterpret_cast<const void*>(_vtable) != reinterpret_cast<const void*>(object_ptr)) {
            std::cout << "[verify] Address of vtable pointer: " << static_cast<const void*>(_vtable) << '\n';
            std::cout << "[verify] Address of object pointer: " << static_cast<const void*>(object_ptr) << '\n';
            std::cout << "[verify] Are addresses the same? "
                << ((reinterpret_cast<const void*>(_vtable) == reinterpret_cast<const void*>(object_ptr)) ? "Yes" : "No")
                << std::endl;
            throw std::runtime_error("_vtable must be the first field in object");
        }
    }
};

struct IdentityDocument {
private:
    static IdentityDocumentVTable _default;
    const IdentityDocumentVTable* _vtable = nullptr;
    static void BasePrint(const IdentityDocument* self) {
        std::cout << "IdentityDocument::PrintID() : "sv << self->unique_id_ << std::endl;
    }
    static void BaseDelete(IdentityDocument* self) {
        delete self;
    }
    static IdentityDocument* BaseCopy(const IdentityDocument* self) {
        return new IdentityDocument(*self);
    }
public:
    IdentityDocument()
        : _vtable(&_default), unique_id_(++unique_id_count_) {
        _vtable->verify(&_vtable, this);
        std::cout << "IdentityDocument::Ctor() : "sv << unique_id_ << std::endl;
    }
    IdentityDocument(const IdentityDocument& other)
        : _vtable(other._vtable), unique_id_(++unique_id_count_) {
        std::cout << "IdentityDocument::CCtor() : "sv << unique_id_ << std::endl;
    }
    ~IdentityDocument() {
        --unique_id_count_;
        std::cout << "IdentityDocument::Dtor() : "sv << unique_id_ << std::endl;
    }

    OVERIDE_FUNC_VTABLE(Delete, void, noexcept, _vtable->Delete(this))
    OVERIDE_FUNC_VTABLE(PrintID, void, const, _vtable->PrintID(this))
    OVERIDE_FUNC_VTABLE(Copy, IdentityDocument*, const, _vtable->Copy(this))

    int GetID() const noexcept { return unique_id_; }
    static void PrintUniqueIDCount() noexcept { std::cout << "unique_id_count_ : "sv << unique_id_count_ << std::endl; }

    const IdentityDocumentVTable* _VTable() {
        return _vtable;
    }
    void _VTable(const IdentityDocumentVTable* vtable) {
        _vtable = vtable;
    }
    void Reset() {
        _vtable = &_default;
    }
private:
    static int unique_id_count_;
    int unique_id_;
};
inline IdentityDocumentVTable IdentityDocument::_default = {
    &IdentityDocument::BasePrint,
    &IdentityDocument::BaseDelete,
    &IdentityDocument::BaseCopy
};
inline int IdentityDocument::unique_id_count_ = 0;