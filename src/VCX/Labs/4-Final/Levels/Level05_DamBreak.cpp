#include "Labs/4-Final/Levels/Level05_DamBreak.h"
#include "Labs/4-Final/Levels/LevelCommon.h"

namespace VCX::Labs::Final {
    void Level05_DamBreak::Setup(World & world, float breakThreshold) {
        float const t = breakThreshold;

        // --- 유체: 왼쪽 절반에 높은 물기둥 ---
        glm::vec3 const tankCenter(2.0f, 1.3f, 0.f);
        glm::vec3 const tankSize(9.0f, 2.6f, 3.2f);
        world.Fluid.emplace();
        world.Fluid->Density = 1.0f;
        world.Fluid->Init(16, tankCenter, tankSize, glm::vec3(0.5f, 0.78f, 0.92f));
        world.Couple.Buoyancy  = true;
        world.Couple.FlowSolid = true;   // ★ two-way

        auto & r = world.Rigid;

        // --- 댐: 돌블록 2열 x 3단 ---
        float const damTough = t * 1.0f;
        for (int col = 0; col < 2; ++col) {
            float const x = 2.0f + float(col) * 0.78f;
            for (int row = 0; row < 3; ++row) {
                float const y = 0.42f + float(row) * 0.84f;
                r.AddBox(BodyKind::Stone, glm::vec3(x, y, 0.f), glm::vec3(.40f, .42f, 1.55f), Mat::DensStone, Mat::Stone, damTough);
            }
        }

        // --- 오른쪽(마른 땅) 마을: 집 + 망루 + 타깃 3 ---
        // 집 (나무 기둥 2 + 유리지붕 + 지붕 위 타깃).
        r.AddBox(BodyKind::Wood,   glm::vec3(4.6f, 0.32f,  .45f), glm::vec3(.28f, .32f, .28f), Mat::DensWood,  Mat::Wood,  t);
        r.AddBox(BodyKind::Wood,   glm::vec3(4.6f, 0.32f, -.45f), glm::vec3(.28f, .32f, .28f), Mat::DensWood,  Mat::Wood,  t);
        r.AddBox(BodyKind::Glass,  glm::vec3(4.6f, 0.86f,  0.f),  glm::vec3(.55f, .22f, .85f), Mat::DensGlass, Mat::Glass, t * .7f);
        r.AddBox(BodyKind::Target, glm::vec3(4.6f, 1.36f,  0.f),  glm::vec3(.28f, .28f, .28f), Mat::DensTarget, Mat::Target, t * .3f);

        // 망루 (돌 받침 + 타깃).
        r.AddBox(BodyKind::Stone,  glm::vec3(5.9f, 0.30f, 0.f), glm::vec3(.50f, .30f, .60f), Mat::DensStone,  Mat::Stone,  t * 1.7f);
        r.AddBox(BodyKind::Target, glm::vec3(5.9f, 0.88f, 0.f), glm::vec3(.28f, .28f, .28f), Mat::DensTarget, Mat::Target, t * .3f);

        // 바닥에 굴러다니는 타깃 (물에 잘 휩쓸림).
        r.AddBox(BodyKind::Target, glm::vec3(3.9f, 0.26f, .8f), glm::vec3(.26f, .26f, .26f), Mat::DensTarget, Mat::Target, t * .3f);

        // 댐 뒤 물 위에 떠 있는 통나무 (방류 시 휩쓸려 나감).
        r.AddBox(BodyKind::Wood, glm::vec3(-0.4f, 1.7f,  .4f), glm::vec3(.35f, .30f, .40f), Mat::DensWood, Mat::Wood, t);
        r.AddBox(BodyKind::Wood, glm::vec3(-0.9f, 1.7f, -.4f), glm::vec3(.35f, .30f, .40f), Mat::DensWood, Mat::Wood, t);
    }

    GameState Level05_DamBreak::Status(World const & world) const {
        return TargetsClearedStatus(world);
    }
} // namespace VCX::Labs::Final
