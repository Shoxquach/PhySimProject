#include "Labs/4-Final/Levels/Level03_TargetPractice.h"
#include "Labs/4-Final/Levels/LevelCommon.h"

namespace VCX::Labs::Final {
    void Level03_TargetPractice::Setup(World & world, float breakThreshold) {
        auto & r = world.Rigid;
        float const t = breakThreshold;

        // 앞줄: 낮은 기둥 위 타깃 4개.
        for (int i = 0; i < 4; ++i) {
            float const x = 2.6f + float(i) * 1.15f;
            r.AddBox(BodyKind::Stone,  glm::vec3(x, 0.25f, 0.f), glm::vec3(.40f, .25f, .45f), Mat::DensStone,  Mat::Stone,  t * 1.7f);
            r.AddBox(BodyKind::Wood,   glm::vec3(x, 0.88f, 0.f), glm::vec3(.18f, .38f, .18f), Mat::DensWood,   Mat::Wood,   t);
            r.AddBox(BodyKind::Target, glm::vec3(x, 1.55f, 0.f), glm::vec3(.27f, .27f, .27f), Mat::DensTarget, Mat::Target, t * .3f);
        }

        // 뒷줄: 높은 기둥 위 타깃 3개 (z 뒤쪽).
        for (int i = 0; i < 3; ++i) {
            float const x = 3.15f + float(i) * 1.15f;
            r.AddBox(BodyKind::Stone,  glm::vec3(x, 0.25f, -1.7f), glm::vec3(.40f, .25f, .40f), Mat::DensStone,  Mat::Stone,  t * 1.7f);
            r.AddBox(BodyKind::Wood,   glm::vec3(x, 1.05f, -1.7f), glm::vec3(.18f, .55f, .18f), Mat::DensWood,   Mat::Wood,   t);
            r.AddBox(BodyKind::Target, glm::vec3(x, 1.95f, -1.7f), glm::vec3(.27f, .27f, .27f), Mat::DensTarget, Mat::Target, t * .3f);
        }

        // 가운데 유리 벽 (장애물) — 정확히 안 맞추면 타깃까지 못 감.
        r.AddBox(BodyKind::Glass, glm::vec3(1.9f, 0.85f, 0.f), glm::vec3(.25f, .85f, 1.6f), Mat::DensGlass, Mat::Glass, t * .7f);
    }

    GameState Level03_TargetPractice::Status(World const & world) const {
        return TargetsClearedStatus(world);
    }
} // namespace VCX::Labs::Final
