//
// Created by Jan on 12-Apr-21.
//

#ifndef ENGINE_TRANSFORM_H
#define ENGINE_TRANSFORM_H

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

class transform {
private:
    glm::quat orientation;
    glm::vec3 scale;
    glm::mat4 trans;

private:
    static void make_trans_matrix(const glm::vec3 &pos, glm::mat4 &trans) noexcept {
        trans[3] = glm::vec4{pos, 1.0f};
    }

    static void make_rot_matrix(const glm::quat &rot, const glm::vec3 &scale, glm::mat4 &trans) noexcept {
        glm::mat3 rotation = toMat3(rot);
        trans[0] = glm::vec4{rotation[0], 0.0f} * scale.x;
        trans[1] = glm::vec4{rotation[1], 0.0f} * scale.y;
        trans[2] = glm::vec4{rotation[2], 0.0f} * scale.z;
    }

    static glm::mat4 make_matrix(const glm::vec3 &pos, const glm::quat &rot, const glm::vec3 &scl) noexcept {
        glm::mat4 trans;
        make_rot_matrix(rot, scl, trans);
        make_trans_matrix(pos, trans);
        return trans;
    }

public:
    transform() noexcept :
            orientation(0, 0, 0, 1),
            scale(1, 1, 1),
            trans(1) {
    }

    transform(const glm::vec3 &pos, const glm::quat &rot, const glm::vec3 &scl) noexcept :
            orientation(rot),
            scale(scl),
            trans(make_matrix(pos, rot, scl)) {
    }

    transform(const glm::vec3 &pos, const glm::vec3 &rot, const glm::vec3 &scl) noexcept :
            transform(pos, glm::quat(rot), scl) {
    }

public:
    [[nodiscard]] glm::mat4 to_mat4() const noexcept {
        return trans;
    }

    [[nodiscard]] glm::mat3 to_mat3() const noexcept {
        return glm::mat3{trans};
    }

    [[nodiscard]] glm::vec3 get_position() const noexcept {
        return trans[3];
    }

    void set_position(const glm::vec3 pos) noexcept {
        make_trans_matrix(pos, trans);
    }

    [[nodiscard]] glm::quat get_orientation() const noexcept {
        return orientation;
    }

    void set_orientation(const glm::quat &rot) noexcept {
        orientation = rot;
        make_rot_matrix(orientation, scale, trans);
    }

    void set_orientation(const glm::vec3 &euler) noexcept {
        set_orientation(glm::quat(euler));
    }

    [[nodiscard]] glm::vec3 get_scale() const noexcept {
        return scale;
    }

    void set_scale(const glm::vec3 &scl) noexcept {
        glm::vec3 factor = scl / scale;
        scale = scl;
        trans[0] *= factor.x;
        trans[1] *= factor.y;
        trans[2] *= factor.z;
    }

    void translate(const glm::vec3 &translation) noexcept {
        set_position(get_position() + translation);
    }

    void translate_local(const glm::vec3 &translation) noexcept {
        translate(to_mat3() * translation);
    }

    void rotate(const glm::quat &rotation) noexcept {
        set_orientation(get_orientation() * rotation);
    }

    [[nodiscard]] glm::vec3 right() const noexcept {
        return to_mat4()[0];
    }

    [[nodiscard]] glm::vec3 up() const noexcept {
        return to_mat4()[1];
    }

    [[nodiscard]] glm::vec3 forward() const noexcept {
        return to_mat4()[2];
    }
};

#endif //ENGINE_TRANSFORM_H
