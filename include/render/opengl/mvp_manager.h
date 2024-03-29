// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDER_OPENGL_MVP_MANAGER_H
#define JACTORIO_INCLUDE_RENDER_OPENGL_MVP_MANAGER_H
#pragma once

#include <glm/glm.hpp>

#include "jactorio.h"

namespace jactorio::render
{
    class MvpManager
    {
    public:
        /// Uniform location for "u_model_view_projection_matrix"
        /// Must be set before MVP can be updated
        void SetMvpUniformLocation(int location);

        /// Sends current mvp matrices to GPU
        void UpdateShaderMvp();

        /// Returns mvp matrix if calculated, otherwise invalid call
        J_NODISCARD const glm::mat4& GetMvpMatrix() const;
        /// Calculates mvp matrix if not already calculated
        J_NODISCARD const glm::mat4& CalculateGetMvpMatrix();

        /// Calculates MVP matrix if calculate_matrix is true
        void CalculateMvpMatrix();


        void GlSetModelMatrix(const glm::mat4& matrix);
        void GlSetViewMatrix(const glm::mat4& matrix);
        void GlSetProjectionMatrix(const glm::mat4& matrix);

        /// Modifying the returned pointer will change the location of the camera
        glm::vec3* GetViewTransform() noexcept {
            return &cameraTransform_;
        }

        /// Sets the current view transform
        void UpdateViewTransform();

        /// Converts provided parameters into a matrix, guarantees a zoom of minimum of offset on all axis
        /// May offset more in a certain axis to preserve aspect ratio
        /// \param window_width width of display area in pixels
        /// \param window_height Height of display area in pixels
        /// \param pixel_zoom Number of pixels to hide starting from the border,
        /// will zoom more if necessary on an axis to maintain aspect ratio
        /// Cannot be 0
        ///
        /// Cannot be 0 as a each pixel would then have width of < 1 (e.g 0.99), which is invalid
        /// (OpenGL does not render)
        static glm::mat4 ToProjMatrix(unsigned window_width, unsigned window_height, float pixel_zoom);

    private:
        int mvpUniformLocation_ = -1;

        /// If true, the matrix will be calculated and stored upon calling update_shader_mvp() or get_matrix()
        bool calculateMatrix_ = false;

        glm::mat4 model_{};
        glm::mat4 view_{};
        glm::mat4 projection_{};

        glm::mat4 mvpMatrix_{};


        bool debugHasModel_      = false;
        bool debugHasView_       = false;
        bool debugHasProjection_ = false;


        glm::vec3 cameraTransform_ = glm::vec3(0, 0, 0);
    };
} // namespace jactorio::render

#endif // JACTORIO_INCLUDE_RENDER_OPENGL_MVP_MANAGER_H
