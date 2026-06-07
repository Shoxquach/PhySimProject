#include "Labs/4-Final/Levels/Level01_ClassicTower.h"
#include "Labs/4-Final/Levels/LevelCommon.h"

namespace VCX::Labs::Final {
    void Level01_ClassicTower::Setup(World & world, float breakThreshold) {
        auto & r = world.Rigid;
        float const t = breakThreshold;

        // 좌우 돌기둥 (각 2단).
        for (float x : { 2.6f, 5.4f }) {
            r.AddBox(BodyKind::Stone, glm::vec3(x, 0.45f, 0.f), glm::vec3(.40f, .45f, .60f), Mat::DensStone, Mat::Stone, t * 1.7f);
            r.AddBox(BodyKind::Stone, glm::vec3(x, 1.32f, 0.f), glm::vec3(.40f, .42f, .60f), Mat::DensStone, Mat::Stone, t * 1.7f);
        }

        // 상단 가로보 (나무) + 그 위 유리 상자 2개 + 돌지붕.
        r.AddBox(BodyKind::Wood,  glm::vec3(4.0f, 1.95f, 0.f), glm::vec3(1.90f, .22f, .60f), Mat::DensWood,  Mat::Wood,  t);
        r.AddBox(BodyKind::Glass, glm::vec3(3.3f, 2.42f, 0.f), glm::vec3(.50f, .25f, .50f), Mat::DensGlass, Mat::Glass, t * .72f);
        r.AddBox(BodyKind::Glass, glm::vec3(4.7f, 2.42f, 0.f), glm::vec3(.50f, .25f, .50f), Mat::DensGlass, Mat::Glass, t * .72f);
        r.AddBox(BodyKind::Stone, glm::vec3(4.0f, 2.92f, 0.f), glm::vec3(1.10f, .22f, .55f), Mat::DensStone, Mat::Stone, t * 1.7f);

        // 타깃: 가운데(유리 사이) + 좌우 기둥 위.
        r.AddBox(BodyKind::Target, glm::vec3(4.0f, 2.45f, 0.f), glm::vec3(.30f, .30f, .30f), Mat::DensTarget, Mat::Target, t * .36f);
        r.AddBox(BodyKind::Target, glm::vec3(2.6f, 2.05f, 0.f), glm::vec3(.28f, .28f, .28f), Mat::DensTarget, Mat::Target, t * .40f);
        r.AddBox(BodyKind::Target, glm::vec3(5.4f, 2.05f, 0.f), glm::vec3(.28f, .28f, .28f), Mat::DensTarget, Mat::Target, t * .40f);

        // 앞쪽 나무상자 더미.
        r.AddBox(BodyKind::Wood, glm::vec3(1.3f, 0.35f,  .42f), glm::vec3(.35f, .35f, .35f), Mat::DensWood, Mat::Wood, t);
        r.AddBox(BodyKind::Wood, glm::vec3(1.3f, 0.35f, -.42f), glm::vec3(.35f, .35f, .35f), Mat::DensWood, Mat::Wood, t);
        r.AddBox(BodyKind::Wood, glm::vec3(1.3f, 1.06f,  0.f),  glm::vec3(.35f, .35f, .35f), Mat::DensWood, Mat::Wood, t);
    }

    GameState Level01_ClassicTower::Status(World const & world) const {
        return TargetsClearedStatus(world);
    }
} // namespace VCX::Labs::Final
