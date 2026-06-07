#include "Labs/4-Final/CaseAngryBirds3D.h"

#include <algorithm>
#include <cmath>
#include <numbers>

#include <glm/ext.hpp>
#include <imgui_internal.h>

#include "Engine/app.h"
#include "Engine/GL/Texture.hpp"
#include "Labs/Common/ImGuiHelper.h"

namespace VCX::Labs::Final {
    CaseAngryBirds3D::CaseAngryBirds3D():
        _program(Engine::GL::UniqueProgram({ Engine::GL::SharedShader("assets/shaders/sphere_phong.vert"),
                                             Engine::GL::SharedShader("assets/shaders/phong.frag") })),
        _lineProgram(Engine::GL::UniqueProgram({ Engine::GL::SharedShader("assets/shaders/flat.vert"),
                                                 Engine::GL::SharedShader("assets/shaders/flat.frag") })),
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
        _passConstantsBlock(1, Engine::GL::DrawFrequency::Stream) {
        BuildStaticGeometry();
        BuildSphereGeometry();

        VCX::Engine::Texture2D<VCX::Engine::Formats::RGBA8> diffuse{1, 1};
        diffuse.Fill({ 0xff, 0xff, 0xff, 0xff });
        _diffuseTexture = Engine::GL::UniqueTexture2D(diffuse, 0);

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

        if (ImGui::Button("Reset Camera", ImVec2(250, 0))) {
            _cameraManager.Reset(_camera);
        }

        ImGui::Checkbox("Pause", &_pause);
        ImGui::SliderFloat("Launch Power", &_powerScale, 2.f, 12.f, "%.1f");
        ImGui::SliderFloat("Break Threshold", &_breakThreshold, 2.f, 18.f, "%.1f");
        ImGui::SliderFloat("Restitution", &_physics.Restitution, .05f, .8f, "%.2f");
        ImGui::SliderFloat("Friction", &_physics.Friction, .2f, .98f, "%.2f");
        ImGui::SliderInt("Substeps", &_substeps, 1, 12);

        int aliveBreakables = 0;
        int aliveTargets = 0;
        for (auto const & body : _physics.Bodies) {
            if (body.IsAlive && body.Breakable && body.Kind != BodyKind::Bird) {
                aliveBreakables++;
            }
            if (body.IsAlive && body.Kind == BodyKind::Target) {
                aliveTargets++;
            }
        }

        ImGui::Spacing();
        if (!_gameStarted) {
            ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Press S to START");
        } else {
            ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Game Started - Press S to stop");
            ImGui::Text("Drag the red bird with left mouse.");
            ImGui::Text("Release to launch. Right mouse rotates camera.");
        }
        ImGui::Text("Press R to reset level.");
        ImGui::Text("Alive target blocks: %d", aliveTargets);
        ImGui::Text("Alive breakable blocks: %d", aliveBreakables);
        ImGui::Text("Fragments created: %d", _physics.FragmentsCreated);
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

        float const frameDt = std::min(Engine::GetDeltaTime(), 1.f / 30.f);
        if (!_pause) {
            int const steps = std::max(_substeps, 1);
            for (int i = 0; i < steps; ++i) {
                StepSimulation(frameDt / float(steps));
            }
        }

        _cameraManager.Update(_camera);
        DrawScene(desiredSize);

        return Common::CaseRenderResult {
            .Fixed     = false,
            .Flipped   = true,
            .Image     = _frame.GetColorAttachment(),
            .ImageSize = desiredSize,
        };
    }

    void CaseAngryBirds3D::OnProcessInput(ImVec2 const & pos) {
        if (_gameStarted) {
            HandleSlingshotInput(pos);
        } else {
            _cameraManager.ProcessInput(_camera, pos);
        }
    }

    void CaseAngryBirds3D::ResetScene() {
        _dragging = false;
        _birdLaunched = false;
        _dragPosition = _scene.Anchor;
        _birdIndex = _scene.Reset(_physics, _breakThreshold, static_cast<AngryBirdsScene::Level>(_levelIndex));
        _gameStarted = false;
    }

