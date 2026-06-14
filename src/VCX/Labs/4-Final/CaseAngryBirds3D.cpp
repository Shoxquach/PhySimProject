#include "Labs/4-Final/CaseAngryBirds3D.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdlib>
#include <numbers>
#include <string>

#include <glm/ext.hpp>
#include <imgui_internal.h>

#include "Engine/app.h"
#include "Engine/GL/Texture.hpp"
#include "Engine/loader.h"
#include "Labs/Common/ImGuiHelper.h"
#include "Labs/4-Final/Config.h"
#include "Labs/4-Final/Levels/LevelCommon.h"
#include "Labs/4-Final/Levels/LevelRegister.h"

namespace VCX::Labs::Final {
    namespace {
        float frand() { return float(std::rand()) / float(RAND_MAX); }
        float frand2() { return frand() * 2.f - 1.f; }
    }
    CaseAngryBirds3D::CaseAngryBirds3D():
        _program(Engine::GL::UniqueProgram({ Engine::GL::SharedShader("assets/shaders/sphere_phong.vert"),
                                             Engine::GL::SharedShader("assets/shaders/phong.frag") })),
        _lineProgram(Engine::GL::UniqueProgram({ Engine::GL::SharedShader("assets/shaders/flat.vert"),
                                                 Engine::GL::SharedShader("assets/shaders/flat.frag") })),
        _skyProgram(Engine::GL::UniqueProgram({ Engine::GL::SharedShader("assets/shaders/sky.vert"),
                                                Engine::GL::SharedShader("assets/shaders/sky.frag") })),
        _pointProgram(Engine::GL::UniqueProgram({ Engine::GL::SharedShader("assets/shaders/point.vert"),
                                                  Engine::GL::SharedShader("assets/shaders/point.frag") })),
        _boxItem(Engine::GL::VertexLayout()
            .Add<Vertex>("vertex", Engine::GL::DrawFrequency::Stream)
            .At<Vertex, glm::vec3>(0, &Vertex::Position)
            .At<Vertex, glm::vec3>(1, &Vertex::Normal)
            .At<Vertex, glm::vec2>(2, &Vertex::TexCoord)
            .At<Vertex, glm::vec3>(3, &Vertex::Offset), Engine::GL::PrimitiveType::Triangles),
        _sphereItem(Engine::GL::VertexLayout()
            .Add<Vertex>("vertex", Engine::GL::DrawFrequency::Stream)
            .At<Vertex, glm::vec3>(0, &Vertex::Position)
            .At<Vertex, glm::vec3>(1, &Vertex::Normal)
            .At<Vertex, glm::vec2>(2, &Vertex::TexCoord)
            .At<Vertex, glm::vec3>(3, &Vertex::Offset), Engine::GL::PrimitiveType::Triangles),
        _lineItem(Engine::GL::VertexLayout().Add<glm::vec3>("position", Engine::GL::DrawFrequency::Stream, 0), Engine::GL::PrimitiveType::Lines),
        _fluidItem(Engine::GL::VertexLayout()
            .Add<FluidVertex>("vertex", Engine::GL::DrawFrequency::Stream)
            .At<FluidVertex, glm::vec3>(0, &FluidVertex::Position)
            .At<FluidVertex, glm::vec3>(1, &FluidVertex::Color), Engine::GL::PrimitiveType::Points),
        _skyItem(Engine::GL::VertexLayout().Add<glm::vec3>("position", Engine::GL::DrawFrequency::Static, 0), Engine::GL::PrimitiveType::Triangles),
        _passConstantsBlock(1, Engine::GL::DrawFrequency::Stream) {
        BuildStaticGeometry();
        BuildSphereGeometry();

        std::vector<glm::vec3> const skyVerts = {
            { -1.f, -1.f, 0.f }, { 1.f, -1.f, 0.f }, { 1.f, 1.f, 0.f },
            { -1.f, -1.f, 0.f }, { 1.f, 1.f, 0.f }, { -1.f, 1.f, 0.f },
        };
        _skyItem.UpdateVertexBuffer("position", Engine::make_span_bytes<glm::vec3>(skyVerts));

        VCX::Engine::Texture2D<VCX::Engine::Formats::RGBA8> diffuse{1, 1};
        diffuse.Fill({ 0xff, 0xff, 0xff, 0xff });
        _diffuseTexture = Engine::GL::UniqueTexture2D(diffuse, 0);
        _groundTexture = Engine::GL::UniqueTexture2D(
            Engine::LoadImageRGBA("assets/images/ground.jpg"),
            {
                .WrapU     = Engine::GL::WrapMode::Repeat,
                .WrapV     = Engine::GL::WrapMode::Repeat,
                .MinFilter = Engine::GL::FilterMode::Trilinear,
                .MagFilter = Engine::GL::FilterMode::Linear,
            },
            0);

        VCX::Engine::Texture2D<VCX::Engine::Formats::RGBA8> specular{1, 1};
        specular.Fill({ 0xff, 0xff, 0xff, 0xff });
        _specularTexture = Engine::GL::UniqueTexture2D(specular, 1);

        VCX::Engine::Texture2D<VCX::Engine::Formats::RGBA8> height{1, 1};
        height.Fill({ 0x80, 0x80, 0x80, 0xff });
        _heightTexture = Engine::GL::UniqueTexture2D(height, 2);

        _program.BindUniformBlock("PassConstants", 1);
        _program.GetUniforms().SetByName("u_DiffuseMap", 0);
        _program.GetUniforms().SetByName("u_SpecularMap", 1);
        _program.GetUniforms().SetByName("u_HeightMap", 2);
        _program.GetUniforms().SetByName("u_AmbientScale", 0.25f);
        _program.GetUniforms().SetByName("u_UseBlinn", int(true));
        _program.GetUniforms().SetByName("u_Shininess", 32.f);
        _program.GetUniforms().SetByName("u_UseGammaCorrection", int(false));
        _program.GetUniforms().SetByName("u_AttenuationOrder", 2);
        _program.GetUniforms().SetByName("u_BumpMappingBlend", 0.f);
        _program.GetUniforms().SetByName("u_Alpha", 1.f);

        _lineProgram.GetUniforms().SetByName("u_Color", glm::vec3(1.f));

        _cameraManager.AutoRotate = false;
        _cameraManager.EnableDamping = true;
        _cameraManager.EnablePan = true;
        _cameraManager.MinDistance = 4.f;
        _cameraManager.MaxDistance = 40.f;
        _cameraManager.Save(_camera);

        ResetScene();
    }

