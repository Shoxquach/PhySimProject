#include "Labs/4-Final/Levels/Level04_Moat.h"
#include "Labs/4-Final/Levels/LevelCommon.h"

namespace VCX::Labs::Final {
    void Level04_Moat::Setup(World & world, float breakThreshold) {
        float const t = breakThreshold;

        // --- 유체: 바닥 절반 채운 해자 ---
        glm::vec3 const tankCenter(3.4f, 1.0f, 0.f);
        glm::vec3 const tankSize(6.2f, 2.0f, 3.2f);
        world.Fluid.emplace();
        world.Fluid->Density = 1.0f;
        world.Fluid->Init(16, tankCenter, tankSize, glm::vec3(0.94f, 0.5f, 0.94f));
        world.Couple.Buoyancy = true;

        auto & r = world.Rigid;

        // 물 위에 뜨는 나무 뗏목 (4짝).
        for (float x : { 2.4f, 3.2f, 4.0f, 4.8f }) {
            r.AddBox(BodyKind::Wood, glm::vec3(x, 1.02f, 0.f), glm::vec3(.42f, .28f, .80f), Mat::DensWood, Mat::Wood, t);
        }

        // 뗏목 위 작은 요새 (나무 기둥 + 유리지붕 + 타깃 2).
        r.AddBox(BodyKind::Wood,   glm::vec3(2.8f, 1.55f, 0.f), glm::vec3(.25f, .30f, .60f), Mat::DensWood,  Mat::Wood,  t);
        r.AddBox(BodyKind::Wood,   glm::vec3(4.4f, 1.55f, 0.f), glm::vec3(.25f, .30f, .60f), Mat::DensWood,  Mat::Wood,  t);
        r.AddBox(BodyKind::Glass,  glm::vec3(3.6f, 1.95f, 0.f), glm::vec3(.55f, .22f, .60f), Mat::DensGlass, Mat::Glass, t * .7f);
        r.AddBox(BodyKind::Target, glm::vec3(3.6f, 2.35f, 0.f), glm::vec3(.30f, .30f, .30f), Mat::DensTarget, Mat::Target, t * .4f);
        r.AddBox(BodyKind::Target, glm::vec3(2.4f, 1.55f, 0.f), glm::vec3(.27f, .27f, .27f), Mat::DensTarget, Mat::Target, t * .45f);

        // 떠 있는 통 (휩쓸림용).
        r.AddBox(BodyKind::Wood, glm::vec3(5.4f, 1.10f,  .9f), glm::vec3(.30f, .30f, .30f), Mat::DensWood, Mat::Wood, t);
        r.AddBox(BodyKind::Wood, glm::vec3(1.8f, 1.10f, -.9f), glm::vec3(.30f, .30f, .30f), Mat::DensWood, Mat::Wood, t);

        // 가라앉는 돌 (밀도>물, 대비용).
        r.AddBox(BodyKind::Stone, glm::vec3(5.6f, 1.6f,  .6f), glm::vec3(.3f, .3f, .3f), Mat::DensStone, Mat::Stone, t * 1.7f);
        r.AddBox(BodyKind::Stone, glm::vec3(1.4f, 1.6f, -.6f), glm::vec3(.3f, .3f, .3f), Mat::DensStone, Mat::Stone, t * 1.7f);
    }

    GameState Level04_Moat::Status(World const & world) const {
        return TargetsClearedStatus(world);
    }
} // namespace VCX::Labs::Final
