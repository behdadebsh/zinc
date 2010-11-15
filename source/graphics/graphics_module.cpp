/*******************************************************************************
FILE : graphics_module.cpp

==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2010
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
extern "C" {
#include "general/debug.h"
#include "general/object.h"
#include "graphics/glyph.h"
#include "graphics/graphics_module.h"
#include "graphics/graphics_object.h"
#include "graphics/material.h"
#include "graphics/rendition.h"
#include "graphics/scene.h"
#include "graphics/spectrum.h"
}
#include "graphics/tessellation.hpp"
extern "C" {
#include "region/cmiss_region_private.h"
#include "time/time_keeper.h"
#include "user_interface/message.h"
}
#include <list>

struct Startup_material_definition
{
	const char *name;
	MATERIAL_PRECISION ambient[3];
	MATERIAL_PRECISION diffuse[3];
	MATERIAL_PRECISION emission[3];
	MATERIAL_PRECISION specular[3];
	MATERIAL_PRECISION alpha;
	MATERIAL_PRECISION shininess;
};

struct Cmiss_graphics_module
{
	/* attribute managers and defaults: */
	struct MANAGER(GT_object) *glyph_manager;
	struct Material_package *material_package;
	void *material_manager_callback_id;
	struct Graphics_font *default_font;
	struct Graphics_font_package *graphics_font_package;
	struct Light *default_light;
	struct MANAGER(Light) *light_manager;
	struct LIST(Light) *list_of_lights;
	struct MANAGER(Spectrum) *spectrum_manager;
	void *spectrum_manager_callback_id;
	struct Spectrum *default_spectrum;
	struct MANAGER(Scene) *scene_manager;
	struct Scene *default_scene;
	struct Light_model *default_light_model;
	struct MANAGER(Light_model) *light_model_manager;
	struct Element_point_ranges_selection *element_point_ranges_selection;
	struct FE_element_selection *element_selection;
	struct FE_node_selection *data_selection,*node_selection;
	struct Time_keeper *default_time_keeper;
	struct MANAGER(Cmiss_tessellation) *tessellation_manager;
	void *tessellation_manager_callback_id;
	struct Cmiss_tessellation *default_tessellation;
	int access_count;
	std::list<Cmiss_region*> *member_regions_list;
};

namespace {

/***************************************************************************//**
 * Callback for changes in the material manager.
 * Informs all renditions about the changes.
 */
void Cmiss_graphics_module_material_manager_callback(
	struct MANAGER_MESSAGE(Graphical_material) *message, void *graphics_module_void)
{
	Cmiss_graphics_module *graphics_module = (Cmiss_graphics_module *)graphics_module_void;
	if (message && graphics_module)
	{
		int change_summary = MANAGER_MESSAGE_GET_CHANGE_SUMMARY(Graphical_material)(message);
		if (change_summary & MANAGER_CHANGE_RESULT(Graphical_material))
		{
			// minimise scene messages while updating
			MANAGER_BEGIN_CACHE(Cmiss_scene)(graphics_module->scene_manager);
			std::list<Cmiss_region*>::iterator region_iter;
			for (region_iter = graphics_module->member_regions_list->begin();
				region_iter != graphics_module->member_regions_list->end(); ++region_iter)
			{
				Cmiss_region *region = *region_iter;
				Cmiss_rendition *rendition = Cmiss_graphics_module_get_rendition(graphics_module, region);
				Cmiss_rendition_material_change(rendition, message);
				DEACCESS(Cmiss_rendition)(&rendition);
			}
			MANAGER_END_CACHE(Cmiss_scene)(graphics_module->scene_manager);
		}
	}
}

/***************************************************************************//**
 * Callback for changes in the spectrum manager.
 * Informs all renditions about the changes.
 */
void Cmiss_graphics_module_spectrum_manager_callback(
	struct MANAGER_MESSAGE(Spectrum) *message, void *graphics_module_void)
{
	Cmiss_graphics_module *graphics_module = (Cmiss_graphics_module *)graphics_module_void;
	if (message && graphics_module)
	{
		int change_summary = MANAGER_MESSAGE_GET_CHANGE_SUMMARY(Spectrum)(message);
		if (change_summary & MANAGER_CHANGE_RESULT(Spectrum))
		{
			// minimise scene messages while updating
			MANAGER_BEGIN_CACHE(Cmiss_scene)(graphics_module->scene_manager);
			std::list<Cmiss_region*>::iterator region_iter;
			for (region_iter = graphics_module->member_regions_list->begin();
				region_iter != graphics_module->member_regions_list->end(); ++region_iter)
			{
				Cmiss_region *region = *region_iter;
				Cmiss_rendition *rendition = Cmiss_graphics_module_get_rendition(graphics_module, region);
				Cmiss_rendition_spectrum_change(rendition, message);
				DEACCESS(Cmiss_rendition)(&rendition);
			}
			MANAGER_END_CACHE(Cmiss_scene)(graphics_module->scene_manager);
		}
	}
}

/***************************************************************************//**
 * Callback for changes in the tessellation manager.
 * Informs all renditions about the changes.
 */
void Cmiss_graphics_module_tessellation_manager_callback(
	struct MANAGER_MESSAGE(Cmiss_tessellation) *message, void *graphics_module_void)
{
	Cmiss_graphics_module *graphics_module = (Cmiss_graphics_module *)graphics_module_void;
	if (message && graphics_module)
	{
		int change_summary = MANAGER_MESSAGE_GET_CHANGE_SUMMARY(Cmiss_tessellation)(message);
		if (change_summary & MANAGER_CHANGE_RESULT(Cmiss_tessellation))
		{
			// minimise scene messages while updating
			MANAGER_BEGIN_CACHE(Cmiss_scene)(graphics_module->scene_manager);
			std::list<Cmiss_region*>::iterator region_iter;
			for (region_iter = graphics_module->member_regions_list->begin();
				region_iter != graphics_module->member_regions_list->end(); ++region_iter)
			{
				Cmiss_region *region = *region_iter;
				Cmiss_rendition *rendition = Cmiss_graphics_module_get_rendition(graphics_module, region);
				Cmiss_rendition_tessellation_change(rendition, message);
				DEACCESS(Cmiss_rendition)(&rendition);
			}
			MANAGER_END_CACHE(Cmiss_scene)(graphics_module->scene_manager);
		}
	}
}

}

