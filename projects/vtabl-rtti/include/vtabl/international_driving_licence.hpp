#pragma once

#include "vtabl/driving_licence.hpp"
#include <iostream>
#include <string>

using namespace std::string_view_literals;

struct InternationalDrivingLicence {
private:
    static IdentityDocumentVTable _default;
    static void BasePrint(const IdentityDocument* self) {
        std::cout << "InternationalDrivingLicence::PrintID() : "sv << self->GetID() << std::endl;
    }
    static void BaseDelete(IdentityDocument* self) {
        delete reinterpret_cast<InternationalDrivingLicence*>(self);
    }
    static IdentityDocument* BaseCopy(const IdentityDocument* base) {
        auto* self = reinterpret_cast<const InternationalDrivingLicence*>(base);
        auto* copy = new InternationalDrivingLicence(*self);
        return &copy->_base._base;
    }
public:
    DrivingLicence _base;

    InternationalDrivingLicence() {
        _base._base._VTable()->verify(&_base, this);
        _base._base._VTable(&_default);
        std::cout << "InternationalDrivingLicence::Ctor()"sv << std::endl;
    }
    InternationalDrivingLicence(const InternationalDrivingLicence& other)
        : _base(other._base) {
        std::cout << "InternationalDrivingLicence::CCtor()"sv << std::endl;
    }
    ~InternationalDrivingLicence() {
        std::cout << "InternationalDrivingLicence::Dtor()"sv << std::endl;
    }

    OVERIDE_FUNC_VTABLE(Delete, void, noexcept, _base.Delete())
        OVERIDE_FUNC_VTABLE(PrintID, void, const, _base.PrintID())
        OVERIDE_FUNC_VTABLE(Copy, IdentityDocument*, const, _base.Copy())
        OVERIDE_FUNC_VTABLE(GetID, int, const, _base.GetID())
        OVERIDE_FUNC_VTABLE(PrintUniqueIDCount, void, const, _base.PrintUniqueIDCount())

        operator DrivingLicence () {
        _base.Reset();
        return _base;
    }
    operator DrivingLicence* () {
        return &_base;
    }
};
inline IdentityDocumentVTable InternationalDrivingLicence::_default = {
    &InternationalDrivingLicence::BasePrint,
    &InternationalDrivingLicence::BaseDelete,
    &InternationalDrivingLicence::BaseCopy
};