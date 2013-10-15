/***************************************************************************//**
 * rendergl.hpp
 * OpenGL rendering calls
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (RENDERGL_HPP)
#define RENDERGL_HPP
#include "graphics/graphics_object.h"
#include "graphics/render.hpp"
#include "graphics/graphics_object_highlight.hpp"

class Render_graphics_opengl : public Render_graphics_compile_members
{
public:

	/** Indicates that we are rendering specifically to pick objects.
	 * Requires the ndc transformation to be different as the viewport is the picking window.
	 */
	int picking;

	int allow_texture_tiling; /** Flag controls whether when compiling a large texture
								  it can be split into multiple texture tiles */
	Texture_tiling *texture_tiling;  /** If a given texture is compiled into tiles
													 then this field is filled in and expected to
													 be used when compiling graphics that use that material. */
	double viewport_width, viewport_height;

	int current_layer, number_of_layers;

	unsigned int buffer_width;
	unsigned int buffer_height;

	SubObjectGroupHighlightFunctor *highlight_functor;

private:
	// scale factor multiplying graphics render_line_width and render_point_size
	// to get size in pixels. Set it so high resolution output has thick enough
	// lines and visible points compared with on-screen resolution.
	double point_unit_size_pixels;
	
public:
	Render_graphics_opengl() :
		picking(0),
		allow_texture_tiling(0),
		texture_tiling(0),
		viewport_width(1.0),
		viewport_height(1.0),
		current_layer(0),
		number_of_layers(1),
		buffer_width(0),
		buffer_height(0),
		highlight_functor(NULL),
		point_unit_size_pixels(1.0)
	{
	}

	virtual ~Render_graphics_opengl()
	{
		if (highlight_functor)
		{
			delete highlight_functor;
		}
	}

	/***************************************************************************//**
	 * @see Render_graphics::Graphics_object_compile
	 */
	virtual int Graphics_object_compile(GT_object *graphics_object);

	/***************************************************************************//**
	 * @see Render_graphics::Material_compile
	 */
	virtual int Material_compile(Graphical_material *material);

	virtual int rendering_layer(int layer)
	{
		if (layer == current_layer)
			return 1;
		if (layer >= number_of_layers)
			number_of_layers = layer + 1;
		return 0;
	}

	virtual int next_layer()
	{
		++current_layer;
		if (current_layer < number_of_layers)
			return 1;
		current_layer = 0; // reset
		return 0;
	}

	virtual int get_current_layer() const
	{
		return current_layer;
	}

	virtual int set_highlight_functor(SubObjectGroupHighlightFunctor *functor)
	{
		if (highlight_functor)
			delete highlight_functor;
		highlight_functor = functor;
		return 1;
	}

	double get_point_unit_size_pixels() const
	{
		return this->point_unit_size_pixels;
	}

	void set_point_unit_size_pixels(double in_point_unit_size_pixels)
	{
		if (in_point_unit_size_pixels > 0.0)
		{
			this->point_unit_size_pixels = in_point_unit_size_pixels;
		}
	}

	// override to avoid including fixed point size and line width in display lists
	virtual void Graphics_object_execute_point_size(GT_object *graphics_object);

}; /* class Render_graphics_opengl */

/***************************************************************************//**
 * Factory function to create a renderer that uses immediate mode
 * glBegin/glEnd calls to render primitives.
 */
Render_graphics_opengl *Render_graphics_opengl_create_glbeginend_renderer();

/***************************************************************************//**
 * Factory function to create a renderer that compiles objects into display lists
 * for rendering and uses glBegin/glEnd calls when compiling primitives.
 */
Render_graphics_opengl *Render_graphics_opengl_create_glbeginend_display_list_renderer();

/***************************************************************************//**
 * Factory function to create a renderer that uses immediate mode
 * client vertex arrays to render primitives.
 */
Render_graphics_opengl *Render_graphics_opengl_create_client_vertex_arrays_renderer();

/***************************************************************************//**
 * Factory function to create a renderer that compiles objects into display lists
 * for rendering and uses client vertex arrays when compiling primitives.
 */
Render_graphics_opengl *Render_graphics_opengl_create_client_vertex_arrays_display_list_renderer();

/***************************************************************************//**
 * Factory function to create a renderer that uses immediate mode
 * vertex buffer objects to render primitives.
 */
Render_graphics_opengl *Render_graphics_opengl_create_vertex_buffer_object_renderer();

/***************************************************************************//**
 * Factory function to create a renderer that compiles objects into display lists
 * for rendering and uses vertex buffer objects when compiling primitives.
 */
Render_graphics_opengl *Render_graphics_opengl_create_vertex_buffer_object_display_list_renderer();

#endif /* !defined (RENDERGL_HPP) */