struct Cmiss_graphics_module *Cmiss_graphics_module_create(
	struct Context *context)
{
	struct Cmiss_graphics_module *module;

	ENTER(Cmiss_rendtion_graphics_module_create);
	if (context)
	{
		if (ALLOCATE(module, struct Cmiss_graphics_module, 1))
		{
			module->light_manager = NULL;
			module->spectrum_manager = NULL;
			module->material_package = NULL;
			module->list_of_lights = NULL;
			module->glyph_manager = NULL;
			module->default_font = NULL;
			module->default_light = NULL;
			module->default_spectrum = NULL;
			module->graphics_font_package = NULL;
			module->default_scene = NULL;
			module->default_light_model = NULL;
			module->light_manager=CREATE(MANAGER(Light))();
			module->spectrum_manager=CREATE(MANAGER(Spectrum))();
			Spectrum_manager_set_owner(module->spectrum_manager, module);
			module->material_package = ACCESS(Material_package)(CREATE(Material_package)
				(Cmiss_context_get_default_region(context), module->spectrum_manager));
			Material_manager_set_owner(
				Material_package_get_material_manager(module->material_package), module);
			module->material_manager_callback_id =
				MANAGER_REGISTER(Graphical_material)(Cmiss_graphics_module_material_manager_callback,
					(void *)module, Material_package_get_material_manager(module->material_package));
			module->spectrum_manager_callback_id =
				MANAGER_REGISTER(Spectrum)(Cmiss_graphics_module_spectrum_manager_callback,
					(void *)module, module->spectrum_manager);
			module->scene_manager = CREATE(MANAGER(Scene)());
			module->light_model_manager = CREATE(MANAGER(Light_model)());
			module->element_point_ranges_selection = Cmiss_context_get_element_point_ranges_selection(context);
			module->element_selection = Cmiss_context_get_element_selection(context);
			module->data_selection = Cmiss_context_get_data_selection(context);
			module->node_selection = Cmiss_context_get_node_selection(context);
			module->default_time_keeper = NULL;
			module->tessellation_manager = CREATE(MANAGER(Cmiss_tessellation))();
			Cmiss_tessellation_manager_set_owner_private(module->tessellation_manager, module);
			module->default_tessellation = NULL;
			module->member_regions_list = new std::list<Cmiss_region*>;
			module->tessellation_manager_callback_id =
				MANAGER_REGISTER(Cmiss_tessellation)(Cmiss_graphics_module_tessellation_manager_callback,
					(void *)module, module->tessellation_manager);
			module->access_count = 1;
		}
		else
		{
			module = (Cmiss_graphics_module *)NULL;
			display_message(ERROR_MESSAGE,
			"Cmiss_rendtion_graphics_module_create. Not enough memory for Cmiss rendition graphics module");
		}
	}
	else
	{
		module = (Cmiss_graphics_module *)NULL;
		display_message(ERROR_MESSAGE,"Cmiss_rendtion_graphics_module_create.  Invalid argument(s)");
	}
	LEAVE;

	return (module);
}

struct Material_package *Cmiss_graphics_module_get_material_package(
	struct Cmiss_graphics_module *graphics_module)
{
	struct Material_package *material_package = NULL;
	if (graphics_module && graphics_module->material_package)
	{
		material_package = ACCESS(Material_package)(graphics_module->material_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphics_module_get_material_package.  Invalid argument(s)");
	}

	return material_package;
}

struct Cmiss_graphics_module *Cmiss_graphics_module_access(
	struct Cmiss_graphics_module *graphics_module)
{
	if (graphics_module)
	{
		graphics_module->access_count++;
	}
	return graphics_module;
}

int Cmiss_graphics_module_remove_member_regions_rendition(
	struct Cmiss_graphics_module *graphics_module)
{
	int return_code = 0;

	if (graphics_module && graphics_module->member_regions_list)
	{
		std::list<Cmiss_region*>::iterator pos;
		for (pos = graphics_module->member_regions_list->begin();
				pos != graphics_module->member_regions_list->end(); ++pos)
		{
			Cmiss_region_deaccess_rendition(*pos);
		}
		graphics_module->member_regions_list->clear();
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphics_module_remove_member_regions_rendition.  "
			"Invalid argument(s)");
	}

	return return_code;
}

int Cmiss_graphics_module_destroy(
	struct Cmiss_graphics_module **graphics_module_address)
{
	int return_code = 0;
	struct Cmiss_graphics_module *graphics_module = NULL;

