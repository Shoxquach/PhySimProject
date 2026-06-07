#pragma once

#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

#include <glm/glm.hpp>

#include "Engine/Camera.hpp"
#include "Engine/GL/Frame.hpp"
#include "Engine/GL/Program.h"
#include "Engine/GL/RenderItem.h"
#include "Engine/GL/Texture.hpp"
#include "Labs/4-Final/World.h"
#include "Labs/4-Final/Levels/ILevel.h"
#include "Labs/Scene/SceneObject.h"
#include "Labs/Common/ICase.h"
#include "Labs/Common/OrbitCameraManager.h"

namespace VCX::Labs::Final {
    class CaseAngryBirds3D : public Common::ICase {
    public:
        CaseAngryBirds3D();

        std::string_view const GetName() override { return "3D Rigid Slingshot"; }

        void                     OnSetupPropsUI() override;
        Common::CaseRenderResult OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) override;
        void                     OnProcessInput(ImVec2 const & pos) override;

    private:
        struct Vertex {
            glm::vec3 Position;
            glm::vec3 Normal;
            glm::vec2 TexCoord;
            glm::vec3 Offset;
        };

        struct FluidVertex {
            glm::vec3 Position;
            glm::vec3 Color;
        };

        Engine::GL::UniqueProgram                                   _program;
        Engine::GL::UniqueProgram                                   _lineProgram;
        Engine::GL::UniqueProgram                                   _skyProgram;
        Engine::GL::UniqueProgram                                   _pointProgram;
        Engine::GL::UniqueRenderFrame                                _frame;
        Engine::Camera                                              _camera { .Eye = glm::vec3(-8.f, 6.f, 15.f) * WorldScale, .Target = glm::vec3(3.f, 1.8f, 0.f) * WorldScale };
        Common::OrbitCameraManager                                  _cameraManager;
        Engine::GL::UniqueRenderItem                                 _boxItem;
        Engine::GL::UniqueRenderItem                                 _sphereItem;
        Engine::GL::UniqueRenderItem                                 _lineItem;
        Engine::GL::UniqueRenderItem                                 _fluidItem;
        Engine::GL::UniqueRenderItem                                 _skyItem;
        Engine::GL::UniqueUniformBlock<Rendering::SceneObject::PassConstants> _passConstantsBlock;
        Engine::GL::UniqueTexture2D                                  _diffuseTexture;
        Engine::GL::UniqueTexture2D                                  _specularTexture;
        Engine::GL::UniqueTexture2D                                  _heightTexture;

        World                                  _world;
        std::vector<std::unique_ptr<ILevel>>   _levels;
        glm::vec3                              _anchor = glm::vec3(-5.5f, 1.35f, 0.f);
        GameState                              _gameState = GameState::Playing;

        std::vector<glm::vec3> _sphereVertices;

        bool  _pause             = false;
        bool  _gameStarted       = false;
        bool  _dragging          = false;
        bool  _birdLaunched      = false;
        int   _birdIndex         = -1;
        int   _levelIndex        = 0;
        ShotType _currentShot    = ShotType::Bird;

        // 分数 / 发射管理
        int   _score             = 0;
        int   _shotsUsed         = 0;
        int   _prevTargets       = 0;
        int   _prevBlocks        = 0;
        float _projRestTimer     = 0.f;   // 发射体静止的时长 (用于装填判定)
        bool  _wonAwarded        = false;
        float _powerScale        = 6.f;
        float _breakThreshold    = 7.f * WorldScale;   // 冲量(速度)阈值也按缩放调整
        int   _substeps          = 6;
        glm::vec3 _dragPosition  = glm::vec3(-5.5f, 1.35f, 0.f) * WorldScale;

        void ResetScene();
        void StepSimulation(float dt);
        int  SpawnProjectile();           // 生成当前选择的发射体(小鸟/水球)
        void ReloadProjectile();          // 发射后自动在弹弓上装填新发射体
        void LaunchBird();
        void BurstBalloon(int index);     // 引爆水球喷出流体粒子
        bool ProjectileIsBalloon() const;
        void UpdateScore();               // 统计被破坏的方块/目标分数
        void HandleSlingshotInput(ImVec2 const & mousePos);
        void DrawHUD();                   // 屏幕顶部分数叠加层

        void BuildStaticGeometry();
        void BuildSphereGeometry();

        void DrawScene(std::pair<std::uint32_t, std::uint32_t> const desiredSize);
        void DrawSky();
        void DrawScenery();
        void DrawBox(RigidBody const & body);
        void DrawSphere(RigidBody const & body);
        void DrawBirdFace(RigidBody const & body);
        void DrawLine(glm::vec3 const & a, glm::vec3 const & b, glm::vec3 const & color);
        void DrawTrajectoryPreview();
        void DrawFluid();

        glm::vec3 ScreenToLaunchPlane(ImVec2 const & mousePos) const;
    };
} // namespace VCX::Labs::Final
