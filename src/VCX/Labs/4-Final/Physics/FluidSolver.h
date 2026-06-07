#pragma once

// 直接移植自 Lab2 实现的 FLIP/PIC 流体求解器.
// 坐标系为 [-0.5, 0.5]^3 单位立方体(tank). 世界变换由 FluidWorld 负责.
// 相对原版改动: 整理 namespace, setupScene -> setup(res, relWater) 参数化水体区域.

#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>
#include <vector>

namespace VCX::Labs::Final {
    struct FluidSolver {
        static constexpr int EMPTY_CELL = 0;
        static constexpr int FLUID_CELL = 1;
        static constexpr int SOLID_CELL = 2;

        std::vector<glm::vec3> m_particlePos;
        std::vector<glm::vec3> m_particleVel;
        std::vector<glm::vec3> m_particleColor;

        float m_fRatio { 0.95f };
        int   m_iCellX { 0 };
        int   m_iCellY { 0 };
        int   m_iCellZ { 0 };
        float m_h { 0.0f };
        float m_fInvSpacing { 0.0f };
        int   m_iNumCells { 0 };

        int   m_iNumSpheres { 0 };
        float m_particleRadius { 0.0f };

        std::vector<glm::vec3> m_vel;
        std::vector<glm::vec3> m_pre_vel;
        std::vector<float>     m_near_num[3];

        std::vector<float> m_p;
        std::vector<float> m_s;
        std::vector<int>   m_type;
        std::vector<float> m_particleDensity;
        float              m_particleRestDensity { 0.0f };

        // --- 用于 two-way 耦合 ---
        std::vector<float>     m_sStatic;   // 静态墙体掩码 (每步复原的基准)
        std::vector<glm::vec3> m_solidVel;  // 动态 solid 单元的速度 (local 单位/秒)

        glm::vec3 gravity { 0.0f, -9.81f, 0.0f };

        inline int cellId(int i, int j, int k) const {
            return (i * m_iCellY + j) * m_iCellZ + k;
        }

        inline bool insideGrid(int i, int j, int k) const {
            return i >= 0 && i < m_iCellX && j >= 0 && j < m_iCellY && k >= 0 && k < m_iCellZ;
        }

        inline glm::ivec3 getParticleCell(glm::vec3 p) const {
            glm::vec3 q = (p + glm::vec3(0.5f)) * m_fInvSpacing;
            return glm::ivec3(
                std::clamp(int(std::floor(q.x)), 0, m_iCellX - 1),
                std::clamp(int(std::floor(q.y)), 0, m_iCellY - 1),
                std::clamp(int(std::floor(q.z)), 0, m_iCellZ - 1));
        }

        inline bool isValidVelocity(int i, int j, int k, int dir) const {
            if (! insideGrid(i, j, k)) return false;
            if (dir == 0) {
                if (i <= 0) return false;
                return m_s[cellId(i, j, k)] > 0.0f && m_s[cellId(i - 1, j, k)] > 0.0f;
            }
            if (dir == 1) {
                if (j <= 0) return false;
                return m_s[cellId(i, j, k)] > 0.0f && m_s[cellId(i, j - 1, k)] > 0.0f;
            }
            if (k <= 0) return false;
            return m_s[cellId(i, j, k)] > 0.0f && m_s[cellId(i, j, k - 1)] > 0.0f;
        }

        float interpolate(std::vector<glm::vec3> const & data, glm::vec3 q, int dir) const {
            int   i0 = int(std::floor(q.x));
            int   j0 = int(std::floor(q.y));
            int   k0 = int(std::floor(q.z));
            float fx = q.x - float(i0);
            float fy = q.y - float(j0);
            float fz = q.z - float(k0);

            float ans = 0.0f;
            for (int a = 0; a < 2; a++) {
                for (int b = 0; b < 2; b++) {
                    for (int c = 0; c < 2; c++) {
                        int i = std::clamp(i0 + a, 0, m_iCellX - 1);
                        int j = std::clamp(j0 + b, 0, m_iCellY - 1);
                        int k = std::clamp(k0 + c, 0, m_iCellZ - 1);

                        float wx = a == 0 ? 1.0f - fx : fx;
                        float wy = b == 0 ? 1.0f - fy : fy;
                        float wz = c == 0 ? 1.0f - fz : fz;
                        ans += data[cellId(i, j, k)][dir] * wx * wy * wz;
                    }
                }
            }
            return ans;
        }

