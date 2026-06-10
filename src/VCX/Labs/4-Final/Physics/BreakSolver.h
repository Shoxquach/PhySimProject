#pragma once

#include <vector>

#include "Labs/4-Final/Physics/PhysicsTypes.h"

namespace VCX::Labs::Final {
    namespace BreakSolver {
        void TryBreakBodies(std::vector<RigidBody> & bodies, std::vector<Contact> const & contacts, int & fragmentsCreated);
    }
}