    void CaseAngryBirds3D::StepSimulation(float dt) {
        _physics.Step(dt, _dragging ? _birdIndex : -1, _dragPosition);

        _birdIndex = -1;
        for (int i = 0; i < int(_physics.Bodies.size()); ++i) {
            if (_physics.Bodies[i].Kind == BodyKind::Bird) {
                _birdIndex = i;
                break;
            }
        }
        if (_birdIndex < 0 && !_birdLaunched) {
            _birdIndex = _physics.AddBird(_scene.Anchor);
        }
    }

    void CaseAngryBirds3D::LaunchBird() {
        if (_birdIndex < 0 || _birdIndex >= int(_physics.Bodies.size())) return;
        auto & bird = _physics.Bodies[_birdIndex];
        glm::vec3 pull = _scene.Anchor - _dragPosition;
        pull.z = 0.f;
        bird.Position = _dragPosition;
        bird.Velocity = pull * _powerScale;
        bird.AngularVel = glm::vec3(0.f, 0.f, -glm::length(pull) * 8.f);
        bird.LifeTime = 20.f;
        _birdLaunched = true;
        _gameStarted = false;
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

        if (!_birdLaunched && hover && leftClicked && !io.KeyCtrl && !io.KeyShift && !io.KeyAlt) {
            _dragging = true;
        }
        if (_dragging && leftHeld) {
            glm::vec3 const planePos = ScreenToLaunchPlane(mousePos);
            _dragPosition = _scene.Anchor;
            _dragPosition.x = planePos.x;
            _dragPosition.y = planePos.y;

            float const maxPull = 2.2f;
            glm::vec2 const pullXY = glm::vec2(_dragPosition.x - _scene.Anchor.x, _dragPosition.y - _scene.Anchor.y);
            float const pullLen = glm::length(pullXY);
            if (pullLen > maxPull) {
                glm::vec2 const pullDir = glm::normalize(pullXY);
                _dragPosition.x = _scene.Anchor.x + pullDir.x * maxPull;
                _dragPosition.y = _scene.Anchor.y + pullDir.y * maxPull;
            }
        }
        if (_dragging && leftReleased) {
            LaunchBird();
            _dragging = false;
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
        glm::vec3 const planeNormal = glm::vec3(0.f, 0.f, 1.f);
        float const denom = glm::dot(rayDir, planeNormal);
        if (std::abs(denom) < 1e-5f) return _dragPosition;

        float const t = (_scene.Anchor.z - rayOrigin.z) / rayDir.z;
        return rayOrigin + rayDir * t;
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
                    .Intensity  = glm::vec3(.55f),
                    .Direction  = glm::normalize(glm::vec3(0.f, 1.f, 0.2f)),
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
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LINE_SMOOTH);
        glLineWidth(1.2f);

        RigidBody ground;
        ground.Kind = BodyKind::Ground;
        ground.IsStatic = true;
        ground.Position = glm::vec3(1.5f, -.06f, 0.f);
        ground.HalfSize = glm::vec3(10.f, .06f, 4.f);
        ground.Color = glm::vec3(.24f, .46f, .22f);
        DrawBox(ground);

        for (auto const & body : _physics.Bodies) {
            if (!body.IsAlive) continue;
            if (body.Kind == BodyKind::Bird) {
                DrawSphere(body);
            } else {
                DrawBox(body);
            }
        }

        DrawLine(_scene.Anchor + glm::vec3(0.f, .85f, -.55f), _dragging ? _dragPosition : _scene.Anchor, glm::vec3(.1f, .05f, .02f));
        DrawLine(_scene.Anchor + glm::vec3(0.f, .85f, .55f), _dragging ? _dragPosition : _scene.Anchor, glm::vec3(.1f, .05f, .02f));
        DrawLine(_scene.Anchor + glm::vec3(0.f, .85f, -.55f), _scene.Anchor + glm::vec3(0.f, -.5f, -.55f), glm::vec3(.32f, .16f, .06f));
        DrawLine(_scene.Anchor + glm::vec3(0.f, .85f, .55f), _scene.Anchor + glm::vec3(0.f, -.5f, .55f), glm::vec3(.32f, .16f, .06f));
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
            glm::vec3(0,  1,  0), glm::vec3(1,  0,  0), glm::vec3(0,  0,  1),
            glm::vec3(-1, 0,  0), glm::vec3(0,  0, -1), glm::vec3(0, -1,  0),
        };