        float interpolateValid(std::vector<glm::vec3> const & data, glm::vec3 q, int dir) const {
            int   i0 = int(std::floor(q.x));
            int   j0 = int(std::floor(q.y));
            int   k0 = int(std::floor(q.z));
            float fx = q.x - float(i0);
            float fy = q.y - float(j0);
            float fz = q.z - float(k0);

            float ans = 0.0f;
            float sum = 0.0f;
            for (int a = 0; a < 2; a++) {
                for (int b = 0; b < 2; b++) {
                    for (int c = 0; c < 2; c++) {
                        int i = std::clamp(i0 + a, 0, m_iCellX - 1);
                        int j = std::clamp(j0 + b, 0, m_iCellY - 1);
                        int k = std::clamp(k0 + c, 0, m_iCellZ - 1);

                        if (! isValidVelocity(i, j, k, dir)) continue;

                        float wx = a == 0 ? 1.0f - fx : fx;
                        float wy = b == 0 ? 1.0f - fy : fy;
                        float wz = c == 0 ? 1.0f - fz : fz;
                        float w  = wx * wy * wz;
                        ans += data[cellId(i, j, k)][dir] * w;
                        sum += w;
                    }
                }
            }

            if (sum > 0.0f) return ans / sum;
            return interpolate(data, q, dir);
        }

        glm::vec3 facePosition(glm::vec3 p, int dir) const {
            glm::vec3 q = (p + glm::vec3(0.5f)) * m_fInvSpacing;
            if (dir == 0) q += glm::vec3(0.0f, -0.5f, -0.5f);
            if (dir == 1) q += glm::vec3(-0.5f, 0.0f, -0.5f);
            if (dir == 2) q += glm::vec3(-0.5f, -0.5f, 0.0f);
            return q;
        }

        float sampleVelocity(std::vector<glm::vec3> const & data, glm::vec3 p, int dir) const {
            return interpolateValid(data, facePosition(p, dir), dir);
        }

        void addParticleVelocityToGrid(glm::vec3 p, int dir, float v) {
            glm::vec3 q = facePosition(p, dir);

            int   i0 = int(std::floor(q.x));
            int   j0 = int(std::floor(q.y));
            int   k0 = int(std::floor(q.z));
            float fx = q.x - float(i0);
            float fy = q.y - float(j0);
            float fz = q.z - float(k0);

            for (int a = 0; a < 2; a++) {
                for (int b = 0; b < 2; b++) {
                    for (int c = 0; c < 2; c++) {
                        int i = std::clamp(i0 + a, 0, m_iCellX - 1);
                        int j = std::clamp(j0 + b, 0, m_iCellY - 1);
                        int k = std::clamp(k0 + c, 0, m_iCellZ - 1);

                        float wx = a == 0 ? 1.0f - fx : fx;
                        float wy = b == 0 ? 1.0f - fy : fy;
                        float wz = c == 0 ? 1.0f - fz : fz;
                        float w  = wx * wy * wz;
                        int   id = cellId(i, j, k);

                        m_vel[id][dir] += v * w;
                        m_near_num[dir][id] += w;
                    }
                }
            }
        }

        void integrateParticles(float timeStep) {
            for (std::size_t i = 0; i < m_particlePos.size(); i++) {
                m_particleVel[i] += gravity * timeStep;
                m_particlePos[i] += m_particleVel[i] * timeStep;
            }
        }

        void pushParticlesApart(int numIters) {
            float minDist  = 2.0f * m_particleRadius;
            float minDist2 = minDist * minDist;

            for (int iter = 0; iter < numIters; iter++) {
                std::vector<std::vector<int>> buckets(m_iNumCells);

                for (int i = 0; i < int(m_particlePos.size()); i++) {
                    glm::ivec3 c = getParticleCell(m_particlePos[i]);
                    buckets[cellId(c.x, c.y, c.z)].push_back(i);
                }

                for (int i = 0; i < int(m_particlePos.size()); i++) {
                    glm::ivec3 c = getParticleCell(m_particlePos[i]);

                    for (int dx = -1; dx <= 1; dx++) {
                        for (int dy = -1; dy <= 1; dy++) {
                            for (int dz = -1; dz <= 1; dz++) {
                                int x = c.x + dx;
                                int y = c.y + dy;
                                int z = c.z + dz;
                                if (! insideGrid(x, y, z)) continue;

                                for (int j : buckets[cellId(x, y, z)]) {
                                    if (j <= i) continue;

                                    glm::vec3 diff  = m_particlePos[j] - m_particlePos[i];
                                    float     dist2 = glm::dot(diff, diff);
                                    if (dist2 >= minDist2) continue;

                                    float     dist = std::sqrt(std::max(dist2, 0.000001f));
                                    glm::vec3 n    = diff / dist;
                                    float     move = 0.5f * (minDist - dist);

                                    m_particlePos[i] -= n * move;
                                    m_particlePos[j] += n * move;
                                }
                            }
                        }
                    }
                }
            }
        }

