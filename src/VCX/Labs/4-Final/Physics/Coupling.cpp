#include "Labs/4-Final/Physics/Coupling.h"

#include <algorithm>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Labs/4-Final/Physics/FluidWorld.h"
#include "Labs/4-Final/Physics/PhysicsSystem.h"

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

        bool IsCouplingBody(RigidBody const & b) {
            return b.IsAlive && !b.IsStatic && b.Kind != BodyKind::Fragment;
        }

        bool IsSphere(RigidBody const & b) {
            return b.Kind == BodyKind::Bird;
        }

        glm::vec3 WorldExtent(RigidBody const & b) {
            if (IsSphere(b)) return glm::vec3(b.Radius * b.Scale);
            glm::mat3 const R = glm::mat3_cast(b.Rotation);
            glm::vec3 e(0.f);
            for (int r = 0; r < 3; r++)
                for (int c = 0; c < 3; c++)
                    e[r] += std::abs(R[c][r]) * b.HalfSize[c] * b.Scale;
            return e;
        }

        bool SurfaceIfInside(RigidBody const & b, glm::vec3 const & worldP, glm::vec3 & surfaceOut, glm::vec3 & normalOut) {
            if (IsSphere(b)) {
                glm::vec3 d = worldP - b.Position;
                float dist = glm::length(d);
                float const radius = b.Radius * b.Scale;
                if (dist >= radius) return false;
                glm::vec3 n = dist > 1e-6f ? d / dist : glm::vec3(0.f, 1.f, 0.f);
                surfaceOut = b.Position + n * radius;
                normalOut = n;
                return true;
            }
            glm::quat const invq = glm::inverse(b.Rotation);
            glm::vec3 pl = invq * (worldP - b.Position);
            glm::vec3 hs = b.HalfSize * b.Scale;
            if (std::abs(pl.x) >= hs.x || std::abs(pl.y) >= hs.y || std::abs(pl.z) >= hs.z)
                return false;

            float penX = hs.x - std::abs(pl.x);
            float penY = hs.y - std::abs(pl.y);
            float penZ = hs.z - std::abs(pl.z);
            int axis = 0;
            float pen = penX;
            if (penY < pen) { pen = penY; axis = 1; }
            if (penZ < pen) { pen = penZ; axis = 2; }

            glm::vec3 nLocal(0.f);
            float const sign = pl[axis] >= 0.f ? 1.f : -1.f;
            nLocal[axis] = sign;
            glm::vec3 surfLocal = pl;
            surfLocal[axis] = sign * hs[axis];

            surfaceOut = b.Position + b.Rotation * surfLocal;
            normalOut = b.Rotation * nLocal;
            return true;
        }
    }

    void Coupling::ApplyBuoyancy(FluidWorld const & fluid, AngryBirdsPhysics & rigid, float dt, int skipIndex) {
        for (int i = 0; i < int(rigid.Bodies.size()); ++i) {
            if (i == skipIndex) continue;
            RigidBody & b = rigid.Bodies[i];
            if (!IsCouplingBody(b)) continue;

            int in = 0, total = 0;
            for (int ix = -1; ix <= 1; ++ix)
                for (int iy = -1; iy <= 1; ++iy)
                    for (int iz = -1; iz <= 1; ++iz) {
                        glm::vec3 const s = glm::vec3(ix, iy, iz) * 0.6f;
                        glm::vec3 p = IsSphere(b) ? b.Position + s * b.Radius * b.Scale
                                                  : b.Position + b.Rotation * (s * b.HalfSize * b.Scale);
                        ++total;
                        if (fluid.InsideTankXZ(p) && p.y < fluid.SurfaceWorldY(p.x, p.z)) ++in;
                    }
            float const frac = total > 0 ? float(in) / float(total) : 0.f;
            if (frac <= 0.f) continue;

            float const vol = BodyVolume(b);
            float const bodyDensity = vol > 1e-6f ? b.Mass / vol : 1.f;
            float const ratio = fluid.Density / std::max(bodyDensity, 1e-4f);

            b.Velocity -= rigid.Gravity * (ratio * frac) * dt;

            glm::vec3 const fv = fluid.FlowVelocityWorld(b.Position);
            float const k = std::clamp(2.5f * frac * dt, 0.f, 0.7f);
            b.Velocity += (fv - b.Velocity) * k;

            float const angDrag = std::clamp(2.0f * frac * dt, 0.f, 0.8f);
            b.AngularVel *= (1.f - angDrag);
        }
    }

    void Coupling::MarkRigidSolids(AngryBirdsPhysics const & rigid, FluidWorld & fluid) {
        fluid.BeginDynamicSolids();

        glm::vec3 const boxMin = fluid.BoxMin();
        glm::vec3 const boxMax = fluid.BoxMax();

        for (RigidBody const & b : rigid.Bodies) {
            if (!b.IsAlive || b.Kind == BodyKind::Fragment) continue;

            glm::vec3 const ext = WorldExtent(b);
            glm::vec3 const aMin = b.Position - ext;
            glm::vec3 const aMax = b.Position + ext;
            if (aMax.x < boxMin.x || aMin.x > boxMax.x ||
                aMax.y < boxMin.y || aMin.y > boxMax.y ||
                aMax.z < boxMin.z || aMin.z > boxMax.z) continue;

            glm::vec3 lmin = fluid.WorldToLocal(aMin);
            glm::vec3 lmax = fluid.WorldToLocal(aMax);
            auto toCell = [](float l, int n) {
                return int(std::floor((l + 0.5f) * float(n - 1)));
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

    void Coupling::PushParticlesOutOfRigid(AngryBirdsPhysics const & rigid, FluidWorld & fluid) {
        int const n = fluid.ParticleCount();
        for (int p = 0; p < n; ++p) {
            glm::vec3 world = fluid.ParticleWorld(p);
            bool moved = false;
            glm::vec3 worldVel = fluid.Solver.m_particleVel[p] * fluid.Size;

            for (RigidBody const & b : rigid.Bodies) {
                if (!b.IsAlive || b.Kind == BodyKind::Fragment) continue;
                glm::vec3 surf, n3;
                if (!SurfaceIfInside(b, world, surf, n3)) continue;

                world = surf + n3 * 1e-3f;
                glm::vec3 bodyVel = BodyVelAt(b, surf);
                float vn = glm::dot(worldVel, n3);
                float bn = glm::dot(bodyVel, n3);
                worldVel += (std::max(bn, vn) - vn) * n3;
                moved = true;
            }

            if (moved) {
                fluid.Solver.m_particlePos[p] = glm::clamp(fluid.WorldToLocal(world), glm::vec3(-0.49f), glm::vec3(0.49f));
                fluid.Solver.m_particleVel[p] = worldVel / fluid.Size;
            }
        }
    }
}
