// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "render/opengl/mvp_manager.h"

#include "core/convert.h"
#include "core/logger.h"
#include "render/opengl/shader.h"

using namespace jactorio;

void render::MvpManager::SetMvpUniformLocation(const int location) {
    mvpUniformLocation_ = location;

    LOG_MESSAGE(debug, "MVP uniform location set");
}

void render::MvpManager::UpdateShaderMvp() {
    CalculateMvpMatrix();
    Shader::SetUniformMat4F(mvpUniformLocation_, mvpMatrix_);
}

const glm::mat4& render::MvpManager::GetMvpMatrix() const {
    // assert(!calculateMatrix_);
    return mvpMatrix_;
}

const glm::mat4& render::MvpManager::CalculateGetMvpMatrix() {
    CalculateMvpMatrix();
    return mvpMatrix_;
}

void render::MvpManager::CalculateMvpMatrix() {
    if (calculateMatrix_) {
        assert(debugHasModel_);
        assert(debugHasView_);
        assert(debugHasProjection_);

        // Multiply backwards: projection, view, model
        mvpMatrix_       = projection_ * view_ * model_;
        calculateMatrix_ = false;
    }
}


void render::MvpManager::GlSetModelMatrix(const glm::mat4& matrix) {
    model_           = matrix;
    calculateMatrix_ = true;
    debugHasModel_   = true;
}

void render::MvpManager::GlSetViewMatrix(const glm::mat4& matrix) {
    view_            = matrix;
    calculateMatrix_ = true;
    debugHasView_    = true;
}

void render::MvpManager::GlSetProjectionMatrix(const glm::mat4& matrix) {
    projection_         = matrix;
    calculateMatrix_    = true;
    debugHasProjection_ = true;
}

// ======================================================================

void render::MvpManager::UpdateViewTransform() {
    const glm::mat4 view_mat = translate(glm::mat4(1.f), cameraTransform_);
    GlSetViewMatrix(view_mat);
}

glm::mat4 render::MvpManager::ToProjMatrix(const unsigned window_width,
                                           const unsigned window_height,
                                           const float offset) {
    // Calculate aspect ratio scale based on "larger / smaller"
    float x_zoom_ratio = 1.f;
    float y_zoom_ratio = 1.f;
    if (window_width > window_height) {
        x_zoom_ratio = SafeCast<float>(window_width) / SafeCast<float>(window_height);
    }
    else {
        y_zoom_ratio = SafeCast<float>(window_height) / SafeCast<float>(window_width);
    }

    return glm::ortho(offset * x_zoom_ratio,
                      SafeCast<float>(window_width) - offset * x_zoom_ratio,

                      SafeCast<float>(window_height) - offset * y_zoom_ratio,
                      offset * y_zoom_ratio,

                      -1.f,
                      1.f);
}
