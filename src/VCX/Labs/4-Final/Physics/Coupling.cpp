#include "Labs/4-Final/Physics/Coupling.h"

#include <algorithm>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Labs/4-Final/Physics/FluidWorld.h"
#include "Labs/4-Final/Physics/RigidWorld.h"

namespace VCX::Labs::Final {
    namespace {
        constexpr float c_Pi = 3.14159265358979323846f;

        float BodyVolume(RigidBody const & b) {
            if (b.Kind == BodyKind::Bird) {
                return 4.f / 3.f * c_Pi * b.Radius * b.Radius * b.Radius;
            }
            return 8.f * b.HalfSize.x * b.HalfSize.y * b.HalfSize.z;
        }

        glm::vec3 BodyVelAt(RigidBody const & b, glm::vec3 const & worldP) {
            return b.Velocity + glm::cross(b.AngularVel, worldP - b.Position);
        }

        // 강체가 부력/유체 결합 대상인지 (파편/정지체 제외).
        bool IsCouplingBody(RigidBody const & b) {
            return b.IsAlive && !b.IsStatic && b.Kind != BodyKind::Fragment;
        }

        bool IsSphere(RigidBody const & b) {
            return b.Kind == BodyKind::Bird || b.Kind == BodyKind::WaterBalloon;
        }

        // 강체의 월드 AABB 반-범위.
        glm::vec3 WorldExtent(RigidBody const & b) {
            if (IsSphere(b)) return glm::vec3(b.Radius);
            glm::mat3 const R = glm::mat3_cast(b.Rotation);
            glm::vec3 e(0.f);
            for (int r = 0; r < 3; r++)
                for (int c = 0; c < 3; c++)
                    e[r] += std::abs(R[c][r]) * b.HalfSize[c];
            return e;
        }

        // worldP 가 강체 내부면 true, 표면점/외향 법선을 돌려준다.
        bool SurfaceIfInside(RigidBody const & b, glm::vec3 const & worldP, glm::vec3 & surfaceOut, glm::vec3 & normalOut) {
            if (IsSphere(b)) {
                glm::vec3 d = worldP - b.Position;
                float dist = glm::length(d);
                if (dist >= b.Radius) return false;
                glm::vec3 n = dist > 1e-6f ? d / dist : glm::vec3(0.f, 1.f, 0.f);
                surfaceOut = b.Position + n * b.Radius;
                normalOut = n;
                return true;
            }
            // 박스: 바디 로컬로 변환 후 가장 얕은 축으로 밀어낸다.
            glm::quat const invq = glm::inverse(b.Rotation);
            glm::vec3 pl = invq * (worldP - b.Position);
            if (std::abs(pl.x) >= b.HalfSize.x || std::abs(pl.y) >= b.HalfSize.y || std::abs(pl.z) >= b.HalfSize.z)
                return false;

            float penX = b.HalfSize.x - std::abs(pl.x);
            float penY = b.HalfSize.y - std::abs(pl.y);
            float penZ = b.HalfSize.z - std::abs(pl.z);
            int axis = 0;
            float pen = penX;
            if (penY < pen) { pen = penY; axis = 1; }
            if (penZ < pen) { pen = penZ; axis = 2; }

            glm::vec3 nLocal(0.f);
            float const sign = pl[axis] >= 0.f ? 1.f : -1.f;
            nLocal[axis] = sign;
            glm::vec3 surfLocal = pl;
            surfLocal[axis] = sign * b.HalfSize[axis];

            surfaceOut = b.Position + b.Rotation * surfLocal;
            normalOut = b.Rotation * nLocal;
            return true;
        }
    }

    void Coupling::ApplyBuoyancy(FluidWorld const & fluid, RigidWorld & rigid, float dt, int skipIndex) {
        for (int i = 0; i < int(rigid.Bodies.size()); ++i) {
            if (i == skipIndex) continue;
            RigidBody & b = rigid.Bodies[i];
            if (!IsCouplingBody(b)) continue;

            // 바디 내부 27개 표본점 중 "tank 안 + 수면 아래" 비율 = 잠긴 비율.
            int in = 0, total = 0;
            for (int ix = -1; ix <= 1; ++ix)
                for (int iy = -1; iy <= 1; ++iy)
                    for (int iz = -1; iz <= 1; ++iz) {
                        glm::vec3 const s = glm::vec3(ix, iy, iz) * 0.6f;
                        glm::vec3 p = IsSphere(b) ? b.Position + s * b.Radius
                                                  : b.Position + b.Rotation * (s * b.HalfSize);
                        ++total;
                        if (fluid.InsideTankXZ(p) && p.y < fluid.SurfaceWorldY(p.x, p.z)) ++in;
                    }
            float const frac = total > 0 ? float(in) / float(total) : 0.f;
            if (frac <= 0.f) continue;

            float const vol = BodyVolume(b);
            float const bodyDensity = vol > 1e-6f ? b.Mass / vol : 1.f;
            float const ratio = fluid.Density / std::max(bodyDensity, 1e-4f);

            // 부력 가속도 = -(rho_water/rho_body) * frac * Gravity (위로).
            b.Velocity -= rigid.Gravity * (ratio * frac) * dt;

            // 흐름 항력: 주변 유체 속도로 끌림.
            //   정지 수면(fv≈0) -> 감쇠처럼 작동, 흐르는 물 -> 강체를 떠밀어 운반.
            glm::vec3 const fv = fluid.FlowVelocityWorld(b.Position);
            float const k = std::clamp(2.5f * frac * dt, 0.f, 0.7f);
            b.Velocity += (fv - b.Velocity) * k;

            float const angDrag = std::clamp(2.0f * frac * dt, 0.f, 0.8f);
            b.AngularVel *= (1.f - angDrag);
        }
    }