        void handleParticleCollisions() {
            float lo = -0.5f + m_h + m_particleRadius;
            float hi = 0.5f - 2.0f * m_h - m_particleRadius;
            float wallBand = 2.5f * m_particleRadius;

            for (std::size_t i = 0; i < m_particlePos.size(); i++) {
                for (int d = 0; d < 3; d++) {
                    if (m_particlePos[i][d] < lo) {
                        m_particlePos[i][d] = lo;
                        if (m_particleVel[i][d] < 0.0f) m_particleVel[i][d] = 0.0f;
                    }
                    if (m_particlePos[i][d] > hi) {
                        m_particlePos[i][d] = hi;
                        if (m_particleVel[i][d] > 0.0f) m_particleVel[i][d] = 0.0f;
                    }
                }

                if (m_particlePos[i].y < lo + wallBand) {
                    m_particleVel[i].x *= 0.35f;
                    m_particleVel[i].z *= 0.35f;
                }
            }
        }

        void updateParticleDensity() {
            std::fill(m_particleDensity.begin(), m_particleDensity.end(), 0.0f);

            for (glm::vec3 const & p : m_particlePos) {
                glm::vec3 q = (p + glm::vec3(0.5f)) * m_fInvSpacing - glm::vec3(0.5f);

                int   i0 = int(std::floor(q.x));
                int   j0 = int(std::floor(q.y));
                int   k0 = int(std::floor(q.z));
                float fx = q.x - float(i0);
                float fy = q.y - float(j0);
                float fz = q.z - float(k0);

                for (int a = 0; a < 2; a++) {
                    for (int b = 0; b < 2; b++) {
                        for (int c = 0; c < 2; c++) {
                            int i = std::clamp(i0 + a, 0, m_iCellX - 1);
                            int j = std::clamp(j0 + b, 0, m_iCellY - 1);
                            int k = std::clamp(k0 + c, 0, m_iCellZ - 1);

                            float wx = a == 0 ? 1.0f - fx : fx;
                            float wy = b == 0 ? 1.0f - fy : fy;
                            float wz = c == 0 ? 1.0f - fz : fz;
                            m_particleDensity[cellId(i, j, k)] += wx * wy * wz;
                        }
                    }
                }
            }

            if (m_particleRestDensity == 0.0f) {
                float sum = 0.0f;
                int   cnt = 0;
                for (int id = 0; id < m_iNumCells; id++) {
                    if (m_type[id] == FLUID_CELL) {
                        sum += m_particleDensity[id];
                        cnt++;
                    }
                }
                if (cnt > 0) m_particleRestDensity = sum / float(cnt);
            }
        }

        void transferVelocities(bool toGrid, float flipRatio) {
            if (toGrid) {
                std::fill(m_vel.begin(), m_vel.end(), glm::vec3(0.0f));
                for (int d = 0; d < 3; d++) std::fill(m_near_num[d].begin(), m_near_num[d].end(), 0.0f);

                for (int id = 0; id < m_iNumCells; id++) {
                    m_type[id] = m_s[id] == 0.0f ? SOLID_CELL : EMPTY_CELL;
                }
                for (glm::vec3 const & p : m_particlePos) {
                    glm::ivec3 c  = getParticleCell(p);
                    int        id = cellId(c.x, c.y, c.z);
                    if (m_type[id] != SOLID_CELL) m_type[id] = FLUID_CELL;
                }

                for (std::size_t i = 0; i < m_particlePos.size(); i++) {
                    addParticleVelocityToGrid(m_particlePos[i], 0, m_particleVel[i].x);
                    addParticleVelocityToGrid(m_particlePos[i], 1, m_particleVel[i].y);
                    addParticleVelocityToGrid(m_particlePos[i], 2, m_particleVel[i].z);
                }

                for (int id = 0; id < m_iNumCells; id++) {
                    for (int d = 0; d < 3; d++) {
                        if (m_near_num[d][id] > 0.0f) m_vel[id][d] /= m_near_num[d][id];
                    }
                }

                for (int i = 0; i < m_iCellX; i++) {
                    for (int j = 0; j < m_iCellY; j++) {
                        for (int k = 0; k < m_iCellZ; k++) {
                            int id = cellId(i, j, k);
                            for (int d = 0; d < 3; d++) {
                                if (isValidVelocity(i, j, k, d)) continue;
                                // 静态墙为0, 运动 solid 面则赋予该 solid 的速度
                                // (移动边界条件 → 压力投影把水推开).
                                float sv = 0.0f;
                                if (m_s[id] == 0.0f) {
                                    sv = m_solidVel[id][d];
                                } else {
                                    int pi = i - (d == 0 ? 1 : 0);
                                    int pj = j - (d == 1 ? 1 : 0);
                                    int pk = k - (d == 2 ? 1 : 0);
                                    if (insideGrid(pi, pj, pk) && m_s[cellId(pi, pj, pk)] == 0.0f) {
                                        sv = m_solidVel[cellId(pi, pj, pk)][d];
                                    }
                                }
                                m_vel[id][d] = sv;
                            }
                        }
                    }
                }

                m_pre_vel = m_vel;
            } else {
                float r = std::clamp(flipRatio, 0.0f, 1.0f);

                for (std::size_t i = 0; i < m_particlePos.size(); i++) {
                    glm::vec3 pic(0.0f);
                    glm::vec3 delta(0.0f);
                    for (int d = 0; d < 3; d++) {
                        float newVel = sampleVelocity(m_vel, m_particlePos[i], d);
                        float oldVel = sampleVelocity(m_pre_vel, m_particlePos[i], d);
                        pic[d]       = newVel;
                        delta[d]     = newVel - oldVel;
                    }

                    glm::vec3 flip = m_particleVel[i] + delta;
                    m_particleVel[i] = (1.0f - r) * pic + r * flip;
                }
            }
        }

