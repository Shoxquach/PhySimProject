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
#include "Labs/4-Final/AngryBirdsPhysics.h"
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

        Engine::GL::UniqueProgram                                   _program;
        Engine::GL::UniqueProgram                                   _lineProgram;
        Engine::GL::UniqueRenderFrame                                _frame;
        Engine::Camera                                              _camera { .Eye = glm::vec3(-8.f, 6.f, 15.f), .Target = glm::vec3(3.f, 1.8f, 0.f) };
        Common::OrbitCameraManager                                  _cameraManager;
        Engine::GL::UniqueRenderItem                                 _boxItem;
        Engine::GL::UniqueRenderItem                                 _sphereItem;
        Engine::GL::UniqueRenderItem                                 _lineItem;
        Engine::GL::UniqueUniformBlock<Rendering::SceneObject::PassConstants> _passConstantsBlock;
        Engine::GL::UniqueTexture2D                                  _diffuseTexture;
        Engine::GL::UniqueTexture2D                                  _groundTexture;
        Engine::GL::UniqueTexture2D                                  _specularTexture;
        Engine::GL::UniqueTexture2D                                  _heightTexture;

        AngryBirdsPhysics _physics;
        AngryBirdsScene   _scene;

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
        float _timeSinceBirdLaunch = 0.f;
        float _birdMoveTime        = 0.f;
        float _powerScale        = 6.f;
        float _breakThreshold    = 7.f;
        int   _substeps          = 6;
        glm::vec3 _dragPosition  = glm::vec3(-5.5f, 1.35f, 0.f);
        glm::vec3 _birdMoveStart = glm::vec3(0.f);

        void ResetScene();
        int  FindBirdBySlot(std::size_t slot) const;
        glm::vec3 BirdWaitingPosition(std::size_t slot) const;
        void StepSimulation(float dt);
        void LaunchBird();
        void ActivateBoomerangBird();
        void HandleSlingshotInput(ImVec2 const & mousePos);

        void BuildStaticGeometry();
        void BuildSphereGeometry();

        void DrawScene(std::pair<std::uint32_t, std::uint32_t> const desiredSize);
        void DrawBox(RigidBody const & body);
        void DrawSphere(RigidBody const & body);
        void DrawLine(glm::vec3 const & a, glm::vec3 const & b, glm::vec3 const & color);
        void DrawTrajectoryPreview();

        glm::vec3 ScreenToLaunchPlane(ImVec2 const & mousePos) const;
    };
} // namespace VCX::Labs::Final