	if (NULL != (graphics_module = *graphics_module_address))
	{
		graphics_module->access_count--;
		if (0 == graphics_module->access_count)
		{
			MANAGER_DEREGISTER(Graphical_material)(
				graphics_module->material_manager_callback_id, Material_package_get_material_manager(graphics_module->material_package));
			MANAGER_DEREGISTER(Spectrum)(
				graphics_module->spectrum_manager_callback_id, graphics_module->spectrum_manager);
			MANAGER_DEREGISTER(Cmiss_tessellation)(
				graphics_module->tessellation_manager_callback_id,	graphics_module->tessellation_manager);
			if (graphics_module->member_regions_list)
			{
				Cmiss_graphics_module_remove_member_regions_rendition(graphics_module);
				delete graphics_module->member_regions_list;
			}
			if (graphics_module->default_scene)
				DEACCESS(Scene)(&graphics_module->default_scene);
 			if (graphics_module->scene_manager)
 				DESTROY(MANAGER(Scene))(&graphics_module->scene_manager);
 			if (graphics_module->glyph_manager)
 				DESTROY(MANAGER(GT_object))(&graphics_module->glyph_manager);
			if (graphics_module->default_light)
				DEACCESS(Light)(&graphics_module->default_light);
			if (graphics_module->light_manager)
				DESTROY(MANAGER(Light))(&graphics_module->light_manager);
			if (graphics_module->default_light_model)
				DEACCESS(Light_model)(&graphics_module->default_light_model);
			if (graphics_module->light_model_manager)
				DESTROY(MANAGER(Light_model))(&graphics_module->light_model_manager);
			if (graphics_module->default_spectrum)
				DEACCESS(Spectrum)(&graphics_module->default_spectrum);
			if (graphics_module->spectrum_manager)
				DESTROY(MANAGER(Spectrum))(&graphics_module->spectrum_manager);
			if (graphics_module->material_package)
				DEACCESS(Material_package)(&graphics_module->material_package);
			if (graphics_module->default_font)
				DEACCESS(Graphics_font)(&graphics_module->default_font);
			if (graphics_module->default_time_keeper)
				DEACCESS(Time_keeper)(&graphics_module->default_time_keeper);
			if (graphics_module->graphics_font_package)
				DESTROY(Graphics_font_package)(&graphics_module->graphics_font_package);
			if (graphics_module->default_tessellation)
				DEACCESS(Cmiss_tessellation)(&graphics_module->default_tessellation);
			DESTROY(MANAGER(Cmiss_tessellation))(&graphics_module->tessellation_manager);
			DEALLOCATE(*graphics_module_address);
		}
		*graphics_module_address = NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphics_module_destroy.  Missing graphics module");
		return_code = 0;
	}

	return return_code;
}

int Cmiss_graphics_module_create_rendition(
	struct Cmiss_graphics_module *graphics_module,
	struct Cmiss_region *cmiss_region)
{
	struct Cmiss_rendition *rendition;
	int return_code;

