// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDERER_OPENGL_SHADER_MANAGER_H
#define JACTORIO_INCLUDE_RENDERER_OPENGL_SHADER_MANAGER_H
#pragma once

#include <glm/glm.hpp>

#include "jactorio.h"

namespace jactorio::renderer
{
	class MvpManager
	{
	public:
		///
		/// \brief Uniform location for "u_model_view_projection_matrix"
		/// Must be set before MVP can be updated
		void SetMvpUniformLocation(int location);

		///
		/// \brief Sends current mvp matrices to GPU
		void UpdateShaderMvp();

		///
		/// \brief Returns mvp matrix if calculated, otherwise invalid call
		J_NODISCARD const glm::mat4& GetMvpMatrix() const;
		///
		/// \brief Calculates mvp matrix if not already calculated
		J_NODISCARD const glm::mat4& CalculateGetMvpMatrix();

		///
		/// \brief Calculates MVP matrix if calculate_matrix is true
		void CalculateMvpMatrix();


		void GlSetModelMatrix(const glm::mat4& matrix);
		void GlSetViewMatrix(const glm::mat4& matrix);
		void GlSetProjectionMatrix(const glm::mat4& matrix);

		///
		/// \brief Modifying the returned pointer will change the location of the camera
		glm::vec3* GetViewTransform() noexcept {
			return &cameraTransform_;
		}

		///
		/// \brief Sets the current view transform
		void UpdateViewTransform();

		///
		/// \brief Converts provided parameters into a matrix, guarantees a zoom of minimum of offset on all axis
		/// May offset more in a certain axis to preserve aspect ratio
		/// \param window_width width of display area in pixels
		/// \param window_height Height of display area in pixels
		/// \param offset number of tiles horizontally to from the edge to hide (giving a zoom effect) <br>
		/// Vertical tiles is calculated based on ration of tiles in tile_data
		static glm::mat4 ToProjMatrix(unsigned short window_width,
		                              unsigned short window_height, float offset);
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
}

#endif //JACTORIO_INCLUDE_RENDERER_OPENGL_SHADER_MANAGER_H
