#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

#include <glm/glm.hpp>

#include "Engine/Camera.hpp"
#include "Engine/GL/Frame.hpp"
#include "Engine/GL/Program.h"
#include "Engine/GL/RenderItem.h"
#include "Engine/GL/Texture.hpp"
#include "Labs/4-Final/World.h"
#include "Labs/4-Final/AngryBirdsScene.h"
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
        Engine::Camera                                              _camera { .Eye = glm::vec3(-8.f, 6.f, 15.f), .Target = glm::vec3(3.f, 1.8f, 0.f) };
        Common::OrbitCameraManager                                  _cameraManager;
        Engine::GL::UniqueRenderItem                                 _boxItem;
        Engine::GL::UniqueRenderItem                                 _sphereItem;
        Engine::GL::UniqueRenderItem                                 _lineItem;
        Engine::GL::UniqueRenderItem                                 _fluidItem;
        Engine::GL::UniqueRenderItem                                 _skyItem;
        Engine::GL::UniqueUniformBlock<Rendering::SceneObject::PassConstants> _passConstantsBlock;
        Engine::GL::UniqueTexture2D                                  _diffuseTexture;
        Engine::GL::UniqueTexture2D                                  _groundTexture;
        Engine::GL::UniqueTexture2D                                  _specularTexture;
        Engine::GL::UniqueTexture2D                                  _heightTexture;

        World             _world;
        AngryBirdsScene   _scene;
        GameState         _gameState = GameState::Playing;

        std::vector<glm::vec3> _sphereVertices;
        std::vector<BirdType>  _birdQueue;

        bool  _pause             = false;
        bool  _gameStarted       = false;
        bool  _dragging          = false;
        bool  _birdLaunched      = false;
        bool  _birdMovingToSlingshot = false;
        bool  _developerMode     = false;
        int   _birdIndex         = -1;
        int   _levelIndex        = 0;
        std::size_t _activeBirdSlot = 0;

        int   _score             = 0;
        int   _shotsUsed         = 0;
        int   _prevTargets       = 0;
        int   _prevBlocks        = 0;
        bool  _wonAwarded        = false;

        float _timeSinceBirdLaunch = 0.f;
        float _birdMoveTime        = 0.f;
        float _springConstant    = 45.f;    // spring stiffness constant k (N/m)
        float _springDamping     = 3.5f;    // snap-back damping (lower = longer visible oscillation)
        float _springOscillationFreq = 4.5f;// snap-back oscillation frequency (Hz)

        // spring snap-back animation state
        float _springSnapTime      = 0.f;
        float _springSnapDuration  = 0.60f;
        bool  _springSnapping      = false;
        glm::vec3 _springSnapFrom  = glm::vec3(0.f);
        glm::vec3 _springSnapPos   = glm::vec3(0.f);

        // auto-advance to next level after clearing current
        bool  _autoAdvanceActive  = false;
        float _autoAdvanceTimer   = 0.f;
        float _autoAdvanceDelay   = 2.5f;   // seconds before auto-advancing

        float _maxPull           = 2.2f;   // per-level max pull distance
        float _launchPlaneYaw    = 0.f;    // launch plane rotation around Y (radians)
        float _launchPlaneYawLimit = 3.14f * 10.f / 180.f;

        float _breakThreshold    = 10.f;
        int   _substeps          = 6;
        glm::vec3 _dragPosition  = glm::vec3(-5.5f, 1.65f, 0.f);
        glm::vec3 _birdMoveStart = glm::vec3(0.f);

        void ResetScene();
        int  FindBirdBySlot(std::size_t slot) const;
        glm::vec3 BirdWaitingPosition(std::size_t slot) const;
        glm::vec3 ClampBirdPositionAboveGround(glm::vec3 position) const;
        void StepSimulation(float dt);
        void LaunchBird();
        void ActivateBoomerangBird();
        void BurstWaterBalloon(int index);
        void UpdateScore();
        void HandleSlingshotInput(ImVec2 const & mousePos);

        void BuildStaticGeometry();
        void BuildSphereGeometry();

        void DrawScene(std::pair<std::uint32_t, std::uint32_t> const desiredSize);
        void DrawSky();
        void DrawScenery();
        void DrawHUD();
        void DrawBox(RigidBody const & body);
        void DrawSphere(RigidBody const & body);
        void DrawLine(glm::vec3 const & a, glm::vec3 const & b, glm::vec3 const & color);
        void DrawTrajectoryPreview();
        void DrawLaunchPlaneGuide();
        void DrawSlingshot(glm::vec3 const & leftFork, glm::vec3 const & rightFork, glm::vec3 const & birdPos, float stretchRatio, glm::vec3 const & baseColor);
        void DrawFluid();
        void DrawFluidSurface(FluidWorld const & fluid);

        glm::vec3 GetLaunchPlaneForward() const;
        glm::vec3 GetLaunchPlaneNormal() const;
        glm::vec3 ProjectOntoLaunchPlane(glm::vec3 position) const;
        void      UpdateLaunchPlaneRotation(float dt);
        glm::vec3 ScreenToLaunchPlane(ImVec2 const & mousePos) const;
    };
}
