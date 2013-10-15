/**
 * FILE : scenepicker.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_SCENEPICKER_HPP__
#define CMZN_SCENEPICKER_HPP__

#include "zinc/scenepicker.h"
#include "zinc/element.hpp"
#include "zinc/fieldgroup.hpp"
#include "zinc/graphics.hpp"
#include "zinc/node.hpp"
#include "zinc/scene.hpp"
#include "zinc/scenefilter.hpp"
#include "zinc/sceneviewer.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class ScenePicker
{
protected:
	cmzn_scene_picker_id id;

public:

	ScenePicker() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit ScenePicker(cmzn_scene_picker_id in_scene_picker_id) :
		id(in_scene_picker_id)
	{  }

	ScenePicker(const ScenePicker& scene_picker) :
		id(cmzn_scene_picker_access(scene_picker.id))
	{  }

	ScenePicker& operator=(const ScenePicker& scene_picker)
	{
		cmzn_scene_picker_id temp_id = cmzn_scene_picker_access(scene_picker.id);
		if (0 != id)
		{
			cmzn_scene_picker_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~ScenePicker()
	{
		if (0 != id)
		{
			cmzn_scene_picker_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_scene_picker_id getId()
	{
		return id;
	}

	int setSceneviewerRectangle(Sceneviewer& sceneviewer, SceneCoordinateSystem coordinateSystem, double x1,
		double y1, double x2, double y2)
	{
		return cmzn_scene_picker_set_sceneviewer_rectangle(
			id , sceneviewer.getId(),
			static_cast<cmzn_scene_coordinate_system>(coordinateSystem),
			x1, y1, x2, y2);
	}

	Element getNearestElement()
	{
		return Element(cmzn_scene_picker_get_nearest_element(id));
	}

	Node getNearestNode()
	{
		return Node(cmzn_scene_picker_get_nearest_node(id));
	}

	Graphics getNearestElementGraphics()
	{
		return Graphics(cmzn_scene_picker_get_nearest_element_graphics(id));
	}

	Graphics getNearestNodeGraphics()
	{
		return Graphics(cmzn_scene_picker_get_nearest_node_graphics(id));
	}

	Graphics getNearestGraphics()
	{
		return Graphics(cmzn_scene_picker_get_nearest_graphics(id));
	}

	int addPickedElementsToGroup(FieldGroup& fieldGroup)
	{
		return cmzn_scene_picker_add_picked_elements_to_group(id,
			(reinterpret_cast<cmzn_field_group_id>(fieldGroup.getId())));
	}

	int addPickedNodesToGroup(FieldGroup& fieldGroup)
	{
		return cmzn_scene_picker_add_picked_nodes_to_group(id,
			(reinterpret_cast<cmzn_field_group_id>(fieldGroup.getId())));
	}

	Scene getScene()
	{
		return Scene(cmzn_scene_picker_get_scene(id));
	}

	int setScene(Scene& scene)
	{
		return cmzn_scene_picker_set_scene(id, scene.getId());
	}

	Scenefilter getScenefilter()
	{
		return Scenefilter(cmzn_scene_picker_get_scenefilter(id));
	}

	int setScenefilter(Scenefilter& filter)
	{
		return cmzn_scene_picker_set_scenefilter(id, filter.getId());
	}

};

inline ScenePicker Scene::createPicker()
{
	return ScenePicker(cmzn_scene_create_picker(id));
}

}  // namespace Zinc
}

#endif
