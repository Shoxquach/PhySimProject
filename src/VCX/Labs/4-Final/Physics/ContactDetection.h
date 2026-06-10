#pragma once

#include <vector>

#include "Labs/4-Final/Physics/PhysicsTypes.h"

namespace VCX::Labs::Final {
    namespace ContactDetection {
        void CollectContacts(std::vector<RigidBody> const & bodies, std::vector<Contact> & contacts);
        bool FindContact(std::vector<RigidBody> const & bodies, int a, int b, Contact & contact);
        bool SphereBoxContact(RigidBody const & sphere, RigidBody const & box, int sphereIndex, int boxIndex, Contact & contact);
        bool BoxBoxContact(RigidBody const & a, RigidBody const & b, int aIndex, int bIndex, Contact & contact);
        bool GroundContact(RigidBody const & body, int index, Contact & contact);
    }
}
