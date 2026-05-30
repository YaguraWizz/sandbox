#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "vtabl/identity_document.hpp"
#include "vtabl/passport.hpp"
#include "vtabl/driving_licence.hpp"
#include "vtabl/international_driving_licence.hpp"
#include "vtabl/travel_pack.hpp"


TEST(VTableTest, IdentityDocumentLifetimeAndId) {
    auto* doc1 = new IdentityDocument();
    auto* doc2 = new IdentityDocument();

    EXPECT_NE(doc1->GetID(), doc2->GetID());
    EXPECT_EQ(doc2->GetID(), doc1->GetID() + 1);
    
    doc1->Delete();
    doc2->Delete();
}

TEST(VTableTest, PolymorphicPointers) {
    std::vector<IdentityDocument*> docs = {
        reinterpret_cast<IdentityDocument*>(new Passport()),
        reinterpret_cast<IdentityDocument*>(new DrivingLicence())
    };

    EXPECT_TRUE(docs[0]->GetID() > 0);
    EXPECT_TRUE(docs[1]->GetID() > 0);
    EXPECT_NE(docs[0]->GetID(), docs[1]->GetID());

    for (auto* doc : docs) {
        doc->Delete();
    }
}

TEST(VTableTest, MultilevelInheritance) {
    auto* int_lic = new InternationalDrivingLicence();
    
    DrivingLicence* lic_ptr = *int_lic;
    IdentityDocument* doc_ptr = *lic_ptr;

    EXPECT_EQ(int_lic->GetID(), doc_ptr->GetID());
    
    int_lic->Delete();
}

TEST(VTableTest, TravelPackDeepCopy) {
    auto* pack1 = new TravelPack();
    auto* pack2 = new TravelPack(*pack1);

    EXPECT_NE(pack1->GetID(), pack2->GetID());

    pack1->Delete();
    
    EXPECT_TRUE(pack2->GetID() > 0);
    pack2->Delete();
}