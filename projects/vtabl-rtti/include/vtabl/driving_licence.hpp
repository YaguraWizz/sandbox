#pragma once

#include "vtabl/identity_document.hpp"
#include <iostream>
#include <string_view>

using namespace std::string_view_literals;

struct DrivingLicence {
private:
    static IdentityDocumentVTable _default;
    static void BasePrint(const IdentityDocument* self) {
        std::cout << "DrivingLicence::PrintID() : "sv << self->GetID() << std::endl;
    }
    static void BaseDelete(IdentityDocument* self) {
        delete reinterpret_cast<DrivingLicence*>(self);
    }
    static IdentityDocument* BaseCopy(const IdentityDocument* base) {
        auto* self = reinterpret_cast<const DrivingLicence*>(base);
        auto* copy = new DrivingLicence(*self);
        return &copy->_base;
    }
public:
    IdentityDocument _base;

    DrivingLicence() {
        _base._VTable()->verify(&_base, this);
        _base._VTable(&_default);
        std::cout << "DrivingLicence::Ctor()"sv << std::endl;
    }
    DrivingLicence(const DrivingLicence& other)
        : _base(other._base) {
        std::cout << "DrivingLicence::CCtor()"sv << std::endl;
    }
    ~DrivingLicence() {
        std::cout << "DrivingLicence::Dtor()"sv << std::endl;
    }

    OVERIDE_FUNC_VTABLE(Delete, void, noexcept, _base.Delete())
        OVERIDE_FUNC_VTABLE(PrintID, void, const, _base.PrintID())
        OVERIDE_FUNC_VTABLE(Copy, IdentityDocument*, const, _base.Copy())
        OVERIDE_FUNC_VTABLE(GetID, int, const, _base.GetID())
        OVERIDE_FUNC_VTABLE(PrintUniqueIDCount, void, const, _base.PrintUniqueIDCount())

        operator IdentityDocument () {
        _base.Reset();
        return _base;
    }
    operator IdentityDocument* () {
        return &_base;
    }
    void Reset() {
        _base._VTable(&_default);
    }

};
inline IdentityDocumentVTable DrivingLicence::_default = {
    &DrivingLicence::BasePrint,
    &DrivingLicence::BaseDelete,
    &DrivingLicence::BaseCopy
};