        static std::array<glm::vec2, 4> const uvs = {
            glm::vec2(0.f, 0.f), glm::vec2(1.f, 0.f), glm::vec2(1.f, 1.f), glm::vec2(0.f, 1.f),
        };

        std::array<glm::vec3, 8> corners;
        for (std::size_t i = 0; i < signs.size(); ++i) {
            corners[i] = body.Position + body.Rotation * (signs[i] * body.HalfSize * body.Scale);
        }

        std::vector<Vertex> vertices;
        vertices.reserve(36);
        for (std::size_t face = 0; face < faces.size(); ++face) {
            auto const normal = glm::normalize(body.Rotation * faceNormals[face]);
            auto const & indices = faces[face];
            vertices.push_back(Vertex{ corners[indices[0]], normal, uvs[0], glm::vec3(0.f) });
            vertices.push_back(Vertex{ corners[indices[1]], normal, uvs[1], glm::vec3(0.f) });
            vertices.push_back(Vertex{ corners[indices[2]], normal, uvs[2], glm::vec3(0.f) });
            vertices.push_back(Vertex{ corners[indices[0]], normal, uvs[0], glm::vec3(0.f) });
            vertices.push_back(Vertex{ corners[indices[2]], normal, uvs[2], glm::vec3(0.f) });
            vertices.push_back(Vertex{ corners[indices[3]], normal, uvs[3], glm::vec3(0.f) });
        }

        _program.GetUniforms().SetByName("u_Color", body.Color);
        _boxItem.UpdateVertexBuffer("vertex", Engine::make_span_bytes<Vertex>(vertices));
        _boxItem.Draw({ _diffuseTexture.Use(), _specularTexture.Use(), _heightTexture.Use(), _program.Use() });
    }

    void CaseAngryBirds3D::DrawSphere(RigidBody const & body) {
        std::vector<Vertex> vertices;
        vertices.reserve(_sphereVertices.size());
        for (auto const & p : _sphereVertices) {
            float const u = std::atan2(p.z, p.x) * (0.5f / std::numbers::pi_v<float>) + 0.5f;
            float const v = std::acos(glm::clamp(p.y, -1.f, 1.f)) * (1.f / std::numbers::pi_v<float>);
            vertices.push_back(Vertex{ p * body.Radius * body.Scale, p, glm::vec2(u, v), body.Position });
        }

        _program.GetUniforms().SetByName("u_Color", body.Color);
        _sphereItem.UpdateVertexBuffer("vertex", Engine::make_span_bytes<Vertex>(vertices));
        _sphereItem.Draw({ _diffuseTexture.Use(), _specularTexture.Use(), _heightTexture.Use(), _program.Use() }, vertices.size());
    }

    void CaseAngryBirds3D::DrawLine(glm::vec3 const & a, glm::vec3 const & b, glm::vec3 const & color) {
        std::vector<glm::vec3> verts = { a, b };
        _lineProgram.GetUniforms().SetByName("u_Color", color);
        _lineItem.UpdateVertexBuffer("position", Engine::make_span_bytes<glm::vec3>(verts));
        _lineItem.Draw({ _lineProgram.Use() });
    }

    void CaseAngryBirds3D::DrawTrajectoryPreview() {
        if (!_dragging) return;
        glm::vec3 pos = _dragPosition;
        glm::vec3 vel = (_scene.Anchor - _dragPosition) * _powerScale;
        glm::vec3 prev = pos;
        for (int i = 0; i < 32; ++i) {
            vel += _physics.Gravity * .07f;
            pos += vel * .07f;
            DrawLine(prev, pos, glm::vec3(1.f, .86f, .25f));
            prev = pos;
            if (pos.y < GroundY) break;
        }
    }
} // namespace VCX::Labs::Final
