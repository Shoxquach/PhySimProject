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

            float const vol         = BodyVolume(b);
            float const bodyDensity = vol > 1e-6f ? b.Mass / vol : 1.f;
            // Only apply buoyancy to objects lighter than water
            if (bodyDensity >= fluid.Density) continue;

            // Check if body is inside the tank (at least partially)
            if (!fluid.InsideTankXZ(b.Position)) continue;
            float const surfaceY = fluid.SurfaceWorldY(b.Position.x, b.Position.z);
            // Body entirely above water surface → no buoyancy
            float const bodyBottom = IsSphere(b) ? b.Position.y - b.Radius * b.Scale
                                                 : b.Position.y - b.HalfSize.y * b.Scale;
            if (bodyBottom >= surfaceY) continue;

            // ---- Analytical equilibrium buoyancy (spring-damper) ----
            float const bodyHeight  = IsSphere(b) ? 2.f * b.Radius * b.Scale
                                                  : 2.f * b.HalfSize.y * b.Scale;
            float const fracEq      = bodyDensity / fluid.Density; // submerged fraction at rest
            float const targetY     = surfaceY - bodyHeight * (fracEq - 0.5f);

            // Critically-damped harmonic oscillator: d = 2·√(k·m)
            float const springK     = 80.f;
            float const dampingC    = 2.f * std::sqrt(springK * b.Mass);
            float const displacement = targetY - b.Position.y;
            float const springForce  = springK * displacement;
            float const dampingForce = dampingC * b.Velocity.y;
            b.Velocity.y += (springForce - dampingForce) * b.InvMass * dt;

            // Lateral drag in water (keep object from drifting horizontally)
            float const lateralDrag = std::clamp(4.f * fracEq * dt, 0.f, 0.55f);
            b.Velocity.x *= (1.f - lateralDrag);
            b.Velocity.z *= (1.f - lateralDrag);

            // Soft containment near tank edges
            float const marginX = (fluid.BoxMax().x - fluid.BoxMin().x) * 0.08f;
            float const marginZ = (fluid.BoxMax().z - fluid.BoxMin().z) * 0.08f;
            float const edgeK   = 6.f;
            if (b.Position.x < fluid.BoxMin().x + marginX)
                b.Velocity.x += edgeK * (fluid.BoxMin().x + marginX - b.Position.x) * dt;
            if (b.Position.x > fluid.BoxMax().x - marginX)
                b.Velocity.x -= edgeK * (b.Position.x - fluid.BoxMax().x + marginX) * dt;
            if (b.Position.z < fluid.BoxMin().z + marginZ)
                b.Velocity.z += edgeK * (fluid.BoxMin().z + marginZ - b.Position.z) * dt;
            if (b.Position.z > fluid.BoxMax().z - marginZ)
                b.Velocity.z -= edgeK * (b.Position.z - fluid.BoxMax().z + marginZ) * dt;

            // Angular drag
            float const angDrag = std::clamp(5.f * fracEq * dt, 0.f, 0.9f);
            b.AngularVel *= (1.f - angDrag);
        }
    }

    void Coupling::MarkRigidSolids(AngryBirdsPhysics const & rigid, FluidWorld & fluid) {
        fluid.BeginDynamicSolids();

        glm::vec3 const boxMin = fluid.BoxMin();
        glm::vec3 const boxMax = fluid.BoxMax();

        for (RigidBody const & b : rigid.Bodies) {
            if (!b.IsAlive || b.Kind == BodyKind::Fragment) continue;

            // Skip floating bodies (density < fluid): they don't block water,
            // preventing pressure artifacts that push them out of the tank.
            float const vol         = BodyVolume(b);
            float const bodyDensity = vol > 1e-6f ? b.Mass / vol : 1.f;
            if (bodyDensity < fluid.Density * 0.98f) continue;

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
