#include "Labs/4-Final/Levels/TargetPracticeLevel.h"
#include "Labs/4-Final/Levels/LevelCommon.h"

namespace VCX::Labs::Final {
    void TargetPracticeLevel::Setup(AngryBirdsPhysics & physics, float breakThreshold) const {
        LevelBuilder builder(physics, breakThreshold);

        for (int i = 0; i < 3; ++i) {
            float const x = 3.0f + float(i) * 1.1f;
            builder.AddStone(glm::vec3(x, .22f, 0.f), glm::vec3(.45f, .22f, .45f));
            builder.AddTarget(glm::vec3(x, .72f, 0.f), glm::vec3(.28f, .28f, .28f));
        }
    }
}
