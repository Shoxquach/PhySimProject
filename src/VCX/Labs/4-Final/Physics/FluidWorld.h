#pragma once

#include <algorithm>
#include <vector>

#include <glm/glm.hpp>

#include "Labs/4-Final/Config.h"
#include "Labs/4-Final/Physics/FluidSolver.h"

namespace VCX::Labs::Final {
    // 把 FLIP 求解器(单位立方体空间)映射到世界空间一个盒子(tank)的封装.
    //   world = Center + local * Size,  local in [-0.5, 0.5]^3
    class FluidWorld {
    public:
        FluidSolver Solver;
        glm::vec3   Center { 0.f };  // tank 中心 (世界)
        glm::vec3   Size   { 1.f };  // tank 大小 (世界)
        float       Density = 1.0f;  // 浮力计算用的基准密度 (与方块 density 同单位)

        // 每列水面 local-y (浮力用 height field). 大小 = X * Z.
        std::vector<float> SurfaceLocalY;

        void Init(int res, glm::vec3 center, glm::vec3 size, glm::vec3 relWater) {
            Center = center * WorldScale;   // tank 也按舞台缩放放大
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

        // 单元 (i,j,k) 中心的世界坐标.
        glm::vec3 CellCenterWorld(int i, int j, int k) const {
            glm::vec3 local = (glm::vec3(i, j, k) + 0.5f) * Solver.m_h - 0.5f;
            return Center + local * Size;
        }

        // ---- two-way: 刚体 -> 流体 ----
        void BeginDynamicSolids() { Solver.beginDynamicSolids(); }

        void MarkSolidCell(int i, int j, int k, glm::vec3 const & worldVel) {
            if (i < 0 || i >= Solver.m_iCellX || j < 0 || j >= Solver.m_iCellY || k < 0 || k >= Solver.m_iCellZ) return;
            int const id = Solver.cellId(i, j, k);
            Solver.m_s[id] = 0.0f;                  // 标记为 solid (水无法通过)
            // 边界速度 (local 单位). 为防发散做 clamp.
            Solver.m_solidVel[id] = glm::clamp(worldVel / Size, glm::vec3(-8.f), glm::vec3(8.f));
        }

        // ---- 浮力用的水面查询 ----
        float SurfaceWorldY(float worldX, float worldZ) const {
            glm::vec3 l = WorldToLocal(glm::vec3(worldX, 0.f, worldZ));
            l.x = std::clamp(l.x, -0.5f, 0.5f);
            l.z = std::clamp(l.z, -0.5f, 0.5f);
            glm::ivec3 c = Solver.getParticleCell(glm::vec3(l.x, 0.f, l.z));
            int const Z = Solver.m_iCellZ;
            int const col = c.x * Z + c.z;
            float ly = -0.5f;
            if (col >= 0 && col < int(SurfaceLocalY.size())) ly = SurfaceLocalY[col];
            return Center.y + ly * Size.y;
        }

        // 世界坐标处的流体速度 (流动阻力用). tank 外则为0.
        glm::vec3 FlowVelocityWorld(glm::vec3 const & worldP) const {
            glm::vec3 l = WorldToLocal(worldP);
            if (l.x < -0.5f || l.x > 0.5f || l.y < -0.5f || l.y > 0.5f || l.z < -0.5f || l.z > 0.5f)
                return glm::vec3(0.f);
            glm::vec3 v;
            for (int d = 0; d < 3; ++d) v[d] = Solver.sampleVelocity(Solver.m_vel, l, d);
            return v * Size;
        }

        // ---- 水球: 运行时添加粒子 ----
        void AddParticleWorld(glm::vec3 const & worldPos, glm::vec3 const & worldVel) {
            if (int(Solver.m_particlePos.size()) >= 24000) return; // 防爆涨上限
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
            SurfaceLocalY.assign(std::size_t(X) * Z, -0.5f);
            for (glm::vec3 const & p : Solver.m_particlePos) {
                glm::ivec3 c = Solver.getParticleCell(p);
                int const col = c.x * Z + c.z;
                if (p.y > SurfaceLocalY[col]) SurfaceLocalY[col] = p.y;
            }
        }
    };
} // namespace VCX::Labs::Final
