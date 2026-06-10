#include "Labs/4-Final/Physics/ContactDetection.h"

#include <array>
#include <limits>

#include <Eigen/Geometry>
#include <fcl/narrowphase/collision.h>
#include <fcl/narrowphase/contact.h>
#include <glm/ext.hpp>

namespace VCX::Labs::Final {
    namespace {
        glm::vec3 SafeNormalize(glm::vec3 const & v, glm::vec3 const & fallback = glm::vec3(0.f, 1.f, 0.f)) {
            float const len = glm::length(v);
            return len > 1e-6f ? v / len : fallback;
        }

        fcl::CollisionObjectf CreateCollisionObject(RigidBody const & body) {
            fcl::Transform3f transform = fcl::Transform3f::Identity();
            transform.translation() = Eigen::Vector3f(body.Position.x, body.Position.y, body.Position.z);
            transform.linear() = Eigen::Quaternionf(body.Rotation.w, body.Rotation.x, body.Rotation.y, body.Rotation.z).toRotationMatrix();

            if (body.Kind == BodyKind::Bird) {
                auto shape = std::make_shared<fcl::Spheref>(body.Radius * body.Scale);
                return fcl::CollisionObjectf(shape, transform);
            }

            glm::vec3 const scaledHalfSize = body.HalfSize * body.Scale;
            auto shape = std::make_shared<fcl::Boxf>(scaledHalfSize.x * 2.f, scaledHalfSize.y * 2.f, scaledHalfSize.z * 2.f);
            return fcl::CollisionObjectf(shape, transform);
        }

        bool QueryFclContact(RigidBody const & a, RigidBody const & b, Contact & contact) {
            auto objA = CreateCollisionObject(a);
            auto objB = CreateCollisionObject(b);

            fcl::CollisionRequestf request;
            request.enable_contact = true;
            request.num_max_contacts = 16;
            fcl::CollisionResultf result;
            fcl::collide(&objA, &objB, request, result);
            if (!result.isCollision()) return false;

            glm::vec3 normal(0.f);
            glm::vec3 position(0.f);
            float penetration = 0.f;
            contact.Points.clear();
            contact.Points.reserve(result.numContacts());
            for (int i = 0; i < result.numContacts(); ++i) {
                auto const & c = result.getContact(i);
                glm::vec3 const point = glm::vec3(c.pos[0], c.pos[1], c.pos[2]);
                normal += glm::vec3(c.normal[0], c.normal[1], c.normal[2]);
                position += point;
                penetration += c.penetration_depth;
                contact.Points.push_back(point);
            }
            normal = SafeNormalize(normal);
            if (glm::dot(normal, b.Position - a.Position) < 0.f) {
                normal = -normal;
            }
            contact.Normal = normal;
            contact.Point = position / float(result.numContacts());
            contact.Penetration = penetration / float(result.numContacts());
            return true;
        }
    }

    namespace ContactDetection {
    void CollectContacts(std::vector<RigidBody> const & bodies, std::vector<Contact> & contacts) {
        contacts.clear();
        for (int i = 0; i < int(bodies.size()); ++i) {
            if (!bodies[i].IsAlive) continue;

            Contact ground;
            if (GroundContact(bodies[i], i, ground)) {
                contacts.push_back(ground);
            }
            for (int j = i + 1; j < int(bodies.size()); ++j) {
                Contact contact;
                if (FindContact(bodies, i, j, contact)) {
                    contacts.push_back(contact);
                }
            }
        }
    }

    bool FindContact(std::vector<RigidBody> const & bodies, int a, int b, Contact & contact) {
        auto const & ba = bodies[a];
        auto const & bb = bodies[b];
        if (!ba.IsAlive || !bb.IsAlive || (ba.IsStatic && bb.IsStatic)) return false;
        if (ba.Kind == BodyKind::Fragment || bb.Kind == BodyKind::Fragment) return false;
        if (ba.Kind == BodyKind::Bird && bb.Kind == BodyKind::Bird) {
            if (!ba.BirdWasLaunched && !bb.BirdWasLaunched) return false;
            return SphereBoxContact(ba, bb, a, b, contact);
        }

        if (ba.Kind == BodyKind::Bird && bb.Kind != BodyKind::Bird) {
            return SphereBoxContact(ba, bb, a, b, contact);
        }
        if (bb.Kind == BodyKind::Bird && ba.Kind != BodyKind::Bird) {
            return SphereBoxContact(bb, ba, b, a, contact);
        }
        return BoxBoxContact(ba, bb, a, b, contact);
    }

    bool SphereBoxContact(RigidBody const & sphere, RigidBody const & box, int sphereIndex, int boxIndex, Contact & contact) {
        if (!QueryFclContact(sphere, box, contact)) return false;
        contact.A = sphereIndex;
        contact.B = boxIndex;
        return true;
    }

    bool BoxBoxContact(RigidBody const & a, RigidBody const & b, int aIndex, int bIndex, Contact & contact) {
        if (!QueryFclContact(a, b, contact)) return false;
        contact.A = aIndex;
        contact.B = bIndex;
        return true;
    }

    bool GroundContact(RigidBody const & body, int index, Contact & contact) {
        if (body.IsStatic || !body.IsAlive) return false;

        float bottom = body.Position.y - body.Radius * body.Scale;
        glm::vec3 contactPoint = glm::vec3(body.Position.x, GroundY, body.Position.z);
        contact.Points.clear();
        contact.Points.push_back(contactPoint);
        if (body.Kind != BodyKind::Bird) {
            bottom = std::numeric_limits<float>::max();
            glm::vec3 sumPoint(0.f);
            int count = 0;
            contact.Points.clear();
            std::array<glm::vec3, 8> const signs = {
                glm::vec3(-1,  1,  1), glm::vec3( 1,  1,  1), glm::vec3( 1,  1, -1), glm::vec3(-1,  1, -1),
                glm::vec3(-1, -1,  1), glm::vec3( 1, -1,  1), glm::vec3( 1, -1, -1), glm::vec3(-1, -1, -1),
            };
            glm::vec3 const scaledHalfSize = body.HalfSize * body.Scale;
            for (auto const & sign : signs) {
                glm::vec3 const vertex = body.Position + body.Rotation * (sign * scaledHalfSize);
                if (vertex.y < bottom - 1e-5f) {
                    bottom = vertex.y;
                    sumPoint = vertex;
                    count = 1;
                    contact.Points.clear();
                    contact.Points.push_back(vertex);
                } else if (vertex.y <= bottom + 1e-5f) {
                    sumPoint += vertex;
                    ++count;
                    contact.Points.push_back(vertex);
                }
            }
            if (count > 0) {
                contactPoint = sumPoint / float(count);
            }
        }
        if (bottom >= GroundY) return false;

        contact.A = index;
        contact.B = -1;
        contact.Normal = glm::vec3(0.f, -1.f, 0.f);
        contact.Penetration = GroundY - bottom;
        contact.Point = contactPoint;
        if (contact.Points.empty()) {
            contact.Points.push_back(contactPoint);
        }
        return true;
    }
    }
}
