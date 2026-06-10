#pragma once

#include <vector>

#include "Labs/4-Final/Physics/PhysicsTypes.h"

namespace VCX::Labs::Final {
    namespace ImpulseSolver {
        void ResolveCollisions(
            std::vector<RigidBody> & bodies,
            float restitution,
            float friction,
            std::vector<Contact> & impactContacts
        );
    }
}
