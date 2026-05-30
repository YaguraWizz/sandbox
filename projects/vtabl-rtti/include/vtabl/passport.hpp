#pragma once
#include "vtabl/identity_document.hpp"
#include <iostream>
#include <string>
#include <ctime>

using namespace std::string_view_literals;

struct PassportVTable {
    void (*PrintVisa)(const IdentityDocument* base, const std::string& country);
};
struct Passport {
private:
    static PassportVTable _default;
    static IdentityDocumentVTable base_default;
    static void BasePrint(const IdentityDocument* base) {
        auto* self = reinterpret_cast<const Passport*>(base);
        std::cout << "Passport::PrintID() : "sv << self->GetID();
        std::cout << " expiration date : "sv << self->expiration_date_.tm_mday
            << "/"sv << self->expiration_date_.tm_mon << "/"sv
            << self->expiration_date_.tm_year + 1900 << std::endl;
    }
    static void BaseDelete(IdentityDocument* self) {
        delete reinterpret_cast<Passport*>(self);
    }
    static IdentityDocument* BaseCopy(const IdentityDocument* base) {
        auto* self = reinterpret_cast<const Passport*>(base);
        auto* copy = new Passport(*self);
        return &copy->_base;
    }
    static void PrintVisa(const IdentityDocument* base, const std::string& country) {
        std::cout << "Passport::PrintVisa("sv << country << ") : "sv << base->GetID() << std::endl;
    }
public:
    IdentityDocument _base;

    Passport() : _vtable(&_default), expiration_date_(GetExpirationDate()) {
        _base._VTable()->verify(&_base, this);
        _base._VTable(&base_default);
        std::cout << "Passport::Ctor()"sv << std::endl;
    }
    Passport(const Passport& other)
        : _base(other._base), _vtable(other._vtable),
        expiration_date_(other.expiration_date_) {
        std::cout << "Passport::CCtor()"sv << std::endl;
    }
    ~Passport() {
        std::cout << "Passport::Dtor()"sv << std::endl;
    }

    OVERIDE_FUNC_VTABLE(Delete, void, noexcept, _base.Delete())
        OVERIDE_FUNC_VTABLE(PrintID, void, const, _base.PrintID())
        OVERIDE_FUNC_VTABLE(Copy, IdentityDocument*, const, _base.Copy())
        OVERIDE_FUNC_VTABLE(GetID, int, const, _base.GetID())
        OVERIDE_FUNC_VTABLE(PrintUniqueIDCount, void, const, _base.PrintUniqueIDCount())

        void PrintVisa(const std::string& country) const {
        _vtable->PrintVisa(&_base, country);
    }

    operator IdentityDocument () {
        _base.Reset();
        return _base;
    }
    operator IdentityDocument* () {
        return &_base;
    }
private:
    const PassportVTable* _vtable = nullptr;
    const struct tm expiration_date_;
    tm GetExpirationDate() {
        time_t t = time(nullptr);
        tm exp_date;
#ifdef _MSC_VER  // Для MSVC компилятора
        localtime_s(&exp_date, &t);
#else  // Для других компиляторов
        localtime_r(&t, &exp_date);
#endif
        exp_date.tm_year += 10;
        mktime(&exp_date);
        return exp_date;
    }
};
inline IdentityDocumentVTable Passport::base_default = {
    &Passport::BasePrint,
    &Passport::BaseDelete,
    &Passport::BaseCopy
};
inline PassportVTable Passport::_default = {
    &Passport::PrintVisa,
};