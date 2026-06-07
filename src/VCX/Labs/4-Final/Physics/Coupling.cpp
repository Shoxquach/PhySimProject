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

        // 该刚体是否为浮力/流体耦合对象 (排除碎片/静止体).
        bool IsCouplingBody(RigidBody const & b) {
            return b.IsAlive && !b.IsStatic && b.Kind != BodyKind::Fragment;
        }

        bool IsSphere(RigidBody const & b) {
            return b.Kind == BodyKind::Bird || b.Kind == BodyKind::WaterBalloon;
        }

        // 刚体的世界 AABB 半范围.
        glm::vec3 WorldExtent(RigidBody const & b) {
            if (IsSphere(b)) return glm::vec3(b.Radius);
            glm::mat3 const R = glm::mat3_cast(b.Rotation);
            glm::vec3 e(0.f);
            for (int r = 0; r < 3; r++)
                for (int c = 0; c < 3; c++)
                    e[r] += std::abs(R[c][r]) * b.HalfSize[c];
            return e;
        }

        // 若 worldP 在刚体内部返回 true, 给出表面点/外向法线.
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
            // 盒子: 转到刚体局部坐标后, 沿最浅的轴推出.
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

            // 刚体内部27个采样点中"在tank内 + 水面以下"的比例 = 浸没比例.
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

            // 浮力加速度 = -(rho_water/rho_body) * frac * Gravity (向上).
            b.Velocity -= rigid.Gravity * (ratio * frac) * dt;

            // 流动阻力: 被周围流体速度拖拽.
            //   静止水面(fv≈0) -> 类似阻尼, 流动的水 -> 推动并运送刚体.
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
            // 与 tank 不相交则跳过.
            if (aMax.x < boxMin.x || aMin.x > boxMax.x ||
                aMax.y < boxMin.y || aMin.y > boxMax.y ||
                aMax.z < boxMin.z || aMin.z > boxMax.z) continue;

            // 把刚体 AABB 转为网格单元范围.
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

                world = surf + n3 * 1e-3f;                 // 推到表面之外
                glm::vec3 bodyVel = BodyVelAt(b, surf);
                // 法向取刚体速度, 切向保留粒子速度 (防穿透 + 水花).
                float vn = glm::dot(worldVel, n3);
                float bn = glm::dot(bodyVel, n3);
                worldVel += (std::max(bn, vn) - vn) * n3;  // 去除向内侵入的分量 + 推出
                moved = true;
            }

            if (moved) {
                fluid.Solver.m_particlePos[p] = glm::clamp(fluid.WorldToLocal(world), glm::vec3(-0.49f), glm::vec3(0.49f));
                fluid.Solver.m_particleVel[p] = worldVel / fluid.Size;
            }
        }
    }
} // namespace VCX::Labs::Final
