#pragma once

#include "vtabl/identity_document.hpp"
#include <iostream>
#include <string>

using namespace std::string_view_literals;

struct TravelPack {
private:
    static IdentityDocumentVTable _default;
    static void BasePrint(const IdentityDocument* base) {
        auto* self = reinterpret_cast<const TravelPack*>(base);
        if (self->identity_doc1_) { 
            self->identity_doc1_->PrintID(); 
        }
        if (self->identity_doc2_) {
            self->identity_doc2_->PrintID(); 
        }
        self->additional_pass_.PrintID();
        self->additional_dr_licence_.PrintID();
    }
    static void BaseDelete(IdentityDocument* self) {
        delete reinterpret_cast<TravelPack*>(self);
    }
    static IdentityDocument* BaseCopy(const IdentityDocument* base) {
        auto* self = reinterpret_cast<const TravelPack*>(base);
        auto* copy = new TravelPack(*self);
        return &copy->_base;
    }
public:
    IdentityDocument _base;

    TravelPack()
        : _base({})
        , identity_doc1_((IdentityDocument*)(new Passport()))
        , identity_doc2_((IdentityDocument*)(new DrivingLicence()))
        , additional_pass_({})
        , additional_dr_licence_({})
    {
        _base._VTable()->verify(&_base, this);
        _base._VTable(&_default);
        std::cout << "TravelPack::Ctor()"sv << std::endl;
    }

    TravelPack(const TravelPack& other)
        : _base(other._base)
        , identity_doc1_((IdentityDocument*)(reinterpret_cast<const Passport*>(other.identity_doc1_)->Copy()))
        , identity_doc2_((IdentityDocument*)(reinterpret_cast<const DrivingLicence*>(other.identity_doc2_)->Copy()))
        , additional_pass_(other.additional_pass_)
        , additional_dr_licence_(other.additional_dr_licence_)
    {
        std::cout << "TravelPack::CCtor()"sv << std::endl;
    }

    ~TravelPack() {
        identity_doc1_->Delete();
        identity_doc2_->Delete();
        std::cout << "TravelPack::Dtor()"sv << std::endl;
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
private:
    IdentityDocument* identity_doc1_;
    IdentityDocument* identity_doc2_;
    Passport additional_pass_;
    DrivingLicence additional_dr_licence_;
};
inline IdentityDocumentVTable TravelPack::_default = {
    &TravelPack::BasePrint,
    &TravelPack::BaseDelete,
    &TravelPack::BaseCopy
};
