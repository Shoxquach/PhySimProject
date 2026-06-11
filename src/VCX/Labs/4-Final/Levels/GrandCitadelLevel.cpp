#include "Labs/4-Final/Levels/GrandCitadelLevel.h"
#include "Labs/4-Final/Levels/LevelCommon.h"

namespace VCX::Labs::Final {
    void GrandCitadelLevel::Setup(World & world, float breakThreshold) const {
        LevelBuilder b(world.Rigid, breakThreshold);

        b.AddStone(glm::vec3(.8f, .75f, .0f), glm::vec3(.8f, .75f, .5f));
        b.AddWood(glm::vec3(.2f, 2.1f, .0f), glm::vec3(.1f, .6f, .6f));
        b.AddWood(glm::vec3(1.4f, 2.1f, .0f), glm::vec3(.1f, .6f, .6f));
        b.AddWood(glm::vec3(.8f, 2.95f, .0f), glm::vec3(.8f, .25f, .6f));
        b.AddStone(glm::vec3(.6f, 1.7f, .0f), glm::vec3(.2f, .2f, .2f));
        b.AddTarget(glm::vec3(1.f, 1.7f, .0f), glm::vec3(.2f, .2f, .2f));
        b.AddGlass(glm::vec3(.8f, 3.3f, .0f), glm::vec3(.5f, .1f, .4f));
        b.AddGlass(glm::vec3(.45f, 3.5f, .0f), glm::vec3(.1f, .1f, .1f));
        b.AddGlass(glm::vec3(.8f, 3.5f, .0f), glm::vec3(.1f, .1f, .1f));
        b.AddGlass(glm::vec3(1.15f, 3.5f, .0f), glm::vec3(.1f, .1f, .1f));

        b.AddStone(glm::vec3(2.5f, .25f, 0.f), glm::vec3(.85f, .25f, .96f));
        b.AddStone(glm::vec3(5.2f, .25f, 0.f), glm::vec3(.85f, .25f, .96f));
        b.AddGlass(glm::vec3(3.1f, 1.85f, 0.f), glm::vec3(.25f, 1.35f, .25f));
        b.AddGlass(glm::vec3(4.6f, 1.85f, 0.f), glm::vec3(.25f, 1.35f, .25f));
        b.AddWood(glm::vec3(3.85f, .15f, 0.f), glm::vec3(.3f, .15f, .3f));
        b.AddTarget(glm::vec3(3.85f, .5f, 0.f), glm::vec3(.2f, .2f, .2f));
        b.AddWood(glm::vec3(3.85f, 3.5f, 0.f), glm::vec3(1.24f, .3f, .7f));
        b.AddTarget(glm::vec3(2.95f, 4.f, .5f), glm::vec3(.2f, .2f, .2f));
        b.AddGlass(glm::vec3(3.85f, 4.f, .5f), glm::vec3(.2f, .2f, .2f));
        b.AddTarget(glm::vec3(4.75f, 4.f, .5f), glm::vec3(.2f, .2f, .2f));
        b.AddTarget(glm::vec3(2.95f, 4.f, -.5f), glm::vec3(.2f, .2f, .2f));
        b.AddGlass(glm::vec3(3.85f, 4.f, -.5f), glm::vec3(.2f, .2f, .2f));
        b.AddTarget(glm::vec3(4.75f, 4.f, -.5f), glm::vec3(.2f, .2f, .2f));
        b.AddGlass(glm::vec3(3.85f, .8f, 0.f), glm::vec3(.26f, .1f, .3f));
        b.AddWood(glm::vec3(3.85f, .6f, .78f), glm::vec3(1.16f, .1f, .18f));
        b.AddWood(glm::vec3(3.85f, .6f, -.78f), glm::vec3(1.16f, .1f, .18f));
        
        b.AddStone(glm::vec3(6.5f, .6f, 0.f), glm::vec3(.15f, .6f, .7f));
        b.AddStone(glm::vec3(8.1f, .6f, 0.f), glm::vec3(.15f, .6f, .7f));
        b.AddTarget(glm::vec3(7.3f, .2f, 0.f), glm::vec3(.2f, .2f, .2f));
        b.AddWood(glm::vec3(7.3f, 1.3f, 0.f), glm::vec3(1.15f, .1f, .7f));

        b.AddStone(glm::vec3(6.7f, 2.f, 0.f), glm::vec3(.15f, .6f, .55f));
        b.AddStone(glm::vec3(7.9f, 2.f, 0.f), glm::vec3(.15f, .6f, .55f));
        b.AddTarget(glm::vec3(7.3f, 1.6f, 0.f), glm::vec3(.2f, .2f, .2f));
        b.AddWood(glm::vec3(7.3f, 2.7f, 0.f), glm::vec3(.95f, .1f, .55f));

        b.AddStone(glm::vec3(6.9f, 3.4f, 0.f), glm::vec3(.15f, .6f, .3f));
        b.AddStone(glm::vec3(7.7f, 3.4f, 0.f), glm::vec3(.15f, .6f, .3f));
        b.AddTarget(glm::vec3(7.3f, 3.f, 0.f), glm::vec3(.2f, .2f, .2f));
        b.AddWood(glm::vec3(7.3f, 4.1f, 0.f), glm::vec3(.75f, .1f, .3f));
    }
}