        void solveIncompressibility(int numIters, float dt, float overRelaxation, bool compensateDrift) {
            std::fill(m_p.begin(), m_p.end(), 0.0f);

            for (int iter = 0; iter < numIters; iter++) {
                for (int i = 1; i < m_iCellX - 1; i++) {
                    for (int j = 1; j < m_iCellY - 1; j++) {
                        for (int k = 1; k < m_iCellZ - 1; k++) {
                            int id = cellId(i, j, k);
                            if (m_type[id] != FLUID_CELL) continue;

                            float sx0 = m_s[cellId(i - 1, j, k)];
                            float sx1 = m_s[cellId(i + 1, j, k)];
                            float sy0 = m_s[cellId(i, j - 1, k)];
                            float sy1 = m_s[cellId(i, j + 1, k)];
                            float sz0 = m_s[cellId(i, j, k - 1)];
                            float sz1 = m_s[cellId(i, j, k + 1)];
                            float s   = sx0 + sx1 + sy0 + sy1 + sz0 + sz1;
                            if (s == 0.0f) continue;

                            float div = m_vel[cellId(i + 1, j, k)].x - m_vel[id].x
                                      + m_vel[cellId(i, j + 1, k)].y - m_vel[id].y
                                      + m_vel[cellId(i, j, k + 1)].z - m_vel[id].z;

                            if (compensateDrift && m_particleRestDensity > 0.0f) {
                                float compression = m_particleDensity[id] - m_particleRestDensity;
                                if (compression > 0.0f) div -= 0.02f * compression;
                            }

                            float p = -div / s;
                            p *= overRelaxation;
                            m_p[id] += p / std::max(dt, 0.0001f);

                            m_vel[id].x -= sx0 * p;
                            m_vel[cellId(i + 1, j, k)].x += sx1 * p;
                            m_vel[id].y -= sy0 * p;
                            m_vel[cellId(i, j + 1, k)].y += sy1 * p;
                            m_vel[id].z -= sz0 * p;
                            m_vel[cellId(i, j, k + 1)].z += sz1 * p;
                        }
                    }
                }
            }
        }

        void updateParticleColors() {
            for (std::size_t i = 0; i < m_particleColor.size(); i++) {
                float speed = glm::length(m_particleVel[i]);
                float t     = std::clamp(speed / 3.0f, 0.0f, 1.0f);
                t           = std::sqrt(t);

                glm::vec3 blue(0.00f, 0.05f, 1.00f);
                glm::vec3 cyan(0.00f, 0.95f, 1.00f);
                glm::vec3 yellow(1.00f, 0.95f, 0.00f);
                glm::vec3 red(1.00f, 0.05f, 0.00f);

                if (t < 0.33f) {
                    float a = t / 0.33f;
                    m_particleColor[i] = blue * (1.0f - a) + cyan * a;
                } else if (t < 0.66f) {
                    float a = (t - 0.33f) / 0.33f;
                    m_particleColor[i] = cyan * (1.0f - a) + yellow * a;
                } else {
                    float a = (t - 0.66f) / 0.34f;
                    m_particleColor[i] = yellow * (1.0f - a) + red * a;
                }
            }
        }

