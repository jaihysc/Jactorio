// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <GL/glew.h>

#include "renderer/rendering/renderer.h"

#include <glm/gtc/matrix_transform.hpp>

#include "renderer/opengl/error.h"
#include "renderer/opengl/shader_manager.h"
#include "renderer/rendering/mvp_manager.h"

unsigned short jactorio::renderer::Renderer::windowWidth_  = 0;
unsigned short jactorio::renderer::Renderer::windowHeight_ = 0;

unsigned short jactorio::renderer::Renderer::tileWidth = 6;

std::unordered_map<unsigned int, jactorio::core::QuadPosition> jactorio::renderer::Renderer::spritemapCoords_{};

void jactorio::renderer::Renderer::SetSpritemapCoords(
	const std::unordered_map<unsigned, core::QuadPosition>& spritemap_coords) {
	spritemapCoords_ = spritemap_coords;
}

jactorio::core::QuadPosition jactorio::renderer::Renderer::GetSpritemapCoords(const unsigned internal_id) {
	return spritemapCoords_.at(internal_id);
}


// non static

jactorio::renderer::Renderer::Renderer() {
	// Initialize model matrix
	const glm::mat4 model_matrix = translate(glm::mat4(1.f), glm::vec3(0, 0, 0));
	SetgModelMatrix(model_matrix);
	UpdateShaderMvp();

	// Get window size
	GLint m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);

	LOG_MESSAGE_F(debug, "%d layers used for rendering", kRenderLayerCount);
	for (auto& render_layer : renderLayers) {
		render_layer.GInitBuffer();
	}

	GRecalculateBuffers(m_viewport[2], m_viewport[3]);
}

void jactorio::renderer::Renderer::GRecalculateBuffers(const unsigned short window_x,
                                                       const unsigned short window_y) {
	// Initialize fields
	windowWidth_  = window_x;
	windowHeight_ = window_y;
	UpdateTileProjectionMatrix();

	// Raise the bottom and right by tile_width so the last tile has enough space to render out
	tileCountX_ = windowWidth_ / tileWidth + 1;
	tileCountY_ = windowHeight_ / tileWidth + 1;

	gridElementsCount_ = tileCountX_ * tileCountY_;

	// Render layer (More may be reserved as needed by the renderer)
	for (auto& render_layer : renderLayers) {
		render_layer.Reserve(gridElementsCount_);
		render_layer.GUpdateData();
	}
}


// openGL methods

void jactorio::renderer::Renderer::GDraw(const unsigned int element_count) {
	DEBUG_OPENGL_CALL(
		// There are 6 indices for each tile
		glDrawElements(GL_TRIANGLES, element_count * 6, GL_UNSIGNED_INT, nullptr)
	); // Pointer not needed as buffer is already bound
}

void jactorio::renderer::Renderer::GClear() {
	DEBUG_OPENGL_CALL(glClear(GL_COLOR_BUFFER_BIT));
}


// Grid properties
unsigned short jactorio::renderer::Renderer::GetWindowWidth() {
	return windowWidth_;
}

unsigned short jactorio::renderer::Renderer::GetWindowHeight() {
	return windowHeight_;
}


unsigned short jactorio::renderer::Renderer::GetGridSizeX() const {
	return tileCountX_;
}

unsigned short jactorio::renderer::Renderer::GetGridSizeY() const {
	return tileCountY_;
}


void jactorio::renderer::Renderer::UpdateTileProjectionMatrix() {
	const auto max_tile_width = static_cast<float>(tileWidth * 2);

	if (tileProjectionMatrixOffset < max_tile_width)
		// Prevent zooming out too far
		tileProjectionMatrixOffset = max_tile_width;
	else {
		// Prevent zooming too far in
		unsigned short smallest_axis;
		if (windowWidth_ > windowHeight_) {
			smallest_axis = windowHeight_;
		}
		else {
			smallest_axis = windowWidth_;
		}

		// Maximum zoom is 30 from center
		const int max_zoom_offset = 30;
		if (tileProjectionMatrixOffset > static_cast<float>(smallest_axis) / 2 - max_zoom_offset) {
			tileProjectionMatrixOffset = static_cast<float>(smallest_axis) / 2 - max_zoom_offset;
		}
	}

	SetgProjectionMatrix(ToProjMatrix(windowWidth_, windowHeight_, tileProjectionMatrixOffset));
}
