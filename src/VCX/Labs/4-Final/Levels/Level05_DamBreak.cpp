#include "Labs/4-Final/Levels/Level05_DamBreak.h"
#include "Labs/4-Final/Levels/LevelCommon.h"

namespace VCX::Labs::Final {
    void Level05_DamBreak::Setup(World & world, float breakThreshold) {
        float const t = breakThreshold;

        // --- 流体: 左半部分的高水柱 ---
        glm::vec3 const tankCenter(2.0f, 1.3f, 0.f);
        glm::vec3 const tankSize(9.0f, 2.6f, 3.2f);
        world.Fluid.emplace();
        world.Fluid->Density = 1.0f;
        world.Fluid->Init(16, tankCenter, tankSize, glm::vec3(0.5f, 0.78f, 0.92f));
        world.Couple.Buoyancy  = true;
        world.Couple.FlowSolid = true;   // ★ two-way

        auto & r = world.Rigid;

        // --- 水坝: 石块 2列 x 3层 ---
        float const damTough = t * 1.0f;
        for (int col = 0; col < 2; ++col) {
            float const x = 2.0f + float(col) * 0.78f;
            for (int row = 0; row < 3; ++row) {
                float const y = 0.42f + float(row) * 0.84f;
                r.AddBox(BodyKind::Stone, glm::vec3(x, y, 0.f), glm::vec3(.40f, .42f, 1.55f), Mat::DensStone, Mat::Stone, damTough);
            }
        }

        // --- 右侧(干地)村庄: 房屋 + 望楼 + 3目标 ---
        // 房屋 (2木柱 + 玻璃顶 + 顶上目标).
        r.AddBox(BodyKind::Wood,   glm::vec3(4.6f, 0.32f,  .45f), glm::vec3(.28f, .32f, .28f), Mat::DensWood,  Mat::Wood,  t);
        r.AddBox(BodyKind::Wood,   glm::vec3(4.6f, 0.32f, -.45f), glm::vec3(.28f, .32f, .28f), Mat::DensWood,  Mat::Wood,  t);
        r.AddBox(BodyKind::Glass,  glm::vec3(4.6f, 0.86f,  0.f),  glm::vec3(.55f, .22f, .85f), Mat::DensGlass, Mat::Glass, t * .7f);
        r.AddBox(BodyKind::Target, glm::vec3(4.6f, 1.36f,  0.f),  glm::vec3(.28f, .28f, .28f), Mat::DensTarget, Mat::Target, t * .3f);

        // 望楼 (石座 + 目标).
        r.AddBox(BodyKind::Stone,  glm::vec3(5.9f, 0.30f, 0.f), glm::vec3(.50f, .30f, .60f), Mat::DensStone,  Mat::Stone,  t * 1.7f);
        r.AddBox(BodyKind::Target, glm::vec3(5.9f, 0.88f, 0.f), glm::vec3(.28f, .28f, .28f), Mat::DensTarget, Mat::Target, t * .3f);

        // 地面上滚动的目标 (易被水冲走).
        r.AddBox(BodyKind::Target, glm::vec3(3.9f, 0.26f, .8f), glm::vec3(.26f, .26f, .26f), Mat::DensTarget, Mat::Target, t * .3f);

        // 坝后水面漂浮的圆木 (放水时被冲走).
        r.AddBox(BodyKind::Wood, glm::vec3(-0.4f, 1.7f,  .4f), glm::vec3(.35f, .30f, .40f), Mat::DensWood, Mat::Wood, t);
        r.AddBox(BodyKind::Wood, glm::vec3(-0.9f, 1.7f, -.4f), glm::vec3(.35f, .30f, .40f), Mat::DensWood, Mat::Wood, t);
    }

    GameState Level05_DamBreak::Status(World const & world) const {
        return TargetsClearedStatus(world);
    }
} // namespace VCX::Labs::Final