    void CaseAngryBirds3D::OnSetupPropsUI() {
        static char const * const LevelNames[] = {
            "Classic Tower",
            "Stone Castle",
            "Target Practice",
            "Domino Run",
            "Boomerang Challenge",
            "Grand Citadel",
            "Moat (Buoyancy)",
            "Dam Break (Two-way)",
            "Sunken Temple",
            "Overhang Fort",
        };

        if (ImGui::Combo("Level", &_levelIndex, LevelNames, IM_ARRAYSIZE(LevelNames))) {
            ResetScene();
        }

        if (ImGui::Button("Reset Scene", ImVec2(250, 0))) {
            ResetScene();
        }
        ImGui::SameLine();
        if (ImGui::Button(_pause ? "Resume" : "Pause", ImVec2(120, 0))) {
            _pause = !_pause;
        }

        int aliveBreakables = 0;
        int aliveTargets = 0;
        for (auto const & body : _world.Rigid.Bodies) {
            if (body.IsAlive && body.Breakable && body.Kind != BodyKind::Bird) {
                aliveBreakables++;
            }
            if (body.IsAlive && body.Kind == BodyKind::Target) {
                aliveTargets++;
            }
        }

        ImGui::Spacing();
        ImGui::TextDisabled("Camera: Left-drag=rotate  Right-drag=pan  Wheel=zoom");
        if (!_gameStarted) {
            ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Press S to start");
        } else {
            ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Ready - drag the bird to launch");
            ImGui::TextDisabled("A/D = rotate launch plane (+-10 deg)");
        }
        if (_birdIndex >= 0 && _birdIndex < int(_world.Rigid.Bodies.size())) {
            auto const & bird = _world.Rigid.Bodies[_birdIndex];
            if (bird.Bird == BirdType::Boomerang) {
                ImGui::Text("Boomerang: press G before impact");
            } else if (bird.Bird == BirdType::WaterBalloon) {
                ImGui::Text("Water Balloon: bursts on impact or in water");
            }
        }
        ImGui::Text("Targets left: %d", aliveTargets);
        ImGui::Text("Score: %d", _score);
        if (_world.Fluid) {
            ImGui::Text("Fluid particles: %d", _world.Fluid->ParticleCount());
        }
        if (_gameState == GameState::Won) {
            ImGui::TextColored(ImVec4(0.2f, 1.f, 0.2f, 1.f), "LEVEL CLEARED!");
            if (_autoAdvanceActive) {
                float const remaining = std::max(_autoAdvanceDelay - _autoAdvanceTimer, 0.f);
                ImGui::TextColored(ImVec4(1.f, 1.f, 0.4f, 1.f), "Next level in %.1fs...", remaining);
            }
        }

        ImGui::Spacing();
        ImGui::Checkbox("Developer Mode", &_developerMode);
        if (_developerMode) {
            ImGui::Separator();
            if (ImGui::Button("Reset Camera", ImVec2(250, 0))) {
                _cameraManager.Reset(_camera);
            }

            ImGui::Checkbox("Pause", &_pause);
            ImGui::SliderFloat("Auto-Advance Delay", &_autoAdvanceDelay, 0.5f, 8.f, "%.1f");
            ImGui::SliderFloat("Spring Constant (k)", &_springConstant, 8.f, 80.f, "%.1f");
            ImGui::SliderFloat("Spring Damping", &_springDamping, 2.f, 30.f, "%.1f");
            ImGui::SliderFloat("Oscillation Freq", &_springOscillationFreq, 2.f, 10.f, "%.1f");
            ImGui::SliderFloat("Break Threshold", &_breakThreshold, 2.f, 18.f, "%.1f");
            ImGui::SliderFloat("Restitution", &_world.Rigid.Restitution, .05f, .8f, "%.2f");
            ImGui::SliderFloat("Friction", &_world.Rigid.Friction, .2f, .98f, "%.2f");
            ImGui::SliderInt("Substeps", &_substeps, 1, 12);

            ImGui::Separator();
            static char const * const SolverTypeNames[] = {
                "Sequential Impulse",
                "Constraint-Based (Jacobi)",
            };
            int solverType = static_cast<int>(_world.Rigid.GetSolverType());
            if (ImGui::Combo("Solver Type", &solverType, SolverTypeNames, IM_ARRAYSIZE(SolverTypeNames))) {
                _world.Rigid.SetSolverType(static_cast<SolverType>(solverType));
            }
            ImGui::Text("Current Solver: %s", SolverTypeNames[solverType]);

            ImGui::Separator();
            ImGui::Text("Press R to reset level.");
            ImGui::Text("Alive breakable blocks: %d", aliveBreakables);
            ImGui::Text("Fragments created: %d", _world.Rigid.FragmentsCreated);
        }
    }

