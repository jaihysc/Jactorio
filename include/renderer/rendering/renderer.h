// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_RENDERER_RENDERING_RENDERER_H
#define JACTORIO_INCLUDE_RENDERER_RENDERING_RENDERER_H
#pragma once

#include <glm/glm.hpp>

#include "renderer/rendering/renderer_layer.h"
#include "renderer/rendering/spritemap_generator.h"

namespace jactorio::renderer
{
	class Renderer
	{
	public:
		Renderer();

	private:
		// #################################################
		// Sprites
		// Internal ids to spritemap positions
		static std::unordered_map<unsigned int, core::QuadPosition> spritemapCoords_;

	public:
		static void SetSpritemapCoords(
			const std::unordered_map<unsigned, core::QuadPosition>& spritemap_coords);

		///
		/// \param internal_id internal id of sprite prototype
		static core::QuadPosition GetSpritemapCoords(unsigned int internal_id);

	public:
		// #################################################
		// Rendering (Recalculated on window resize)

		// 2 Rendering layers so that one can be drawn to while another is being rendered
		// Since objects are of varying lengths, the layer must resize

		// 3rd layer is for rendering items that should be above all layers (e.g transport line items)
		static constexpr uint8_t kRenderLayerCount = 3;

		RendererLayer renderLayers[kRenderLayerCount];

		///
		/// \brief Changes zoom 
		float tileProjectionMatrixOffset = 0;

		static unsigned short tileWidth;

		///
		/// \brief Resizes opengl buffers used for rendering,
		/// Must be called from an OpenGL context
		void GRecalculateBuffers(unsigned short window_x, unsigned short window_y);

		///
		/// \brief Draws current data to the screen
		/// \param element_count Count of elements to draw (1 element = 6 indices)
		static void GDraw(unsigned int element_count);
		static void GClear();


	private:
		// #################################################
		// Window properties
		static unsigned short windowWidth_;
		static unsigned short windowHeight_;

	public:
		J_NODISCARD static unsigned short GetWindowWidth();
		J_NODISCARD static unsigned short GetWindowHeight();


	private:
		// #################################################
		// Grid properties (rendering, MVP matrices)

		unsigned int gridVerticesCount_ = 0;
		unsigned int gridElementsCount_ = 0;

		unsigned short tileCountX_ = 0;
		unsigned short tileCountY_ = 0;

	public:
		J_NODISCARD unsigned short GetGridSizeX() const;
		J_NODISCARD unsigned short GetGridSizeY() const;

		///
		/// \brief Updates projection matrix and zoom level
		void UpdateTileProjectionMatrix();
	};
};

#endif //JACTORIO_INCLUDE_RENDERER_RENDERING_RENDERER_H