    void Coupling::MarkRigidSolids(RigidWorld const & rigid, FluidWorld & fluid) {
        fluid.BeginDynamicSolids();

        glm::vec3 const boxMin = fluid.BoxMin();
        glm::vec3 const boxMax = fluid.BoxMax();

        for (RigidBody const & b : rigid.Bodies) {
            if (!b.IsAlive || b.Kind == BodyKind::Fragment) continue;

            glm::vec3 const ext = WorldExtent(b);
            glm::vec3 const aMin = b.Position - ext;
            glm::vec3 const aMax = b.Position + ext;
            // tank 와 겹치지 않으면 skip.
            if (aMax.x < boxMin.x || aMin.x > boxMax.x ||
                aMax.y < boxMin.y || aMin.y > boxMax.y ||
                aMax.z < boxMin.z || aMin.z > boxMax.z) continue;

            // 바디 AABB 를 grid 셀 범위로.
            glm::vec3 lmin = fluid.WorldToLocal(aMin);
            glm::vec3 lmax = fluid.WorldToLocal(aMax);
            auto toCell = [](float l, int n) {
                int c = int(std::floor((l + 0.5f) * float(n - 1)));
                return c;
            };
            int i0 = std::max(0, toCell(std::min(lmin.x, lmax.x), fluid.GridX()));
            int i1 = std::min(fluid.GridX() - 1, toCell(std::max(lmin.x, lmax.x), fluid.GridX()) + 1);
            int j0 = std::max(0, toCell(std::min(lmin.y, lmax.y), fluid.GridY()));
            int j1 = std::min(fluid.GridY() - 1, toCell(std::max(lmin.y, lmax.y), fluid.GridY()) + 1);
            int k0 = std::max(0, toCell(std::min(lmin.z, lmax.z), fluid.GridZ()));
            int k1 = std::min(fluid.GridZ() - 1, toCell(std::max(lmin.z, lmax.z), fluid.GridZ()) + 1);

            for (int i = i0; i <= i1; ++i)
                for (int j = j0; j <= j1; ++j)
                    for (int k = k0; k <= k1; ++k) {
                        glm::vec3 const cc = fluid.CellCenterWorld(i, j, k);
                        glm::vec3 surf, nrm;
                        if (SurfaceIfInside(b, cc, surf, nrm)) {
                            fluid.MarkSolidCell(i, j, k, BodyVelAt(b, cc));
                        }
                    }
        }
    }

    void Coupling::PushParticlesOutOfRigid(RigidWorld const & rigid, FluidWorld & fluid) {
        int const n = fluid.ParticleCount();
        for (int p = 0; p < n; ++p) {
            glm::vec3 world = fluid.ParticleWorld(p);
            bool moved = false;
            glm::vec3 worldVel = fluid.Solver.m_particleVel[p] * fluid.Size; // local -> world

            for (RigidBody const & b : rigid.Bodies) {
                if (!b.IsAlive || b.Kind == BodyKind::Fragment) continue;
                glm::vec3 surf, n3;
                if (!SurfaceIfInside(b, world, surf, n3)) continue;

                world = surf + n3 * 1e-3f;                 // 표면 밖으로
                glm::vec3 bodyVel = BodyVelAt(b, surf);
                // 법선 성분은 강체 속도, 접선 성분은 입자 속도 유지 (관통 방지 + 물보라).
                float vn = glm::dot(worldVel, n3);
                float bn = glm::dot(bodyVel, n3);
                worldVel += (std::max(bn, vn) - vn) * n3;  // 안쪽으로 파고드는 성분 제거 + 밀어줌
                moved = true;
            }

            if (moved) {
                fluid.Solver.m_particlePos[p] = glm::clamp(fluid.WorldToLocal(world), glm::vec3(-0.49f), glm::vec3(0.49f));
                fluid.Solver.m_particleVel[p] = worldVel / fluid.Size;
            }
        }
    }
} // namespace VCX::Labs::Final