    Common::CaseRenderResult CaseAngryBirds3D::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        if (ImGui::IsKeyPressed(ImGuiKey_S, false)) {
            _gameStarted = !_gameStarted;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Space, false)) {
            _pause = !_pause;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_R, false)) {
            _gameStarted = false;
            ResetScene();
        }
        if (ImGui::IsKeyPressed(ImGuiKey_G, false)) {
            ActivateBoomerangBird();
        }

        float const frameDt = std::min(Engine::GetDeltaTime(), 1.f / 30.f);
        if (_gameStarted && !_birdLaunched && !_birdMovingToSlingshot) {
            UpdateLaunchPlaneRotation(frameDt);
        }
        if (!_pause) {
            int const steps = std::max(_substeps, 1);
            for (int i = 0; i < steps; ++i) {
                StepSimulation(frameDt / float(steps));
            }
            _world.StepFluid(frameDt);
        }

        _cameraManager.Update(_camera);
        DrawScene(desiredSize);
        DrawHUD();

        return Common::CaseRenderResult {
            .Fixed     = false,
            .Flipped   = true,
            .Image     = _frame.GetColorAttachment(),
            .ImageSize = desiredSize,
        };
    }

    void CaseAngryBirds3D::OnProcessInput(ImVec2 const & pos) {
        if (_gameStarted) {
            ImGuiIO const & io = ImGui::GetIO();
            // Pass through to camera when using right-click, Ctrl, or wheel — even while aiming
            bool const cameraOp = io.KeyCtrl || io.KeyShift ||
                ImGui::IsMouseDown(ImGuiMouseButton_Right) ||
                ImGui::IsMouseDown(ImGuiMouseButton_Middle) ||
                io.MouseWheel != 0.f;
            if (cameraOp) {
                _cameraManager.ProcessInput(_camera, pos);
            } else {
                HandleSlingshotInput(pos);
            }
        } else {
            _cameraManager.ProcessInput(_camera, pos);
        }
    }

    void CaseAngryBirds3D::ResetScene() {
        _dragging = false;
        _birdLaunched = false;
        _birdMovingToSlingshot = false;
        _launchPlaneYaw = 0.f;
        _dragPosition = _scene.Anchor;
        _birdQueue = _scene.Reset(_world, _breakThreshold, static_cast<LevelRegister::LevelID>(_levelIndex));
        _activeBirdSlot = 0;
        {
            auto lvl = LevelRegister::GetInstance().GetLevel(static_cast<LevelRegister::LevelID>(_levelIndex));
            _maxPull = lvl ? lvl->GetMaxPull() : 2.2f;
        }
        _timeSinceBirdLaunch = 0.f;
        _birdMoveTime = 0.f;
        _gameState = GameState::Playing;
        _shotsUsed = 0;
        _wonAwarded = false;
        _autoAdvanceActive = false;
        _autoAdvanceTimer  = 0.f;
        _prevTargets = CountAliveTargets(_world);
        _prevBlocks = CountAliveBreakables(_world);

        for (std::size_t i = 0; i < _birdQueue.size(); ++i) {
            glm::vec3 const position = i == 0 ? _scene.Anchor : BirdWaitingPosition(i);
            _world.Rigid.AddBird(position, _birdQueue[i], int(i));
        }
        _birdIndex = FindBirdBySlot(_activeBirdSlot);
        _gameStarted = false;
    }

    int CaseAngryBirds3D::FindBirdBySlot(std::size_t slot) const {
        for (int i = 0; i < int(_world.Rigid.Bodies.size()); ++i) {
            auto const & body = _world.Rigid.Bodies[i];
            if (body.IsAlive && body.Kind == BodyKind::Bird && body.BirdSlot == int(slot)) {
                return i;
            }
        }
        return -1;
    }

    glm::vec3 CaseAngryBirds3D::BirdWaitingPosition(std::size_t slot) const {
        float const queueIndex = slot > 0 ? float(slot - 1) : 0.f;
        return ClampBirdPositionAboveGround(glm::vec3(-6.5f - 0.95f * queueIndex, GroundY + BirdRadius + 0.08f, 0.82f));
    }

    glm::vec3 CaseAngryBirds3D::ClampBirdPositionAboveGround(glm::vec3 position) const {
        constexpr float GroundClearance = 0.08f;
        position.y = std::max(position.y, GroundY + BirdRadius + GroundClearance);
        return position;
    }

    void CaseAngryBirds3D::StepSimulation(float dt) {
        constexpr float NextBirdDelay = 2.f;
        constexpr float BirdMoveDuration = .55f;

        if (_birdLaunched) {
            _timeSinceBirdLaunch += dt;
            if (_timeSinceBirdLaunch >= NextBirdDelay && _activeBirdSlot + 1 < _birdQueue.size()) {
                _activeBirdSlot++;
                _birdIndex = FindBirdBySlot(_activeBirdSlot);
                if (_birdIndex >= 0) {
                    _birdMovingToSlingshot = true;
                    _birdLaunched = false;
                    _dragging = false;
                    _dragPosition = _scene.Anchor;
                    _birdMoveStart = _world.Rigid.Bodies[_birdIndex].Position;
                    _birdMoveTime = 0.f;
                }
            }
        }

        std::vector<PinnedBody> pinnedBodies;
        pinnedBodies.reserve(_birdQueue.size());
        for (int i = 0; i < int(_world.Rigid.Bodies.size()); ++i) {
            auto const & body = _world.Rigid.Bodies[i];
            if (!body.IsAlive || body.Kind != BodyKind::Bird || body.BirdSlot < 0) continue;

            std::size_t const slot = std::size_t(body.BirdSlot);
            if (slot < _activeBirdSlot) continue;

            glm::vec3 position = BirdWaitingPosition(slot);
            if (slot == _activeBirdSlot) {
                if (_birdLaunched) continue;

                if (_birdMovingToSlingshot) {
                    _birdMoveTime += dt;
                    float const t = glm::clamp(_birdMoveTime / BirdMoveDuration, 0.f, 1.f);
                    position = glm::mix(_birdMoveStart, _scene.Anchor, t);
                    if (t >= 1.f) {
                        _birdMovingToSlingshot = false;
                    }
                } else {
                    position = _dragging ? _dragPosition : _scene.Anchor;
                }
            }

            position = ClampBirdPositionAboveGround(position);
            pinnedBodies.push_back(PinnedBody { i, position });
        }

        int const buoyancySkip = (_birdLaunched && _birdIndex >= 0) ? _birdIndex : -1;
        _world.StepRigid(dt, pinnedBodies, buoyancySkip);

        if (_birdLaunched && _birdIndex >= 0 && _birdIndex < int(_world.Rigid.Bodies.size())) {
            auto & bird = _world.Rigid.Bodies[_birdIndex];
            if (bird.IsAlive && bird.Bird == BirdType::WaterBalloon) {
                bool const inWater = _world.Fluid && _world.Fluid->InsideTankXZ(bird.Position)
                    && bird.Position.y < _world.Fluid->SurfaceWorldY(bird.Position.x, bird.Position.z);
                if (bird.LastImpact > 2.0f || inWater) {
                    BurstWaterBalloon(_birdIndex);
                    _birdIndex = -1;
                }
            }
        }

        if (!_birdLaunched) {
            _birdIndex = FindBirdBySlot(_activeBirdSlot);
        }

        auto level = LevelRegister::GetInstance().GetLevel(static_cast<LevelRegister::LevelID>(_levelIndex));
        if (level) {
            level->Tick(_world, dt);
            _gameState = level->Status(_world);
            if (_gameState == GameState::Won && !_wonAwarded) {
                _score += 1000;
                _wonAwarded = true;
            }
        }
        UpdateScore();

        // Auto-advance to next level when current is cleared
        if (_gameState == GameState::Won && !_autoAdvanceActive) {
            _autoAdvanceActive = true;
            _autoAdvanceTimer  = 0.f;
        }
        if (_autoAdvanceActive) {
            _autoAdvanceTimer += dt;
            if (_autoAdvanceTimer >= _autoAdvanceDelay) {
                _autoAdvanceActive = false;
                _levelIndex = (_levelIndex + 1) % 10;  // 8 levels, wrap around
                ResetScene();
            }
        }

        // Spring snap-back animation after launch
        if (_springSnapping) {
            _springSnapTime += dt;
            float const t = glm::clamp(_springSnapTime / _springSnapDuration, 0.f, 1.f);
            // Damped oscillation: e^(-damping*t) * cos(2π*freq*t)
            // Let the exponential decay naturally — no extra (1-t) multiplier
            float const envelope = std::exp(-_springDamping * t);
            float const oscillation = std::cos(2.0f * 3.14159f * _springOscillationFreq * t);
            float const amplitude = envelope * oscillation;
            _springSnapPos = glm::mix(_scene.Anchor, _springSnapFrom, amplitude);
            if (t >= 1.f) {
                _springSnapping = false;
                _springSnapPos = _scene.Anchor;
            }
        }
    }

    void CaseAngryBirds3D::LaunchBird() {
        if (_birdMovingToSlingshot || _birdIndex < 0 || _birdIndex >= int(_world.Rigid.Bodies.size())) return;
        auto & bird = _world.Rigid.Bodies[_birdIndex];
        glm::vec3 pull = _scene.Anchor - _dragPosition;
        float const stretch = glm::length(pull);
        float launchSpeedScale = 1.f;
        if (bird.Bird == BirdType::Boomerang) {
            launchSpeedScale = 1.3f;
        } else if (bird.Bird == BirdType::Speed) {
            launchSpeedScale = 1.5f;
        } else if (bird.Bird == BirdType::WaterBalloon) {
            launchSpeedScale = 0.85f;
        }

        // Spring energy conservation: ½k·stretch² = ½m·v²  →  v = stretch·√(k/m)
        float const springSpeed = stretch * std::sqrt(_springConstant / bird.Mass);
        glm::vec3 const launchDir = stretch > 1e-4f ? pull / stretch : GetLaunchPlaneForward();

        bird.Position = ClampBirdPositionAboveGround(_dragPosition);
        bird.Velocity = launchDir * springSpeed * launchSpeedScale;
        bird.AngularVel = glm::vec3(0.f, 0.f, -stretch * 8.f);
        bird.Age = 0.f;
        bird.LifeTime = 10.f;
        bird.Scale = 1.f;
        bird.BirdWasLaunched = true;
        bird.BirdHasCollided = false;
        bird.BoomerangActive = false;
        bird.BoomerangAcceleration = glm::vec3(0.f);
        _birdLaunched = true;
        _timeSinceBirdLaunch = 0.f;
        _gameStarted = false;
        _shotsUsed++;

        // Trigger spring snap-back animation
        _springSnapping = true;
        _springSnapTime = 0.f;
        _springSnapFrom = _dragPosition;
        _springSnapPos  = _dragPosition;
    }

    void CaseAngryBirds3D::ActivateBoomerangBird() {
        if (!_birdLaunched || _birdIndex < 0 || _birdIndex >= int(_world.Rigid.Bodies.size())) return;

        auto & bird = _world.Rigid.Bodies[_birdIndex];
        if (bird.Kind != BodyKind::Bird || bird.Bird != BirdType::Boomerang || bird.BirdHasCollided) return;

        constexpr float BoomerangAcceleration = 26.f;
        bird.BoomerangAcceleration = glm::vec3(-BoomerangAcceleration, 0.f, 0.f);
        bird.BoomerangActive = true;
    }

    void CaseAngryBirds3D::HandleSlingshotInput(ImVec2 const & mousePos) {
        auto * window = ImGui::GetCurrentWindow();
        bool hover = false;
        bool anyHeld = false;
        ImGui::ButtonBehavior(window->Rect(), window->GetID("##io"), &hover, &anyHeld);

        bool const leftClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
        bool const leftHeld = ImGui::IsMouseDown(ImGuiMouseButton_Left);
        bool const leftReleased = ImGui::IsMouseReleased(ImGuiMouseButton_Left);
        ImGuiIO const & io = ImGui::GetIO();

        if (!_birdLaunched && !_birdMovingToSlingshot && hover && leftClicked && !io.KeyCtrl && !io.KeyShift && !io.KeyAlt) {
            _dragging = true;
        }
        if (_dragging && leftHeld) {
            _dragPosition = ScreenToLaunchPlane(mousePos);

            float const maxPull = _maxPull;
            float const pullLen = glm::length(_dragPosition - _scene.Anchor);
            if (pullLen > maxPull) {
                glm::vec3 const pullDir = glm::normalize(_dragPosition - _scene.Anchor);
                _dragPosition = _scene.Anchor + pullDir * maxPull;
            }
            _dragPosition = ClampBirdPositionAboveGround(_dragPosition);
        }
        if (_dragging && leftReleased) {
            LaunchBird();
            _dragging = false;
        }
    }

    glm::vec3 CaseAngryBirds3D::GetLaunchPlaneForward() const {
        return glm::vec3(std::cos(_launchPlaneYaw), 0.f, std::sin(_launchPlaneYaw));
    }

    glm::vec3 CaseAngryBirds3D::GetLaunchPlaneNormal() const {
        glm::vec3 const up(0.f, 1.f, 0.f);
        return glm::normalize(glm::cross(up, GetLaunchPlaneForward()));
    }

    glm::vec3 CaseAngryBirds3D::ProjectOntoLaunchPlane(glm::vec3 position) const {
        glm::vec3 const anchor = _scene.Anchor;
        glm::vec3 const forward = GetLaunchPlaneForward();
        glm::vec3 const up(0.f, 1.f, 0.f);
        glm::vec3 const rel = position - anchor;
        return anchor + forward * glm::dot(rel, forward) + up * glm::dot(rel, up);
    }

    void CaseAngryBirds3D::UpdateLaunchPlaneRotation(float dt) {
        constexpr float kRotateSpeed = .2f;
        float delta = 0.f;
        if (ImGui::IsKeyDown(ImGuiKey_A)) delta -= kRotateSpeed * dt;
        if (ImGui::IsKeyDown(ImGuiKey_D)) delta += kRotateSpeed * dt;

        _launchPlaneYaw += delta;
        if (ImGui::IsKeyDown(ImGuiKey_W)) _launchPlaneYaw = 0.f;
        _launchPlaneYaw = std::clamp(_launchPlaneYaw, -_launchPlaneYawLimit, _launchPlaneYawLimit);
        if (_dragging) {
            _dragPosition = ProjectOntoLaunchPlane(_dragPosition);
            float const pullLen = glm::length(_dragPosition - _scene.Anchor);
            if (pullLen > _maxPull) {
                glm::vec3 const pullDir = glm::normalize(_dragPosition - _scene.Anchor);
                _dragPosition = _scene.Anchor + pullDir * _maxPull;
            }
            _dragPosition = ClampBirdPositionAboveGround(_dragPosition);
        }
    }

    glm::vec3 CaseAngryBirds3D::ScreenToLaunchPlane(ImVec2 const & mousePos) const {
        auto * window = ImGui::GetCurrentWindow();
        float const width = std::max(window->Rect().GetWidth(), 1.f);
        float const height = std::max(window->Rect().GetHeight(), 1.f);
        glm::vec2 const ndc(
            2.f * mousePos.x / width - 1.f,
            1.f - 2.f * mousePos.y / height);

        glm::mat4 const invVP = glm::inverse(_camera.GetProjectionMatrix(width / height) * _camera.GetViewMatrix());
        glm::vec4 nearPoint = invVP * glm::vec4(ndc, -1.f, 1.f);
        glm::vec4 farPoint = invVP * glm::vec4(ndc, 1.f, 1.f);
        nearPoint /= nearPoint.w;
        farPoint /= farPoint.w;

        glm::vec3 const rayOrigin = glm::vec3(nearPoint);
        glm::vec3 const rayDir = glm::normalize(glm::vec3(farPoint - nearPoint));
        glm::vec3 const planeNormal = GetLaunchPlaneNormal();
        float const denom = glm::dot(rayDir, planeNormal);
        if (std::abs(denom) < 1e-5f) return ProjectOntoLaunchPlane(_dragPosition);

        float const t = glm::dot(_scene.Anchor - rayOrigin, planeNormal) / denom;
        return ProjectOntoLaunchPlane(rayOrigin + rayDir * t);
    }

    void CaseAngryBirds3D::BuildStaticGeometry() {
        // Box geometry is built per-body in DrawBox so the index buffer is not needed.
    }

    void CaseAngryBirds3D::BuildSphereGeometry() {
        _sphereVertices.clear();
        int const rings = 10;
        int const segments = 18;
        for (int r = 0; r < rings; ++r) {
            float const v0 = float(r) / rings;
            float const v1 = float(r + 1) / rings;
            float const phi0 = v0 * std::numbers::pi_v<float>;
            float const phi1 = v1 * std::numbers::pi_v<float>;
            for (int s = 0; s < segments; ++s) {
                float const u0 = float(s) / segments;
                float const u1 = float(s + 1) / segments;
                float const th0 = u0 * std::numbers::pi_v<float> * 2.f;
                float const th1 = u1 * std::numbers::pi_v<float> * 2.f;

                auto point = [](float phi, float th) {
                    return glm::vec3(std::sin(phi) * std::cos(th), std::cos(phi), std::sin(phi) * std::sin(th));
                };
                glm::vec3 const p00 = point(phi0, th0);
                glm::vec3 const p01 = point(phi0, th1);
                glm::vec3 const p10 = point(phi1, th0);
                glm::vec3 const p11 = point(phi1, th1);
                _sphereVertices.insert(_sphereVertices.end(), { p00, p10, p11, p00, p11, p01 });
            }
        }
    }

    void CaseAngryBirds3D::DrawScene(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        _frame.Resize(desiredSize);

        auto const projection = _camera.GetProjectionMatrix(float(desiredSize.first) / float(desiredSize.second));
        auto const view = _camera.GetViewMatrix();

        auto passConstants = Rendering::SceneObject::PassConstants {
            .Projection           = projection,
            .View                 = view,
            .ViewPosition         = _camera.Eye,
            .AmbientIntensity     = glm::vec3(2.5f),
            .Lights               = {
                Rendering::SceneObject::Light {
                    .Intensity  = glm::vec3(.7f),
                    .Direction  = glm::normalize(glm::vec3(-0.25f, 1.f, 0.85f)),
                    .Position   = glm::vec3(0.f),
                    .CutOff     = 1.f,
                    .OuterCutOff= 0.f,
                }
            },
            .CntPointLights       = 0,
            .CntSpotLights        = 0,
            .CntDirectionalLights = 1,
        };
        _passConstantsBlock.Update(passConstants);

        _lineProgram.GetUniforms().SetByName("u_Projection", projection);
        _lineProgram.GetUniforms().SetByName("u_View", view);

        gl_using(_frame);
        DrawSky();

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LINE_SMOOTH);
        glLineWidth(1.2f);

        DrawScenery();

        constexpr float GroundRenderHalfExtent = 80.f;
        constexpr float GroundRenderThickness  = .06f;

        RigidBody ground;
        ground.Kind = BodyKind::Ground;
        ground.IsStatic = true;
        ground.Position = glm::vec3(3.f, -GroundRenderThickness, 0.f);
        ground.HalfSize = glm::vec3(GroundRenderHalfExtent, GroundRenderThickness, GroundRenderHalfExtent);
        ground.Color = glm::vec3(1.f);
        DrawBox(ground);

        for (auto const & body : _world.Rigid.Bodies) {
            if (!body.IsAlive) continue;
            if (body.Alpha < .999f) continue;
            if (body.Kind == BodyKind::Bird) {
                DrawSphere(body);
            } else {
                DrawBox(body);
            }
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        for (auto const & body : _world.Rigid.Bodies) {
            if (!body.IsAlive || body.Alpha >= .999f) continue;
            if (body.Kind == BodyKind::Bird) {
                DrawSphere(body);
            } else {
                DrawBox(body);
            }
        }
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        DrawFluid();

        // Slingshot rubber band — single continuous V-shaped ribbon (no seams)
        {
            float const stretch = _dragging ?
                glm::length(_dragPosition - _scene.Anchor) / _maxPull : 0.f;
            glm::vec3 const bandTarget = _springSnapping ? _springSnapPos :
                                         _dragging ? _dragPosition : _scene.Anchor;

            glm::vec3 const leftFork  = _scene.Anchor + glm::vec3(0.f, .85f, -.55f);
            glm::vec3 const rightFork = _scene.Anchor + glm::vec3(0.f, .85f, .55f);
            // Base: warm amber/tan rubber. Tension shifts R↑, G↓, B↓ toward red.
            DrawSlingshot(leftFork, rightFork, bandTarget, stretch, glm::vec3(.38f, .22f, .10f));
        }

        // Wooden posts
        glLineWidth(4.0f);
        DrawLine(_scene.Anchor + glm::vec3(0.f, .85f, -.55f), _scene.Anchor + glm::vec3(0.f, -.5f, -.55f), glm::vec3(.38f, .19f, .07f));
        DrawLine(_scene.Anchor + glm::vec3(0.f, .85f, .55f), _scene.Anchor + glm::vec3(0.f, -.5f, .55f), glm::vec3(.38f, .19f, .07f));
        glLineWidth(1.2f);
        if (_gameStarted && !_birdLaunched && !_birdMovingToSlingshot) {
            DrawLaunchPlaneGuide();
        }
        DrawTrajectoryPreview();

        glLineWidth(1.f);
        glPointSize(1.f);
        glDisable(GL_LINE_SMOOTH);
    }

    void CaseAngryBirds3D::DrawBox(RigidBody const & body) {
        static std::array<glm::vec3, 8> const signs = {
            glm::vec3(-1,  1,  1), glm::vec3( 1,  1,  1), glm::vec3( 1,  1, -1), glm::vec3(-1,  1, -1),
            glm::vec3(-1, -1,  1), glm::vec3( 1, -1,  1), glm::vec3( 1, -1, -1), glm::vec3(-1, -1, -1),
        };

        static std::array<std::array<int, 4>, 6> const faces = {
            std::array<int, 4>{0, 1, 2, 3},
            std::array<int, 4>{1, 5, 6, 2},
            std::array<int, 4>{5, 4, 7, 6},
            std::array<int, 4>{4, 0, 3, 7},
            std::array<int, 4>{3, 2, 6, 7},
            std::array<int, 4>{4, 5, 1, 0},
        };

        static std::array<glm::vec3, 6> const faceNormals = {
            glm::vec3(0,  1,  0), glm::vec3(1,  0,  0), glm::vec3(0, -1,  0),
            glm::vec3(-1, 0,  0), glm::vec3(0,  0, -1), glm::vec3(0,  0,  1),
        };

        static std::array<glm::vec2, 4> const baseUvs = {
            glm::vec2(0.f, 0.f), glm::vec2(1.f, 0.f), glm::vec2(1.f, 1.f), glm::vec2(0.f, 1.f),
        };

        std::array<glm::vec3, 8> corners;
        for (std::size_t i = 0; i < signs.size(); ++i) {
            corners[i] = body.Position + body.Rotation * (signs[i] * body.HalfSize * body.Scale);
        }

        std::vector<Vertex> vertices;
        vertices.reserve(36);
        float const uvScale = body.Kind == BodyKind::Ground ? body.HalfSize.x * .5f : 1.f;
        for (std::size_t face = 0; face < faces.size(); ++face) {
            auto const normal = glm::normalize(body.Rotation * faceNormals[face]);
            auto const & indices = faces[face];
            vertices.push_back(Vertex{ corners[indices[0]], normal, baseUvs[0] * uvScale, glm::vec3(0.f) });
            vertices.push_back(Vertex{ corners[indices[1]], normal, baseUvs[1] * uvScale, glm::vec3(0.f) });
            vertices.push_back(Vertex{ corners[indices[2]], normal, baseUvs[2] * uvScale, glm::vec3(0.f) });
            vertices.push_back(Vertex{ corners[indices[0]], normal, baseUvs[0] * uvScale, glm::vec3(0.f) });
            vertices.push_back(Vertex{ corners[indices[2]], normal, baseUvs[2] * uvScale, glm::vec3(0.f) });
            vertices.push_back(Vertex{ corners[indices[3]], normal, baseUvs[3] * uvScale, glm::vec3(0.f) });
        }

        bool const isTransparent = body.Alpha < .999f;
        _program.GetUniforms().SetByName("u_Color", isTransparent ? body.Color * 1.35f : body.Color);
        _program.GetUniforms().SetByName("u_Alpha", body.Alpha);
        _boxItem.UpdateVertexBuffer("vertex", Engine::make_span_bytes<Vertex>(vertices));
        auto const & diffuseTexture = body.Kind == BodyKind::Ground ? _groundTexture : _diffuseTexture;
        _boxItem.Draw({ diffuseTexture.Use(), _specularTexture.Use(), _heightTexture.Use(), _program.Use() });
    }

    void CaseAngryBirds3D::DrawSphere(RigidBody const & body) {
        std::vector<Vertex> vertices;
        vertices.reserve(_sphereVertices.size());
        for (auto const & p : _sphereVertices) {
            float const u = std::atan2(p.z, p.x) * (0.5f / std::numbers::pi_v<float>) + 0.5f;
            float const v = std::acos(glm::clamp(p.y, -1.f, 1.f)) * (1.f / std::numbers::pi_v<float>);
            glm::vec3 const rotated = body.Rotation * p;
            vertices.push_back(Vertex{ rotated * body.Radius * body.Scale, rotated, glm::vec2(u, v), body.Position });
        }

        _program.GetUniforms().SetByName("u_Color", body.Color);
        _program.GetUniforms().SetByName("u_Alpha", body.Alpha);
        _sphereItem.UpdateVertexBuffer("vertex", Engine::make_span_bytes<Vertex>(vertices));
        _sphereItem.Draw({ _diffuseTexture.Use(), _specularTexture.Use(), _heightTexture.Use(), _program.Use() }, vertices.size());
    }

    void CaseAngryBirds3D::DrawLine(glm::vec3 const & a, glm::vec3 const & b, glm::vec3 const & color) {
        std::vector<glm::vec3> verts = { a, b };
        _lineProgram.GetUniforms().SetByName("u_Color", color);
        _lineItem.UpdateVertexBuffer("position", Engine::make_span_bytes<glm::vec3>(verts));
        _lineItem.Draw({ _lineProgram.Use() });
    }

    void CaseAngryBirds3D::DrawLaunchPlaneGuide() {
        glm::vec3 const anchor = _scene.Anchor;
        glm::vec3 const forward = GetLaunchPlaneForward();
        glm::vec3 const up(0.f, 1.f, 0.f);
        glm::vec3 const guideColor(.35f, .75f, 1.f);

        DrawLine(anchor, anchor + forward * (_maxPull + .4f), guideColor);
        DrawLine(anchor, anchor + up * (_maxPull + .4f), guideColor);

        int const segments = 24;
        glm::vec3 prev = anchor + forward * _maxPull;
        for (int i = 1; i <= segments; ++i) {
            float const angle = float(i) / float(segments) * std::numbers::pi_v<float> * .5f;
            glm::vec3 const point = anchor + forward * (std::cos(angle) * _maxPull) + up * (std::sin(angle) * _maxPull);
            DrawLine(prev, point, glm::vec3(.25f, .55f, .85f));
            prev = point;
        }
    }

    void CaseAngryBirds3D::DrawTrajectoryPreview() {
        if (!_dragging) return;
        glm::vec3 pos = _dragPosition;
        float launchSpeedScale = 1.f;
        float birdMass = 1.4f;
        if (_birdIndex >= 0 && _birdIndex < int(_world.Rigid.Bodies.size())) {
            auto const & bird = _world.Rigid.Bodies[_birdIndex];
            if (bird.Kind == BodyKind::Bird && bird.Bird == BirdType::Speed) {
                launchSpeedScale = 1.5f;
            } else if (bird.Kind == BodyKind::Bird && bird.Bird == BirdType::Boomerang) {
                launchSpeedScale = 1.3f;
            } else if (bird.Kind == BodyKind::Bird && bird.Bird == BirdType::WaterBalloon) {
                launchSpeedScale = 0.85f;
            }
            if (bird.Mass > 0.01f) birdMass = bird.Mass;
        }

        // Spring energy: ½k·stretch² = ½m·v²  →  v = stretch·√(k/m)
        glm::vec3 pull = _scene.Anchor - _dragPosition;
        float const stretch = glm::length(pull);
        float const springSpeed = stretch * std::sqrt(_springConstant / birdMass);
        glm::vec3 const launchDir = stretch > 1e-4f ? pull / stretch : GetLaunchPlaneForward();
        glm::vec3 vel = launchDir * springSpeed * launchSpeedScale;
        glm::vec3 prev = pos;
        for (int i = 0; i < 32; ++i) {
            vel += _world.Rigid.Gravity * .07f;
            pos += vel * .07f;
            DrawLine(prev, pos, glm::vec3(1.f, .86f, .25f));
            prev = pos;
            if (pos.y < GroundY) break;
        }
    }

    void CaseAngryBirds3D::DrawSlingshot(
        glm::vec3 const & leftFork,
        glm::vec3 const & rightFork,
        glm::vec3 const & birdPos,
        float stretchRatio,
        glm::vec3 const & baseColor
    ) {
        constexpr int   kHalfSegments = 24;
        constexpr float kSagBase      = 0.32f;
        constexpr float kSagDecay     = 0.70f;
        constexpr glm::vec3 kWorldUp(0.f, 0.f, 1.f);
        constexpr glm::vec3 kBandNormal(0.f, 1.f, 0.f);

        float const vertSpanL = std::abs(leftFork.y - birdPos.y);
        float const vertSpanR = std::abs(rightFork.y - birdPos.y);
        float const rawSag    = kSagBase * std::max(1.0f - stretchRatio * kSagDecay, 0.06f);
        float const sagL      = std::min(rawSag, vertSpanL * 0.48f);
        float const sagR      = std::min(rawSag, vertSpanR * 0.48f);

        // Dynamic color: redder when tense
        float const tension = stretchRatio;
        glm::vec3 const bandColor = glm::vec3(
            baseColor.x + tension * 0.55f,
            baseColor.y * (1.0f - tension * 0.55f),
            baseColor.z * (1.0f - tension * 0.75f));

        // Ribbon half-width in world units
        float const halfWidth = (0.050f + (1.0f - stretchRatio) * 0.120f);

        // Build V-shaped curve: leftFork → birdPos → rightFork
        // Total points = 2*kHalfSegments + 1  (birdPos shared at midpoint)
        int const totalPts = kHalfSegments * 2 + 1;
        std::vector<glm::vec3> curvePts(totalPts);

        // Left half: leftFork → birdPos
        for (int i = 0; i <= kHalfSegments; ++i) {
            float const t = float(i) / float(kHalfSegments);
            float const catenary = std::sin(3.1415926535f * t);
            curvePts[i] = glm::mix(leftFork, birdPos, t)
                        - glm::vec3(0.f, sagL * catenary, 0.f);
        }
        // Right half: birdPos → rightFork (birdPos already at index kHalfSegments)
        for (int i = 0; i <= kHalfSegments; ++i) {
            float const t = float(i) / float(kHalfSegments);
            float const catenary = std::sin(3.1415926535f * t);
            curvePts[kHalfSegments + i] = glm::mix(birdPos, rightFork, t)
                                        - glm::vec3(0.f, sagR * catenary, 0.f);
        }

        // Compute right vectors for the entire V-curve
        std::vector<glm::vec3> rights(totalPts);
        for (int i = 0; i < totalPts; ++i) {
            glm::vec3 tang;
            if (i == 0)
                tang = glm::normalize(curvePts[1] - curvePts[0]);
            else if (i == totalPts - 1)
                tang = glm::normalize(curvePts[totalPts - 1] - curvePts[totalPts - 2]);
            else
                tang = glm::normalize(curvePts[i + 1] - curvePts[i - 1]);

            rights[i] = glm::normalize(glm::cross(tang, kWorldUp));
            if (glm::length(rights[i]) < 0.1f)
                rights[i] = glm::normalize(glm::cross(tang, glm::vec3(0.f, 1.f, 0.f)));
        }

        // Smooth right vectors across the apex (birdPos) so both halves blend
        // Use a sliding window average around the midpoint
        int const apex = kHalfSegments;
        int const blendRadius = 6;
        for (int i = apex - blendRadius; i <= apex + blendRadius; ++i) {
            if (i < 0 || i >= totalPts) continue;
            float const d = float(std::abs(i - apex)) / float(blendRadius);
            float const w = std::exp(-d * d * 3.0f);  // Gaussian falloff
            glm::vec3 const avg = glm::normalize(rights[apex]);
            rights[i] = glm::normalize(glm::mix(rights[i], avg, w * 0.7f));
        }

        // Build triangle strip vertices
        std::vector<Vertex> verts;
        verts.reserve(totalPts * 2);
        for (int i = 0; i < totalPts; ++i) {
            glm::vec3 const w = rights[i] * halfWidth;
            verts.push_back(Vertex{ curvePts[i] + w, kBandNormal, glm::vec2(0.f), glm::vec3(0.f) });
            verts.push_back(Vertex{ curvePts[i] - w, kBandNormal, glm::vec2(1.f), glm::vec3(0.f) });
        }

        // Emit triangles: two per segment, consistent CCW winding
        int const segs = totalPts - 1;
        std::vector<Vertex> triVerts;
        triVerts.reserve(segs * 6);
        for (int i = 0; i < segs; ++i) {
            int const i0 = i * 2;
            int const i1 = i0 + 1;
            int const i2 = i0 + 2;
            int const i3 = i0 + 3;
            triVerts.push_back(verts[i0]);
            triVerts.push_back(verts[i1]);
            triVerts.push_back(verts[i2]);
            triVerts.push_back(verts[i2]);
            triVerts.push_back(verts[i1]);
            triVerts.push_back(verts[i3]);
        }

        _program.GetUniforms().SetByName("u_Color", bandColor);
        _program.GetUniforms().SetByName("u_Alpha", 1.f);
        _boxItem.UpdateVertexBuffer("vertex", Engine::make_span_bytes<Vertex>(triVerts));
        _boxItem.Draw({ _diffuseTexture.Use(), _specularTexture.Use(), _heightTexture.Use(), _program.Use() });
    }

    void CaseAngryBirds3D::BurstWaterBalloon(int index) {
        if (index < 0 || index >= int(_world.Rigid.Bodies.size())) return;
        RigidBody & b = _world.Rigid.Bodies[index];
        glm::vec3 const pos = b.Position;
        glm::vec3 const vel = b.Velocity;
        b.IsAlive = false;

        if (_world.Fluid && _world.Fluid->InsideTankXZ(pos)) {
            int const count = 180;
            float const spread = 1.2f * WorldScale;
            for (int i = 0; i < count; ++i) {
                glm::vec3 const r(frand2(), frand2(), frand2());
                glm::vec3 const off = r * (b.Radius * 0.9f);
                glm::vec3 const pv = vel * 0.35f + r * spread;
                _world.Fluid->AddParticleWorld(pos + off, pv);
            }
        }
    }

    void CaseAngryBirds3D::UpdateScore() {
        int curTargets = CountAliveTargets(_world);
        int curBlocks = CountAliveBreakables(_world);
        if (curTargets < _prevTargets) _score += (_prevTargets - curTargets) * 500;
        if (curBlocks < _prevBlocks) _score += (_prevBlocks - curBlocks) * 100;
        _prevTargets = curTargets;
        _prevBlocks = curBlocks;
    }

    void CaseAngryBirds3D::DrawSky() {
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        _skyProgram.GetUniforms().SetByName("u_Top", glm::vec3(.33f, .55f, .85f));
        _skyProgram.GetUniforms().SetByName("u_Bottom", glm::vec3(.82f, .91f, .98f));
        _skyItem.Draw({ _skyProgram.Use() });
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
    }

    void CaseAngryBirds3D::DrawScenery() {
        auto sphere = [&](glm::vec3 pos, float r, glm::vec3 col) {
            RigidBody b;
            b.Position = pos * WorldScale;
            b.Radius = r * WorldScale;
            b.Color = col;
            b.Scale = 1.f;
            DrawSphere(b);
        };

        sphere({ -3.f, -4.0f, -13.f }, 9.5f, glm::vec3(.30f, .52f, .28f));
        sphere({ 13.f, -4.5f, -15.f }, 11.0f, glm::vec3(.25f, .47f, .24f));
        sphere({ 5.f, -5.0f, -20.f }, 13.0f, glm::vec3(.22f, .42f, .26f));
        sphere({ -8.f, 5.5f, -18.f }, 4.5f, glm::vec3(.95f, .97f, 1.f));
        sphere({ 2.f, 6.0f, -22.f }, 5.5f, glm::vec3(.92f, .95f, 1.f));
        sphere({ 10.f, 5.0f, -16.f }, 4.0f, glm::vec3(.94f, .96f, 1.f));
    }

    void CaseAngryBirds3D::DrawFluid() {
        if (!_world.Fluid) return;
        FluidWorld const & fluid = *_world.Fluid;

        int const n = fluid.ParticleCount();
        if (n > 0) {
            std::vector<FluidVertex> verts;
            verts.reserve(n);
            for (int i = 0; i < n; ++i) {
                verts.push_back(FluidVertex { fluid.ParticleWorld(i), fluid.Solver.m_particleColor[i] });
            }
            _pointProgram.GetUniforms().SetByName("u_Projection",
                _camera.GetProjectionMatrix(float(_frame.GetSize().first) / float(_frame.GetSize().second)));
            _pointProgram.GetUniforms().SetByName("u_View", _camera.GetViewMatrix());
            _pointProgram.GetUniforms().SetByName("u_PointRadius",
                fluid.Solver.m_particleRadius * std::max({ fluid.Size.x, fluid.Size.y, fluid.Size.z }) * 1.45f);
            _pointProgram.GetUniforms().SetByName("u_PointScale", float(_frame.GetSize().second));

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);
            glEnable(GL_PROGRAM_POINT_SIZE);
            _fluidItem.UpdateVertexBuffer("vertex", Engine::make_span_bytes<FluidVertex>(verts));
            _fluidItem.Draw({ _pointProgram.Use() });
            glDisable(GL_PROGRAM_POINT_SIZE);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        }

        DrawFluidSurface(fluid);

        glm::vec3 const lo = fluid.BoxMin();
        glm::vec3 const hi = fluid.BoxMax();
        glm::vec3 const edgeColor(.2f, .5f, .7f);
        glm::vec3 const v[8] = {
            { lo.x, lo.y, lo.z }, { hi.x, lo.y, lo.z }, { hi.x, lo.y, hi.z }, { lo.x, lo.y, hi.z },
            { lo.x, hi.y, lo.z }, { hi.x, hi.y, lo.z }, { hi.x, hi.y, hi.z }, { lo.x, hi.y, hi.z },
        };
        int const edges[12][2] = {
            {0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}
        };
        for (auto const & e : edges) {
            DrawLine(v[e[0]], v[e[1]], edgeColor);
        }
    }

    void CaseAngryBirds3D::DrawFluidSurface(FluidWorld const & fluid) {
        int const xCount = fluid.GridX();
        int const zCount = fluid.GridZ();
        if (xCount < 2 || zCount < 2 || fluid.SurfaceLocalY.size() < std::size_t(xCount * zCount)) return;

        float const minSurface = -0.5f + fluid.Solver.m_h * 1.35f;
        auto surface = [&](int x, int z) {
            return fluid.SurfaceLocalY[std::size_t(x * zCount + z)];
        };
        auto localX = [&](int x) {
            return (float(x) + 0.5f) * fluid.Solver.m_h - 0.5f;
        };
        auto localZ = [&](int z) {
            return (float(z) + 0.5f) * fluid.Solver.m_h - 0.5f;
        };
        auto point = [&](int x, int z, float y) {
            return fluid.LocalToWorld(glm::vec3(localX(x), y, localZ(z)));
        };

        std::vector<Vertex> vertices;
        vertices.reserve(std::size_t(xCount - 1) * std::size_t(zCount - 1) * 6);
        auto addTri = [&](glm::vec3 const & a, glm::vec3 const & b, glm::vec3 const & c) {
            glm::vec3 normal = glm::cross(b - a, c - a);
            if (glm::length(normal) < 1e-6f) normal = glm::vec3(0.f, 1.f, 0.f);
            else normal = glm::normalize(normal);
            if (normal.y < 0.f) normal = -normal;
            vertices.push_back(Vertex { a, normal, glm::vec2(0.f, 0.f), glm::vec3(0.f) });
            vertices.push_back(Vertex { b, normal, glm::vec2(1.f, 0.f), glm::vec3(0.f) });
            vertices.push_back(Vertex { c, normal, glm::vec2(1.f, 1.f), glm::vec3(0.f) });
        };

        for (int x = 0; x < xCount - 1; ++x) {
            for (int z = 0; z < zCount - 1; ++z) {
                float const h00 = surface(x, z);
                float const h10 = surface(x + 1, z);
                float const h11 = surface(x + 1, z + 1);
                float const h01 = surface(x, z + 1);
                if (h00 <= minSurface || h10 <= minSurface || h11 <= minSurface || h01 <= minSurface) continue;

                glm::vec3 const p00 = point(x, z, h00);
                glm::vec3 const p10 = point(x + 1, z, h10);
                glm::vec3 const p11 = point(x + 1, z + 1, h11);
                glm::vec3 const p01 = point(x, z + 1, h01);
                addTri(p00, p10, p11);
                addTri(p00, p11, p01);
            }
        }
        if (vertices.empty()) return;

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        _program.GetUniforms().SetByName("u_Color", glm::vec3(0.06f, 0.48f, 0.96f));
        _program.GetUniforms().SetByName("u_Alpha", 0.34f);
        _boxItem.UpdateVertexBuffer("vertex", Engine::make_span_bytes<Vertex>(vertices));
        _boxItem.Draw({ _diffuseTexture.Use(), _specularTexture.Use(), _heightTexture.Use(), _program.Use() });
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }

    void CaseAngryBirds3D::DrawHUD() {
        ImGuiViewport const * vp = ImGui::GetMainViewport();
        ImDrawList * dl = ImGui::GetForegroundDrawList();
        ImFont * font = ImGui::GetIO().Fonts->Fonts[0];

        float const cx = vp->Pos.x + vp->Size.x * 0.5f;
        float const top = vp->Pos.y + 14.f;

        std::string const scoreStr = "SCORE  " + std::to_string(_score);
        float const bigSize = 34.f;
        ImVec2 const ssz = font->CalcTextSizeA(bigSize, FLT_MAX, 0.f, scoreStr.c_str());

        int targetsLeft = CountAliveTargets(_world);
        std::string const subStr = "Targets " + std::to_string(targetsLeft) + "     Shots " + std::to_string(_shotsUsed);
        float const subSize = 18.f;
        ImVec2 const subsz = font->CalcTextSizeA(subSize, FLT_MAX, 0.f, subStr.c_str());

        // Auto-advance countdown line
        float advanceH = 0.f;
        ImVec2 advancesz(0.f, 0.f);
        std::string advanceStr;
        if (_autoAdvanceActive) {
            float const remaining = std::max(_autoAdvanceDelay - _autoAdvanceTimer, 0.f);
            int const nextLevel = (_levelIndex + 1) % 10;
            advanceStr = "Cleared! Next level in " + std::to_string(int(remaining * 10.f) / 10.f) + "s...";
            advancesz = font->CalcTextSizeA(subSize, FLT_MAX, 0.f, advanceStr.c_str());
            advanceH = subSize + 4.f;
        }

        float const boxW = std::max({ ssz.x, subsz.x, advancesz.x }) + 48.f;
        float const boxH = bigSize + subSize + advanceH + 24.f;
        ImVec2 const p0(cx - boxW * 0.5f, top);
        ImVec2 const p1(cx + boxW * 0.5f, top + boxH);
        dl->AddRectFilled(p0, p1, IM_COL32(18, 22, 30, 175), 10.f);
        dl->AddRect(p0, p1, IM_COL32(255, 255, 255, 45), 10.f);

        dl->AddText(font, bigSize, ImVec2(cx - ssz.x * 0.5f, top + 8.f), IM_COL32(255, 220, 80, 255), scoreStr.c_str());
        dl->AddText(font, subSize, ImVec2(cx - subsz.x * 0.5f, top + 8.f + bigSize + 4.f), IM_COL32(220, 230, 240, 230), subStr.c_str());
        if (_autoAdvanceActive) {
            dl->AddText(font, subSize, ImVec2(cx - advancesz.x * 0.5f, top + 8.f + bigSize + 4.f + subSize + 4.f), IM_COL32(100, 255, 130, 255), advanceStr.c_str());
        }
    }
} // namespace VCX::Labs::Final