	ENTER(Cmiss_region_add_rendition);
	if (cmiss_region && graphics_module)
	{
		rendition = FIRST_OBJECT_IN_LIST_THAT(ANY_OBJECT(Cmiss_rendition))(
			(ANY_OBJECT_CONDITIONAL_FUNCTION(Cmiss_rendition) *)NULL, (void *)NULL,
			Cmiss_region_private_get_any_object_list(cmiss_region));
		if (!(rendition))
		{
			if (NULL != (rendition = Cmiss_rendition_create_internal(cmiss_region, graphics_module)))
			{
				Cmiss_rendition_set_position(rendition, 1);
				return_code = 1;
			}
			else
			{
				return_code = 0;
				display_message(ERROR_MESSAGE,
					"Cmiss_region_add_rendition. Cannot create rendition for region");
			}
		}
		else
		{
			return_code = 1;
			//ACCESS or not ?
			//ACCESS(Cmiss_rendition)(rendition);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_add_rendition. Invalid argument(s).");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

struct MANAGER(Light) *Cmiss_graphics_module_get_light_manager(
	struct Cmiss_graphics_module *graphics_module)
{
	struct MANAGER(Light) *light_manager = NULL;
	if (graphics_module)
	{
		light_manager = graphics_module->light_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphics_module_get_light_manager.  Invalid argument(s)");
	}
	return light_manager;
}

struct Light *Cmiss_graphics_module_get_default_light(
	struct Cmiss_graphics_module *graphics_module)
{
	struct Light *light = NULL;
	if (graphics_module && graphics_module->light_manager)
	{
		if (!graphics_module->default_light)
		{
			graphics_module->default_light=CREATE(Light)("default");
			if (graphics_module->default_light)
			{
				float default_light_direction[3]={0.0,-0.5,-1.0};
				struct Colour default_colour;
				set_Light_type(graphics_module->default_light,INFINITE_LIGHT);
				default_colour.red=1.0;
				default_colour.green=1.0;
				default_colour.blue=1.0;
				set_Light_colour(graphics_module->default_light,&default_colour);

				set_Light_direction(graphics_module->default_light,default_light_direction);
				/*???DB.  Include default as part of manager ? */
				ACCESS(Light)(graphics_module->default_light);
				if (!ADD_OBJECT_TO_MANAGER(Light)(graphics_module->default_light,
						graphics_module->light_manager))
				{
					DEACCESS(Light)(&(graphics_module->default_light));
				}
			}
		}
		if (graphics_module->default_light)
		{
			light = ACCESS(Light)(graphics_module->default_light);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphics_module_get_default_light.  Invalid argument(s)");
	}

	return light;
}

struct MANAGER(Spectrum) *Cmiss_graphics_module_get_spectrum_manager(
	struct Cmiss_graphics_module *graphics_module)
{
	struct MANAGER(Spectrum) *spectrum_manager = NULL;
	if (graphics_module)
	{
		spectrum_manager = graphics_module->spectrum_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphics_module_get_spectrum_manager.  Invalid argument(s)");
	}

	return spectrum_manager;
}

struct Spectrum *Cmiss_graphics_module_get_default_spectrum(
	struct Cmiss_graphics_module *graphics_module)
{
	struct Spectrum *spectrum = NULL;

	if (graphics_module && graphics_module->spectrum_manager)
	{
		if (!graphics_module->default_spectrum)
		{
			graphics_module->default_spectrum=CREATE(Spectrum)("default");
			if (graphics_module->default_spectrum)
			{
				Spectrum_set_simple_type(graphics_module->default_spectrum,
					BLUE_TO_RED_SPECTRUM);
				Spectrum_set_minimum_and_maximum(graphics_module->default_spectrum,0,1);
				if (!ADD_OBJECT_TO_MANAGER(Spectrum)(graphics_module->default_spectrum,
						graphics_module->spectrum_manager))
				{
					DEACCESS(Spectrum)(&(graphics_module->default_spectrum));
				}
			}
		}
		if (graphics_module->default_spectrum)
		{
			spectrum = ACCESS(Spectrum)(graphics_module->default_spectrum);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphics_module_get_default_spectrum.  Invalid argument(s)");
	}

	return spectrum;
}

Cmiss_spectrum_id Cmiss_graphics_module_find_spectrum_by_name(
	Cmiss_graphics_module_id graphics_module, const char *name)
{
	Cmiss_spectrum_id spectrum = NULL;

	ENTER(Cmiss_graphics_module_find_spectrum_by_name);
	if (graphics_module && name)
	{
		struct MANAGER(Spectrum) *spectrum_manager =
			Cmiss_graphics_module_get_spectrum_manager(graphics_module);
		if (spectrum_manager)
		{
			if (NULL != (spectrum=FIND_BY_IDENTIFIER_IN_MANAGER(Spectrum, name)(
										 name, spectrum_manager)))
			{
				ACCESS(Spectrum)(spectrum);
			}
		}
	}
	LEAVE;

	return spectrum;
}

Cmiss_spectrum_id Cmiss_graphics_module_create_spectrum(
	Cmiss_graphics_module_id graphics_module)
{
	Cmiss_spectrum_id spectrum = NULL;
	struct MANAGER(Spectrum) *spectrum_manager =
		Cmiss_graphics_module_get_spectrum_manager(graphics_module);
	int i = 0;
	char *temp_string = NULL;
	char *num = NULL;

	ENTER(Cmiss_graphics_module_create_spectrum);
	do
	{
		if (temp_string)
		{
			DEALLOCATE(temp_string);
		}
		ALLOCATE(temp_string, char, 18);
		strcpy(temp_string, "temp_spectrum");
		num = strrchr(temp_string, 'm') + 1;
		sprintf(num, "%i", i);
		strcat(temp_string, "\0");
		i++;
	}
	while (FIND_BY_IDENTIFIER_IN_MANAGER(Spectrum, name)(temp_string,
			spectrum_manager));

	if (temp_string)
	{
		if (NULL != (spectrum = CREATE(Spectrum)(temp_string)))
		{
			if (ADD_OBJECT_TO_MANAGER(Spectrum)(
						spectrum, spectrum_manager))
			{
				ACCESS(Spectrum)(spectrum);
			}
			else
			{
				DESTROY(Spectrum)(&spectrum);
			}
		}
		DEALLOCATE(temp_string);
	}
	LEAVE;

	return spectrum;
}

int Cmiss_graphics_module_create_standard_materials(
	struct Cmiss_graphics_module *graphics_module)
{
	/* only the default material is not in this list because its colour changes
		 to contrast with the background; colours are R G B */

	struct Startup_material_definition
		startup_materials[] =
		{
			{"black",
			 /*ambient*/ { 0.00, 0.00, 0.00},
			 /*diffuse*/ { 0.00, 0.00, 0.00},
			 /*emission*/{ 0.00, 0.00, 0.00},
			 /*specular*/{ 0.30, 0.30, 0.30},
	 /*alpha*/1.0,
			 /*shininess*/0.2},
			{"blue",
			 /*ambient*/ { 0.00, 0.00, 0.50},
			 /*diffuse*/ { 0.00, 0.00, 1.00},
			 /*emission*/{ 0.00, 0.00, 0.00},
			 /*specular*/{ 0.20, 0.20, 0.20},
			 /*alpha*/1.0,
			 /*shininess*/0.2},
			{"bone",
			 /*ambient*/ { 0.70, 0.70, 0.60},
			 /*diffuse*/ { 0.90, 0.90, 0.70},
			 /*emission*/{ 0.00, 0.00, 0.00},
			 /*specular*/{ 0.10, 0.10, 0.10},
			 /*alpha*/1.0,
			 /*shininess*/0.2},
			{"gray50",
			 /*ambient*/ { 0.50, 0.50, 0.50},
			 /*diffuse*/ { 0.50, 0.50, 0.50},
			 /*emission*/{ 0.50, 0.50, 0.50},
			 /*specular*/{ 0.50, 0.50, 0.50},
			 /*alpha*/1.0,
			 /*shininess*/0.2},
			{"gold",
			 /*ambient*/ { 1.00, 0.40, 0.00},
			 /*diffuse*/ { 1.00, 0.70, 0.00},
			 /*emission*/{ 0.00, 0.00, 0.00},
			 /*specular*/{ 0.50, 0.50, 0.50},
			 /*alpha*/1.0,
			 /*shininess*/0.3},
			{"green",
			 /*ambient*/ { 0.00, 0.50, 0.00},
			 /*diffuse*/ { 0.00, 1.00, 0.00},
			 /*emission*/{ 0.00, 0.00, 0.00},
			 /*specular*/{ 0.20, 0.20, 0.20},
			 /*alpha*/1.0,
			 /*shininess*/0.1},
			{"muscle",
			 /*ambient*/ { 0.40, 0.14, 0.11},
			 /*diffuse*/ { 0.50, 0.12, 0.10},
			 /*emission*/{ 0.00, 0.00, 0.00},
			 /*specular*/{ 0.30, 0.50, 0.50},
			 /*alpha*/1.0,
			 /*shininess*/0.2},
			{"red",
			 /*ambient*/ { 0.50, 0.00, 0.00},
			 /*diffuse*/ { 1.00, 0.00, 0.00},
			 /*emission*/{ 0.00, 0.00, 0.00},
			 /*specular*/{ 0.20, 0.20, 0.20},
			 /*alpha*/1.0,
			 /*shininess*/0.2},
			{"silver",
			 /*ambient*/ { 0.40, 0.40, 0.40},
			 /*diffuse*/ { 0.70, 0.70, 0.70},
			 /*emission*/{ 0.00, 0.00, 0.00},
			 /*specular*/{ 0.50, 0.50, 0.50},
			 /*alpha*/1.0,
			 /*shininess*/0.3},
			{"tissue",
			 /*ambient*/ { 0.90, 0.70, 0.50},
			 /*diffuse*/ { 0.90, 0.70, 0.50},
			 /*emission*/{ 0.00, 0.00, 0.00},
			 /*specular*/{ 0.20, 0.20, 0.30},
			 /*alpha*/1.0,
			 /*shininess*/0.2},
			/* Used as the default fail_material for texture evaluation. */
			{"transparent_gray50",
			 /*ambient*/ { 0.50, 0.50, 0.50},
			 /*diffuse*/ { 0.50, 0.50, 0.50},
			 /*emission*/{ 0.50, 0.50, 0.50},
			 /*specular*/{ 0.50, 0.50, 0.50},
			 /*alpha*/0.0,
			 /*shininess*/0.2},
			{"white",
			 /*ambient*/ { 1.00, 1.00, 1.00},
			 /*diffuse*/ { 1.00, 1.00, 1.00},
			 /*emission*/{ 0.00, 0.00, 0.00},
			 /*specular*/{ 0.00, 0.00, 0.00},
			 /*alpha*/1.0,
			 /*shininess*/0.0}
		};
	int i, return_code;
	int number_of_startup_materials = sizeof(startup_materials) /
		sizeof(struct Startup_material_definition);
	struct Graphical_material *material;
	struct Colour colour;

	if (graphics_module && graphics_module->material_package)
	{
		for (i = 0; i < number_of_startup_materials; i++)
		{
			if (NULL != (material = CREATE(Graphical_material)(startup_materials[i].name)))
			{
					colour.red   = startup_materials[i].ambient[0];
					colour.green = startup_materials[i].ambient[1];
					colour.blue  = startup_materials[i].ambient[2];
					Graphical_material_set_ambient(material, &colour);
					colour.red   = startup_materials[i].diffuse[0];
					colour.green = startup_materials[i].diffuse[1];
					colour.blue  = startup_materials[i].diffuse[2];
					Graphical_material_set_diffuse(material, &colour);
					colour.red   = startup_materials[i].emission[0];
					colour.green = startup_materials[i].emission[1];
					colour.blue  = startup_materials[i].emission[2];
					Graphical_material_set_emission(material, &colour);
					colour.red   = startup_materials[i].specular[0];
					colour.green = startup_materials[i].specular[1];
					colour.blue  = startup_materials[i].specular[2];
					Graphical_material_set_specular(material, &colour);
					Graphical_material_set_alpha(material, startup_materials[i].alpha);
					Graphical_material_set_shininess(material,
						startup_materials[i].shininess);
					if (!Material_package_manage_material(graphics_module->material_package,
							material))
					{
					DESTROY(Graphical_material)(&material);
				}
			}
		}

		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphics_module_create_standard_material.  Invalid argument(s)");
		return_code = 0;
	}

	return return_code;
}

struct Graphics_font_package *Cmiss_graphics_module_get_font_package(
	struct Cmiss_graphics_module *graphics_module)
{
	struct Graphics_font_package *graphics_font_package = NULL;
	if (graphics_module)
	{
		if (!graphics_module->graphics_font_package)
		{
			graphics_module->graphics_font_package=CREATE(Graphics_font_package)();
		}
		graphics_font_package = graphics_module->graphics_font_package;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphics_module_get_font_package.  Invalid argument(s)");
	}

	return (graphics_font_package);
}

struct Graphics_font *Cmiss_graphics_module_get_default_font(
	struct Cmiss_graphics_module *graphics_module)
{
	struct Graphics_font *default_font = NULL;
	if (graphics_module)
	{
		if (!graphics_module->default_font)
		{
			graphics_module->default_font=ACCESS(Graphics_font)(
				Graphics_font_package_get_font(Cmiss_graphics_module_get_font_package(graphics_module), "default"));
		}
		default_font = ACCESS(Graphics_font)(graphics_module->default_font);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphics_module_get_default_font.  Invalid argument(s)");
	}

	return (default_font);
}

struct MANAGER(GT_object) * Cmiss_graphics_module_get_default_glyph_manager(
		struct Cmiss_graphics_module *graphics_module)
{
	MANAGER(GT_object) *glyph_list = NULL;
	if (graphics_module)
	{
		if (!graphics_module->glyph_manager)
		{
			struct Graphics_font *default_font = Cmiss_graphics_module_get_default_font(
				graphics_module);
			graphics_module->glyph_manager=make_standard_glyphs(default_font);
			DEACCESS(Graphics_font)(&default_font);
		}
		glyph_list = graphics_module->glyph_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphics_module_get_default_glyph_manager.  Invalid argument(s)");
	}
	return (glyph_list);
}

struct MANAGER(Scene) *Cmiss_graphics_module_get_scene_manager(
		struct Cmiss_graphics_module *graphics_module)
{
	struct MANAGER(Scene) *scene_manager = NULL;

	if (graphics_module)
	{
		if (!graphics_module->scene_manager)
		{
			graphics_module->scene_manager=CREATE(MANAGER(Scene)());
		}
		scene_manager = graphics_module->scene_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphics_module_get_scene_manager.  Invalid argument(s)");
	}

	return scene_manager;
}

struct Scene *Cmiss_graphics_module_create_scene(
	struct Cmiss_graphics_module *graphics_module)
{
	Cmiss_scene_id scene = NULL;
	struct MANAGER(Scene) *scene_manager =
		Cmiss_graphics_module_get_scene_manager(graphics_module);
	int i = 0;
	char *temp_string = NULL;
	char *num = NULL;

	ENTER(Cmiss_graphics_module_create_scene);
	do
	{
		if (temp_string)
		{
			DEALLOCATE(temp_string);
		}
		ALLOCATE(temp_string, char, 18);
		strcpy(temp_string, "temp_scene");
		num = strrchr(temp_string, 'e') + 1;
		sprintf(num, "%i", i);
		strcat(temp_string, "\0");
		i++;
	}
	while (FIND_BY_IDENTIFIER_IN_MANAGER(Scene, name)(temp_string,
		scene_manager));

	if (temp_string)
	{
		if (NULL != (scene = CREATE(Scene)()))
		{
			Cmiss_scene_set_name(scene, temp_string);
			if (ADD_OBJECT_TO_MANAGER(Scene)(scene, scene_manager))
			{
				ACCESS(Scene)(scene);
			}
			else
			{
				DESTROY(Scene)(&scene);
			}
		}
		DEALLOCATE(temp_string);
	}
	LEAVE;

	return scene;
}

struct Scene *Cmiss_graphics_module_get_default_scene(
	struct Cmiss_graphics_module *graphics_module)
{
	struct Scene *scene = NULL;
	if (graphics_module)
	{
		if (!graphics_module->default_scene)
		{
			if (NULL != (graphics_module->default_scene=(CREATE(Scene)())))
			{
				Cmiss_scene_filter *filter =
					Cmiss_scene_create_filter_visibility_flags(graphics_module->default_scene);
				Cmiss_scene_filter_destroy(&filter);
				Cmiss_scene_set_name(graphics_module->default_scene, "default");
				struct MANAGER(Scene) *scene_manager =
					Cmiss_graphics_module_get_scene_manager(graphics_module);
				if (!ADD_OBJECT_TO_MANAGER(Scene)(graphics_module->default_scene,
						scene_manager))
				{
					DEACCESS(Scene)(&(graphics_module->default_scene));
				}
			}
		}
		if (graphics_module->default_scene)
			scene = ACCESS(Scene)(graphics_module->default_scene);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphics_module_get_default_scene.  Invalid argument(s)");
	}

	return scene;
}

struct MANAGER(Light_model) *Cmiss_graphics_module_get_light_model_manager(
	struct Cmiss_graphics_module *graphics_module)
{
	struct MANAGER(Light_model) *light_model_manager = NULL;
	if (graphics_module)
	{
		light_model_manager = graphics_module->light_model_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphics_module_get_light_model_manager.  Invalid argument(s)");
	}

	return light_model_manager;
}

struct Light_model *Cmiss_graphics_module_get_default_light_model(
	struct Cmiss_graphics_module *graphics_module)
{
	struct Light_model *light_model = NULL;

	if (graphics_module)
	{
		if (!graphics_module->default_light_model)
		{
			if (NULL != (graphics_module->default_light_model=CREATE(Light_model)("default")))
			{
				struct Colour ambient_colour;
				ambient_colour.red=0.2;
				ambient_colour.green=0.2;
				ambient_colour.blue=0.2;
				Light_model_set_ambient(graphics_module->default_light_model,&ambient_colour);
				Light_model_set_side_mode(graphics_module->default_light_model,
					LIGHT_MODEL_TWO_SIDED);
				ACCESS(Light_model)(graphics_module->default_light_model);
				if (!ADD_OBJECT_TO_MANAGER(Light_model)(
							graphics_module->default_light_model,graphics_module->light_model_manager))
				{
					DEACCESS(Light_model)(&(graphics_module->default_light_model));
				}
			}
		}
		if (graphics_module->default_light_model)
			light_model = ACCESS(Light_model)(graphics_module->default_light_model);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphics_module_get_default_light_model.  Invalid argument(s)");
	}
	return light_model;
}

struct MANAGER(Cmiss_tessellation) *Cmiss_graphics_module_get_tessellation_manager(
	struct Cmiss_graphics_module *graphics_module)
{
	struct MANAGER(Cmiss_tessellation) *tessellation_manager = NULL;
	if (graphics_module)
	{
		tessellation_manager = graphics_module->tessellation_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphics_module_get_tessellation_manager.  Invalid argument(s)");
	}
	return tessellation_manager;
}

struct Cmiss_tessellation *Cmiss_graphics_module_get_default_tessellation(
	struct Cmiss_graphics_module *graphics_module)
{
	const char *default_tessellation_name = "default";
	struct Cmiss_tessellation *tessellation = NULL;
	if (graphics_module && graphics_module->tessellation_manager)
	{
		if (!graphics_module->default_tessellation)
		{
			graphics_module->default_tessellation =
				Cmiss_graphics_module_find_tessellation_by_name(graphics_module, default_tessellation_name);
			if (NULL == graphics_module->default_tessellation)
			{
				graphics_module->default_tessellation = Cmiss_tessellation_create_private();
				Cmiss_tessellation_set_name(graphics_module->default_tessellation, default_tessellation_name);
				Cmiss_tessellation_set_persistent(tessellation, 1);

				const int default_minimum_divisions = 1;
				Cmiss_tessellation_set_minimum_divisions(graphics_module->default_tessellation,
					/*dimensions*/1, &default_minimum_divisions);
				const int default_refinement_factor = 4;
				Cmiss_tessellation_set_refinement_factors(graphics_module->default_tessellation,
					/*dimensions*/1, &default_refinement_factor);
				if (!ADD_OBJECT_TO_MANAGER(Cmiss_tessellation)(graphics_module->default_tessellation,
					graphics_module->tessellation_manager))
				{
					DEACCESS(Cmiss_tessellation)(&(graphics_module->default_tessellation));
				}
			}
		}
		tessellation = ACCESS(Cmiss_tessellation)(graphics_module->default_tessellation);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphics_module_get_default_tessellation.  Invalid argument(s)");
	}
	return tessellation;
}

struct Cmiss_tessellation* Cmiss_graphics_module_find_tessellation_by_name(
	Cmiss_graphics_module_id graphics_module, const char *name)
{
	struct Cmiss_tessellation *tessellation = NULL;
	if (graphics_module && name)
	{
		tessellation = FIND_BY_IDENTIFIER_IN_MANAGER(Cmiss_tessellation, name)(
			name, Cmiss_graphics_module_get_tessellation_manager(graphics_module));
		if (tessellation)
		{
			ACCESS(Cmiss_tessellation)(tessellation);
		}
	}
	return tessellation;
}

Cmiss_tessellation_id Cmiss_graphics_module_create_tessellation(
	Cmiss_graphics_module_id graphics_module)
{
	Cmiss_tessellation_id tessellation = NULL;
	if (graphics_module)
	{
		struct MANAGER(Cmiss_tessellation) *tessellation_manager =
			Cmiss_graphics_module_get_tessellation_manager(graphics_module);
		char temp_name[20];
		int i = NUMBER_IN_MANAGER(Cmiss_tessellation)(tessellation_manager);
		do
		{
			i++;
			sprintf(temp_name, "temp%d",i);
		}
		while (FIND_BY_IDENTIFIER_IN_MANAGER(Cmiss_tessellation,name)(temp_name,
			tessellation_manager));
		tessellation = Cmiss_tessellation_create_private();
		Cmiss_tessellation_set_name(tessellation, temp_name);
		if (!ADD_OBJECT_TO_MANAGER(Cmiss_tessellation)(tessellation, tessellation_manager))
		{
			DEACCESS(Cmiss_tessellation)(&tessellation);
		}
	}
	return tessellation;
}

int Cmiss_graphics_module_set_time_keeper_internal(struct Cmiss_graphics_module *module, struct Time_keeper *time_keeper)
{
	int return_code = 1;

	if (module && time_keeper && !(module->default_time_keeper))
	{
		module->default_time_keeper = ACCESS(Time_keeper)(time_keeper);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphics_module_set_time_keeper_internal.  Invalid argument(s)");
	}

	return return_code;
}

struct Time_keeper *Cmiss_graphics_module_get_time_keeper_internal(
	struct Cmiss_graphics_module *module)
{
	struct Time_keeper *time_keeper = NULL;

	if (module && module->default_time_keeper)
	{
		time_keeper = ACCESS(Time_keeper)(module->default_time_keeper);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphics_module_get_time_keeper_internal.  Invalid argument(s)");
	}

	return time_keeper;
}

int Cmiss_graphics_module_enable_renditions(
	struct Cmiss_graphics_module *graphics_module,
	struct Cmiss_region *cmiss_region)
{
	int return_code;
	struct Cmiss_region *child_region;

	ENTER(Cmiss_region_add_rendition);
	if (cmiss_region && graphics_module)
	{
		return_code = Cmiss_graphics_module_create_rendition(
			graphics_module, cmiss_region);
		if (return_code)
		{
			struct Cmiss_rendition *rendition =
				Cmiss_region_get_rendition_internal(cmiss_region);
			Cmiss_rendition_add_callback(rendition, Cmiss_rendition_update_callback,
				(void *)NULL);
			child_region = Cmiss_region_get_first_child(cmiss_region);
			while (child_region)
			{
				return_code = Cmiss_graphics_module_enable_renditions(
					graphics_module, child_region);
				/* add callback to call from child rendition to parent rendition */
				struct Cmiss_rendition *child;
				if (rendition && (NULL != (child = Cmiss_region_get_rendition_internal(child_region))))
				{
					Cmiss_rendition_add_callback(child,
						Cmiss_rendition_notify_parent_rendition_callback,
						(void *)cmiss_region);
					DEACCESS(Cmiss_rendition)(&child);
				}
				Cmiss_region_reaccess_next_sibling(&child_region);
			}
			DEACCESS(Cmiss_rendition)(&rendition);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_rendition.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

struct MANAGER(Graphical_material) *Cmiss_graphics_module_get_material_manager(
	struct Cmiss_graphics_module *graphics_module)
{
	struct MANAGER(Graphical_material) *material_manager;

	ENTER(Cmiss_graphics_module_get_material_manager);
	if (graphics_module && graphics_module->material_package)
	{
		material_manager = 	Material_package_get_material_manager(
			graphics_module->material_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphics_module_get_material_manager.  Invalid argument(s)");
		material_manager = (struct MANAGER(Graphical_material) *)NULL;
	}
	LEAVE;

	return (material_manager);
}

struct Element_point_ranges_selection *Cmiss_graphics_module_get_element_point_ranges_selection(
	struct Cmiss_graphics_module *graphics_module)
{
	struct Element_point_ranges_selection *element_point_ranges_selection = NULL;

	if (graphics_module && graphics_module->element_point_ranges_selection)
	{
		element_point_ranges_selection = graphics_module->element_point_ranges_selection;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphics_module_get_element_point_ranges_selection.  Invalid argument(s)");
	}

	return (element_point_ranges_selection);
}

struct FE_element_selection *Cmiss_graphics_module_get_element_selection(
	struct Cmiss_graphics_module *graphics_module)
{
	struct FE_element_selection *element_selection = NULL;

	if (graphics_module && graphics_module->element_selection)
	{
		element_selection = graphics_module->element_selection;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphics_module_get_element_selection.  Invalid argument(s)");
	}

	return (element_selection);
}

Cmiss_rendition_id Cmiss_graphics_module_get_rendition(
	Cmiss_graphics_module_id graphics_module, Cmiss_region_id region)
{
	struct Cmiss_rendition *rendition = NULL;
	if (graphics_module && region)
	{
		rendition = Cmiss_region_get_rendition_internal(region);
		if (!rendition)
		{
			char *region_path = Cmiss_region_get_path(region);
			display_message(ERROR_MESSAGE,
				"Cmiss_graphics_module_get_rendition.  Rendition not found for region %s", region_path);
			DEALLOCATE(region_path);
		}
	}
	return rendition;
}

Cmiss_material_id Cmiss_graphics_module_find_material_by_name(
	Cmiss_graphics_module_id graphics_module, const char *name)
{
	Cmiss_material_id material = NULL;

	ENTER(Cmiss_graphics_module_find_material_by_name);
	if (graphics_module && name)
	{
		struct MANAGER(Graphical_material) *material_manager =
			Cmiss_graphics_module_get_material_manager(graphics_module);
		if (material_manager)
		{
			if (NULL != (material=FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material, name)(
										 name, material_manager)))
			{
				ACCESS(Graphical_material)(material);
			}
		}
	}
	LEAVE;

	return material;
}

Cmiss_material_id Cmiss_graphics_module_create_material(
	Cmiss_graphics_module_id graphics_module)
{
	Cmiss_material_id material = NULL;
	struct MANAGER(Graphical_material) *material_manager =
		Cmiss_graphics_module_get_material_manager(graphics_module);
	int i = 0;
	char *temp_string = NULL;
	char *num = NULL;

	ENTER(Cmiss_graphics_module_create_material);
	do
	{
		if (temp_string)
		{
			DEALLOCATE(temp_string);
		}
		ALLOCATE(temp_string, char, 18);
		strcpy(temp_string, "temp_material");
		num = strrchr(temp_string, 'l') + 1;
		sprintf(num, "%i", i);
		strcat(temp_string, "\0");
		i++;
	}
	while (FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material, name)(temp_string,
			material_manager));

	if (temp_string)
	{
		if (NULL != (material = CREATE(Graphical_material)(temp_string)))
		{
			if (Material_package_manage_material(graphics_module->material_package,
					material))
			{
				ACCESS(Graphical_material)(material);
			}
			else
			{
				DESTROY(Graphical_material)(&material);
			}
		}
		DEALLOCATE(temp_string);
	}
	LEAVE;

	return material;
}

int Cmiss_graphics_module_add_member_region(
	struct Cmiss_graphics_module *graphics_module, struct Cmiss_region *region)
{
	int return_code = 0;

	if (graphics_module && region)
	{
		if (graphics_module->member_regions_list)
		{
			graphics_module->member_regions_list->push_back(region);
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphics_module_add_member_region.  Invalid argument(s)");
	}

	return return_code;
}

int Cmiss_graphics_module_remove_member_region(
		struct Cmiss_graphics_module *graphics_module, struct Cmiss_region *region)
{
	int return_code;

	ENTER(Cmiss_graphics_module_remove_member_region);
	if (graphics_module && graphics_module->member_regions_list && region)
	{
		graphics_module->member_regions_list->remove(region);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphics_module_remove_member_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}