        // 每步重置动态 solid 掩码/速度 (只保留静态墙).
        void beginDynamicSolids() {
            m_s = m_sStatic;
            std::fill(m_solidVel.begin(), m_solidVel.end(), glm::vec3(0.0f));
        }

        void SimulateTimestep(float const dt) {
            int   numSubSteps       = 2;
            int   numParticleIters  = 2;
            int   numPressureIters  = 40;
            bool  separateParticles = true;
            float overRelaxation    = 1.6f;
            bool  compensateDrift   = false;

            float sdt = dt / float(numSubSteps);

            for (int step = 0; step < numSubSteps; step++) {
                integrateParticles(sdt);
                handleParticleCollisions();
                if (separateParticles) pushParticlesApart(numParticleIters);
                handleParticleCollisions();
                transferVelocities(true, m_fRatio);
                updateParticleDensity();
                solveIncompressibility(numPressureIters, sdt, overRelaxation, compensateDrift);
                transferVelocities(false, m_fRatio);
            }
            updateParticleColors();
        }

        // res: 网格分辨率, relWater: 水体占 tank 的比例(0~1).
        void setup(int res, glm::vec3 relWater) {
            glm::vec3 tank(1.0f);

            float _h      = tank.y / float(res);
            float point_r = 0.3f * _h;
            float dx      = 2.0f * point_r;
            float dy      = std::sqrt(3.0f) / 2.0f * dx;
            float dz      = dx;

            int numX = int(std::floor((relWater.x * tank.x - 2.0f * _h - 2.0f * point_r) / dx));
            int numY = int(std::floor((relWater.y * tank.y - 2.0f * _h - 2.0f * point_r) / dy));
            int numZ = int(std::floor((relWater.z * tank.z - 2.0f * _h - 2.0f * point_r) / dz));
            numX = std::max(numX, 0);
            numY = std::max(numY, 0);
            numZ = std::max(numZ, 0);

            m_iNumSpheres    = numX * numY * numZ;
            m_iCellX         = res + 1;
            m_iCellY         = res + 1;
            m_iCellZ         = res + 1;
            m_h              = 1.0f / float(res);
            m_fInvSpacing    = float(res);
            m_iNumCells      = m_iCellX * m_iCellY * m_iCellZ;
            m_particleRadius = point_r;
            m_particleRestDensity = 0.0f;

            m_particlePos.assign(m_iNumSpheres, glm::vec3(0.0f));
            m_particleVel.assign(m_iNumSpheres, glm::vec3(0.0f));
            m_particleColor.assign(m_iNumSpheres, glm::vec3(0.0f, 0.05f, 1.0f));

            m_vel.assign(m_iNumCells, glm::vec3(0.0f));
            m_pre_vel.assign(m_iNumCells, glm::vec3(0.0f));
            for (int i = 0; i < 3; ++i) m_near_num[i].assign(m_iNumCells, 0.0f);

            m_p.assign(m_iNumCells, 0.0f);
            m_s.assign(m_iNumCells, 0.0f);
            m_type.assign(m_iNumCells, EMPTY_CELL);
            m_particleDensity.assign(m_iNumCells, 0.0f);
            m_solidVel.assign(m_iNumCells, glm::vec3(0.0f));

            int p = 0;
            for (int i = 0; i < numX; i++) {
                for (int j = 0; j < numY; j++) {
                    for (int k = 0; k < numZ; k++) {
                        float shift = (j % 2 == 0) ? 0.0f : point_r;
                        m_particlePos[p++] = glm::vec3(
                            m_h + point_r + dx * i + shift,
                            m_h + point_r + dy * j,
                            m_h + point_r + dz * k + shift) + glm::vec3(-0.5f);
                    }
                }
            }

            for (int i = 0; i < m_iCellX; i++) {
                for (int j = 0; j < m_iCellY; j++) {
                    for (int k = 0; k < m_iCellZ; k++) {
                        float s = 1.0f;
                        if (i == 0 || i >= m_iCellX - 2 || j == 0 || j >= m_iCellY - 2 || k == 0 || k >= m_iCellZ - 2)
                            s = 0.0f;
                        m_s[cellId(i, j, k)] = s;
                    }
                }
            }

            m_sStatic = m_s;   // 保存静态墙掩码 (two-way 每步复原用)
        }
    };
} // namespace VCX::Labs::Final
