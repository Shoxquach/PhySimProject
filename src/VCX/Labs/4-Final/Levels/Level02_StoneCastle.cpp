#include "Labs/4-Final/Levels/Level02_StoneCastle.h"
#include "Labs/4-Final/Levels/LevelCommon.h"

namespace VCX::Labs::Final {
    void Level02_StoneCastle::Setup(World & world, float breakThreshold) {
        auto & r = world.Rigid;
        float const t = breakThreshold;

        // 城墙底座 (长木).
        r.AddBox(BodyKind::Wood, glm::vec3(4.0f, 0.22f, 0.f), glm::vec3(2.40f, .22f, .90f), Mat::DensWood, Mat::Wood, t);

        // 左右塔 (石2层) + 塔顶目标.
        for (float x : { 2.0f, 6.0f }) {
            r.AddBox(BodyKind::Stone, glm::vec3(x, 0.94f, 0.f), glm::vec3(.45f, .50f, .60f), Mat::DensStone, Mat::Stone, t * 1.6f);
            r.AddBox(BodyKind::Stone, glm::vec3(x, 1.82f, 0.f), glm::vec3(.40f, .40f, .55f), Mat::DensStone, Mat::Stone, t * 1.6f);
            r.AddBox(BodyKind::Target, glm::vec3(x, 2.48f, 0.f), glm::vec3(.28f, .28f, .28f), Mat::DensTarget, Mat::Target, t * .4f);
        }

        // 城墙 (中间玻璃窗 + 左右木).
        r.AddBox(BodyKind::Wood,  glm::vec3(3.2f, 0.96f, 0.f), glm::vec3(.30f, .50f, .60f), Mat::DensWood,  Mat::Wood,  t);
        r.AddBox(BodyKind::Wood,  glm::vec3(4.8f, 0.96f, 0.f), glm::vec3(.30f, .50f, .60f), Mat::DensWood,  Mat::Wood,  t);
        r.AddBox(BodyKind::Glass, glm::vec3(4.0f, 0.96f, 0.f), glm::vec3(.50f, .50f, .60f), Mat::DensGlass, Mat::Glass, t * .7f);

        // 城墙上通道(玻璃) + 内堡(石) + 顶部目标.
        r.AddBox(BodyKind::Glass,  glm::vec3(4.0f, 1.62f, 0.f), glm::vec3(1.40f, .22f, .70f), Mat::DensGlass, Mat::Glass, t * .72f);
        r.AddBox(BodyKind::Stone,  glm::vec3(4.0f, 2.12f, 0.f), glm::vec3(.50f, .30f, .50f), Mat::DensStone, Mat::Stone, t * 1.7f);
        r.AddBox(BodyKind::Target, glm::vec3(4.0f, 2.62f, 0.f), glm::vec3(.30f, .30f, .30f), Mat::DensTarget, Mat::Target, t * .36f);
    }

    GameState Level02_StoneCastle::Status(World const & world) const {
        return TargetsClearedStatus(world);
    }
} // namespace VCX::Labs::Final
