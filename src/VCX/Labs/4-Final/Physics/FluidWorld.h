#pragma once

#include <algorithm>
#include <vector>

#include <glm/glm.hpp>

#include "Labs/4-Final/Config.h"
#include "Labs/4-Final/Physics/FluidSolver.h"

namespace VCX::Labs::Final {
    // FLIP 솔버(단위 정육면체 공간)를 월드 공간의 한 박스(tank)에 매핑하는 래퍼.
    //   world = Center + local * Size,  local in [-0.5, 0.5]^3
    class FluidWorld {
    public:
        FluidSolver Solver;
        glm::vec3   Center { 0.f };  // tank 중심 (월드)
        glm::vec3   Size   { 1.f };  // tank 크기 (월드)
        float       Density = 1.0f;  // 부력 계산용 기준 밀도 (블록 density 와 같은 단위)

        // 컬럼별 수면 local-y (부력용 height field). 크기 = X * Z.
        std::vector<float> SurfaceLocalY;

        void Init(int res, glm::vec3 center, glm::vec3 size, glm::vec3 relWater) {
            Center = center * WorldScale;   // tank 도 무대 배율로 확대
            Size   = size * WorldScale;
            Solver.setup(res, relWater);
            UpdateHeightField();
        }

        void Step(float dt) {
            Solver.SimulateTimestep(dt);
            UpdateHeightField();
        }

        int       ParticleCount() const { return int(Solver.m_particlePos.size()); }
        glm::vec3 ParticleWorld(int i) const { return Center + Solver.m_particlePos[i] * Size; }

        int GridX() const { return Solver.m_iCellX; }
        int GridY() const { return Solver.m_iCellY; }
        int GridZ() const { return Solver.m_iCellZ; }

        glm::vec3 BoxMin() const { return Center - Size * 0.5f; }
        glm::vec3 BoxMax() const { return Center + Size * 0.5f; }

        glm::vec3 LocalToWorld(glm::vec3 const & local) const { return Center + local * Size; }
        glm::vec3 WorldToLocal(glm::vec3 const & w) const { return (w - Center) / Size; }

        bool InsideTankXZ(glm::vec3 const & w) const {
            glm::vec3 l = WorldToLocal(w);
            return l.x >= -0.5f && l.x <= 0.5f && l.z >= -0.5f && l.z <= 0.5f;
        }

        // 셀 (i,j,k) 중심의 월드 좌표.
        glm::vec3 CellCenterWorld(int i, int j, int k) const {
            glm::vec3 local = (glm::vec3(i, j, k) + 0.5f) * Solver.m_h - 0.5f;
            return Center + local * Size;
        }

        // ---- two-way: 강체 -> 유체 ----
        void BeginDynamicSolids() { Solver.beginDynamicSolids(); }

        void MarkSolidCell(int i, int j, int k, glm::vec3 const & worldVel) {
            if (i < 0 || i >= Solver.m_iCellX || j < 0 || j >= Solver.m_iCellY || k < 0 || k >= Solver.m_iCellZ) return;
            int const id = Solver.cellId(i, j, k);
            Solver.m_s[id] = 0.0f;                  // solid 로 마킹 (물 못 지나감)
            // 경계 속도 (local 단위). 폭주 방지로 클램프.
            Solver.m_solidVel[id] = glm::clamp(worldVel / Size, glm::vec3(-8.f), glm::vec3(8.f));
        }

        // ---- 부력용 수면 질의 ----
        float SurfaceWorldY(float worldX, float worldZ) const {
            glm::vec3 l = WorldToLocal(glm::vec3(worldX, 0.f, worldZ));
            l.x = std::clamp(l.x, -0.5f, 0.5f);
            l.z = std::clamp(l.z, -0.5f, 0.5f);
            glm::ivec3 c = Solver.getParticleCell(glm::vec3(l.x, 0.f, l.z));
            int const Z = Solver.m_iCellZ;
            float weighted = 0.f;
            float weights = 0.f;
            for (int dx = -1; dx <= 1; ++dx) {
                for (int dz = -1; dz <= 1; ++dz) {
                    int const x = std::clamp(c.x + dx, 0, Solver.m_iCellX - 1);
                    int const z = std::clamp(c.z + dz, 0, Solver.m_iCellZ - 1);
                    int const col = x * Z + z;
                    if (col < 0 || col >= int(SurfaceLocalY.size()) || SurfaceLocalY[col] <= -0.5f) continue;
                    float const w = (dx == 0 && dz == 0) ? 4.f : ((dx == 0 || dz == 0) ? 2.f : 1.f);
                    weighted += SurfaceLocalY[col] * w;
                    weights += w;
                }
            }
            float ly = weights > 0.f ? weighted / weights : -0.5f;
            return Center.y + ly * Size.y;
        }

        // 월드 좌표에서의 유체 속도 (흐름 항력용). tank 밖이면 0.
        glm::vec3 FlowVelocityWorld(glm::vec3 const & worldP) const {
            glm::vec3 l = WorldToLocal(worldP);
            if (l.x < -0.5f || l.x > 0.5f || l.y < -0.5f || l.y > 0.5f || l.z < -0.5f || l.z > 0.5f)
                return glm::vec3(0.f);
            glm::vec3 v;
            for (int d = 0; d < 3; ++d) v[d] = Solver.sampleVelocity(Solver.m_vel, l, d);
            return v * Size;
        }

        // ---- 물풍선: 런타임 입자 추가 ----
        void AddParticleWorld(glm::vec3 const & worldPos, glm::vec3 const & worldVel) {
            if (int(Solver.m_particlePos.size()) >= 24000) return; // 폭주 방지 상한
            glm::vec3 local = WorldToLocal(worldPos);
            local = glm::clamp(local, glm::vec3(-0.49f), glm::vec3(0.49f));
            Solver.m_particlePos.push_back(local);
            Solver.m_particleVel.push_back(worldVel / Size);
            Solver.m_particleColor.push_back(glm::vec3(0.f, 0.4f, 1.f));
        }

    private:
        void UpdateHeightField() {
            int const X = Solver.m_iCellX;
            int const Z = Solver.m_iCellZ;
            std::vector<float> nextSurface(std::size_t(X) * Z, -0.5f);
            for (glm::vec3 const & p : Solver.m_particlePos) {
                glm::ivec3 c = Solver.getParticleCell(p);
                int const col = c.x * Z + c.z;
                if (p.y > nextSurface[col]) nextSurface[col] = p.y;
            }

            if (SurfaceLocalY.size() != nextSurface.size()) {
                SurfaceLocalY = nextSurface;
                return;
            }

            constexpr float SurfaceBlend = 0.28f;
            for (std::size_t i = 0; i < nextSurface.size(); ++i) {
                if (nextSurface[i] <= -0.5f || SurfaceLocalY[i] <= -0.5f) {
                    SurfaceLocalY[i] = nextSurface[i];
                } else {
                    SurfaceLocalY[i] = SurfaceLocalY[i] * (1.f - SurfaceBlend) + nextSurface[i] * SurfaceBlend;
                }
            }
        }
    };
} // namespace VCX::Labs::Final
