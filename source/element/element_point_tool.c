/*******************************************************************************
FILE : element_point_tool.c

LAST MODIFIED : 5 July 2002

DESCRIPTION :
Interactive tool for selecting element/grid points with mouse and other devices.
==============================================================================*/
#include <Xm/Protocols.h>
#include <Xm/MwmUtil.h>
#include <Xm/Xm.h>
#include <Xm/ToggleBG.h>
#include "choose/choose_computed_field.h"
#include "command/command.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#include "element/element_point_tool.h"
#include "element/element_point_tool.uidh"
#include "finite_element/finite_element_discretization.h"
#include "general/debug.h"
#include "graphics/scene.h"
#include "help/help_interface.h"
#include "interaction/interaction_graphics.h"
#include "interaction/interaction_volume.h"
#include "interaction/interactive_event.h"
#include "motif/image_utilities.h"
#include "user_interface/gui_dialog_macros.h"
#include "user_interface/message.h"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int element_point_tool_hierarchy_open=0;
static MrmHierarchy element_point_tool_hierarchy;
#endif /* defined (MOTIF) */

static char Interactive_tool_element_point_type_string[] = "element_point_tool";


/*
Module types
------------
*/

struct Element_point_tool
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Object storing all the parameters for interactively selecting element points.
==============================================================================*/
{
	struct Execute_command *execute_command;
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	struct Interactive_tool *interactive_tool;
	struct Element_point_ranges_selection *element_point_ranges_selection;
	struct Graphical_material *rubber_band_material;
	struct Time_keeper *time_keeper;
	struct User_interface *user_interface;
	/* user settings */
	struct Computed_field *command_field;
	/* information about picked element_point_ranges */
	int picked_element_point_was_unselected;
	int motion_detected;
	struct Element_point_ranges *last_picked_element_point;
	struct Interaction_volume *last_interaction_volume;
	struct GT_object *rubber_band;
#if defined (MOTIF)
	Display *display;
#endif /* defined (MOTIF) */

	Widget command_field_button, command_field_form, command_field_widget;
	Widget widget, window_shell;
}; /* struct Element_point_tool */

/*
Module functions
----------------
*/

DECLARE_DIALOG_IDENTIFY_FUNCTION(element_point_tool,Element_point_tool,command_field_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(element_point_tool,Element_point_tool,command_field_form)

static void Element_point_tool_close_CB(Widget widget,void *element_point_tool_void,
	void *call_data)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Callback when "close" is selected from the window menu, or it is double
clicked. How this is made to occur is as follows. The dialog has its
XmNdeleteResponse == XmDO_NOTHING, and a window manager protocol callback for
WM_DELETE_WINDOW has been set up with XmAddWMProtocolCallback to call this
function in response to the close command. See CREATE for more details.
Function pops down dialog as a response,
==============================================================================*/
{
	struct Element_point_tool *element_point_tool;

	ENTER(Element_point_tool_close_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (element_point_tool=(struct Element_point_tool *)element_point_tool_void)
	{
		XtPopdown(element_point_tool->window_shell);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_tool_close_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_point_tool_close_CB */

static void Element_point_tool_command_field_button_CB(Widget widget,
	void *element_point_tool_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Callback from toggle button enabling a command_field to be selected.
==============================================================================*/
{
	struct Computed_field *command_field;
	struct Element_point_tool *element_point_tool;

	ENTER(Element_point_tool_command_field_button_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (element_point_tool=(struct Element_point_tool *)element_point_tool_void)
	{
		if (Element_point_tool_get_command_field(element_point_tool))
		{
			Element_point_tool_set_command_field(element_point_tool, (struct Computed_field *)NULL);
		}
		else
		{
			/* get label field from widget */
			command_field = CHOOSE_OBJECT_GET_OBJECT(Computed_field)(
				element_point_tool->command_field_widget);
			if (command_field)
			{
				Element_point_tool_set_command_field(element_point_tool, command_field);
			}
			else
			{
				XtVaSetValues(element_point_tool->command_field_button, XmNset, False, NULL);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_tool_command_field_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_point_tool_command_field_button_CB */

static void Element_point_tool_update_command_field(Widget widget,
	void *element_point_tool_void, void *command_field_void)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Callback for change of command_field.
==============================================================================*/
{
	struct Element_point_tool *element_point_tool;

	ENTER(Element_point_tool_update_command_field);
	USE_PARAMETER(widget);
	if (element_point_tool = (struct Element_point_tool *)element_point_tool_void)
	{
		/* skip messages from chooser if it is grayed out */
		if (XtIsSensitive(element_point_tool->command_field_widget))
		{
			Element_point_tool_set_command_field(element_point_tool,
				(struct Computed_field *)command_field_void);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_tool_update_command_field.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_point_tool_update_command_field */

static void Element_point_tool_interactive_event_handler(void *device_id,
	struct Interactive_event *event,void *element_point_tool_void)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Input handler for input from devices. <device_id> is a unique address enabling
the editor to handle input from more than one device at a time. The <event>
describes the type of event, button numbers and key modifiers, and the volume
of space affected by the interaction. Main events are button press, movement and
release.
==============================================================================*/
{
	char *command_string;
	enum Interactive_event_type event_type;
	FE_value time, xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int clear_selection, input_modifier, shift_pressed, start, stop;
	struct Element_point_ranges *picked_element_point;
	struct Element_point_ranges_identifier element_point_ranges_identifier;
	struct Element_point_tool *element_point_tool;
	struct Interaction_volume *interaction_volume,*temp_interaction_volume;
	struct LIST(Element_point_ranges) *element_point_ranges_list;
	struct LIST(Scene_picked_object) *scene_picked_object_list;
	struct Multi_range *multi_range;
	struct Scene *scene;

	ENTER(Element_point_tool_interactive_event_handler);
	if (device_id&&event&&(element_point_tool=
		(struct Element_point_tool *)element_point_tool_void))
	{
		interaction_volume=Interactive_event_get_interaction_volume(event);
		if (scene=Interactive_event_get_scene(event))
		{
			event_type=Interactive_event_get_type(event);
			input_modifier=Interactive_event_get_input_modifier(event);
			shift_pressed=(INTERACTIVE_EVENT_MODIFIER_SHIFT & input_modifier);
			switch (event_type)
			{
				case INTERACTIVE_EVENT_BUTTON_PRESS:
				{
					/* interaction only works with first mouse button */
					if (1==Interactive_event_get_button_number(event))
					{
						if (scene_picked_object_list=
							Scene_pick_objects(scene,interaction_volume))
						{
							element_point_tool->picked_element_point_was_unselected=0;
							if (picked_element_point=
								Scene_picked_object_list_get_nearest_element_point(
									scene_picked_object_list,(struct Cmiss_region *)NULL,
									(struct Scene_picked_object **)NULL,
									(struct GT_element_group **)NULL,
									(struct GT_element_settings **)NULL))
							{
								/* Execute command_field of picked_element_point */
								if (element_point_tool->command_field)
								{
									if (Element_point_ranges_get_identifier(
										picked_element_point, &element_point_ranges_identifier))
									{
										if (Computed_field_is_defined_in_element(
											element_point_tool->command_field,
											element_point_ranges_identifier.element))
										{
											if (element_point_tool->time_keeper)
											{
												time =
													Time_keeper_get_time(element_point_tool->time_keeper);
											}
											else
											{
												time = 0;
											}
											/* need to get the xi for the picked_element_point
												 in order to evaluate the command_field there */
											if ((multi_range = Element_point_ranges_get_ranges(
												picked_element_point)) &&
												(Multi_range_get_range(multi_range, /*range_no*/0,
													&start, &stop)) &&
												FE_element_get_numbered_xi_point(
													element_point_ranges_identifier.element,
													element_point_ranges_identifier.xi_discretization_mode,
													element_point_ranges_identifier.number_in_xi,
													element_point_ranges_identifier.exact_xi,
													/*coordinate_field*/(struct Computed_field *)NULL,
													/*density_field*/(struct Computed_field *)NULL,
													start, xi, time))
											{
												if (command_string =
													Computed_field_evaluate_as_string_in_element(
														element_point_tool->command_field,
														/*component_number*/-1,
														element_point_ranges_identifier.element, xi, time,
														element_point_ranges_identifier.top_level_element))
												{
													Execute_command_execute_string(element_point_tool->execute_command,
														command_string);
													DEALLOCATE(command_string);
												}
											}
										}
									}
								}
								if (!Element_point_ranges_selection_is_element_point_ranges_selected(
									element_point_tool->element_point_ranges_selection,
									picked_element_point))
								{
									element_point_tool->picked_element_point_was_unselected=1;
								}
							}
							REACCESS(Element_point_ranges)(
								&(element_point_tool->last_picked_element_point),
								picked_element_point);
							if (clear_selection = !shift_pressed)
#if defined (OLD_CODE)
								&&((!picked_element_point)||
									(element_point_tool->picked_element_point_was_unselected))))
#endif /*defined (OLD_CODE) */
							{
								Element_point_ranges_selection_begin_cache(
									element_point_tool->element_point_ranges_selection);
								Element_point_ranges_selection_clear(
									element_point_tool->element_point_ranges_selection);
							}
							if (picked_element_point)
							{
								Element_point_ranges_selection_select_element_point_ranges(
									element_point_tool->element_point_ranges_selection,
									picked_element_point);
							}
							if (clear_selection)
							{
								Element_point_ranges_selection_end_cache(
									element_point_tool->element_point_ranges_selection);
							}
							DESTROY(LIST(Scene_picked_object))(&(scene_picked_object_list));
						}
						element_point_tool->motion_detected=0;
						REACCESS(Interaction_volume)(
							&(element_point_tool->last_interaction_volume),
							interaction_volume);
					}
				} break;
				case INTERACTIVE_EVENT_MOTION_NOTIFY:
				case INTERACTIVE_EVENT_BUTTON_RELEASE:
				{
					if (element_point_tool->last_interaction_volume&&
						((INTERACTIVE_EVENT_MOTION_NOTIFY==event_type) ||
							(1==Interactive_event_get_button_number(event))))
					{
						if (INTERACTIVE_EVENT_MOTION_NOTIFY==event_type)
						{
							element_point_tool->motion_detected=1;
						}
						if (element_point_tool->last_picked_element_point)
						{
							/* unselect last_picked_element_point if not just added */
							if ((INTERACTIVE_EVENT_BUTTON_RELEASE==event_type)&&
								shift_pressed&&
								(!(element_point_tool->picked_element_point_was_unselected)))
							{
								Element_point_ranges_selection_unselect_element_point_ranges(
									element_point_tool->element_point_ranges_selection,
									element_point_tool->last_picked_element_point);
							}
						}
						else if (element_point_tool->motion_detected)
						{
							/* rubber band select */
							if (temp_interaction_volume=
								create_Interaction_volume_bounding_box(
									element_point_tool->last_interaction_volume,
									interaction_volume))
							{
								if (INTERACTIVE_EVENT_MOTION_NOTIFY==event_type)
								{
									if (!element_point_tool->rubber_band)
									{
										/* create rubber_band object and put in scene */
										element_point_tool->rubber_band=CREATE(GT_object)(
											"element_point_tool_rubber_band",g_POLYLINE,
											element_point_tool->rubber_band_material);
										ACCESS(GT_object)(element_point_tool->rubber_band);
										Scene_add_graphics_object(scene,
											element_point_tool->rubber_band,/*position*/0,
											"element_point_tool_rubber_band",/*fast_changing*/1);
									}
									Interaction_volume_make_polyline_extents(
										temp_interaction_volume,element_point_tool->rubber_band);
								}
								else
								{
									Scene_remove_graphics_object(scene,
										element_point_tool->rubber_band);
									DEACCESS(GT_object)(&(element_point_tool->rubber_band));
								}
								if (INTERACTIVE_EVENT_BUTTON_RELEASE==event_type)
								{
									if (scene_picked_object_list=
										Scene_pick_objects(scene,temp_interaction_volume))
									{
										if (element_point_ranges_list=
											Scene_picked_object_list_get_picked_element_points(
												scene_picked_object_list))
										{
											Element_point_ranges_selection_begin_cache(
												element_point_tool->element_point_ranges_selection);
											FOR_EACH_OBJECT_IN_LIST(Element_point_ranges)(
												Element_point_ranges_select,(void *)
												element_point_tool->element_point_ranges_selection,
												element_point_ranges_list);
											Element_point_ranges_selection_end_cache(
												element_point_tool->element_point_ranges_selection);
											DESTROY(LIST(Element_point_ranges))(
												&element_point_ranges_list);
										}
										DESTROY(LIST(Scene_picked_object))(
											&(scene_picked_object_list));
									}
								}
								DESTROY(Interaction_volume)(&temp_interaction_volume);
							}
						}
						if (INTERACTIVE_EVENT_BUTTON_RELEASE==event_type)
						{
							REACCESS(Element_point_ranges)(
								&(element_point_tool->last_picked_element_point),
								(struct Element_point_ranges *)NULL);
							REACCESS(Interaction_volume)(
								&(element_point_tool->last_interaction_volume),
								(struct Interaction_volume *)NULL);
						}
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Element_point_tool_interactive_event_handler.  "
						"Unknown event type");
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_tool_interactive_event_handler.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_point_tool_interactive_event_handler */

static int Element_point_tool_bring_up_interactive_tool_dialog(
	void *element_point_tool_void)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Brings up a dialog for editing settings of the Element_point_tool - in a standard
format for passing to an Interactive_toolbar.
==============================================================================*/
{
	int return_code;

	ENTER(Element_point_tool_bring_up_interactive_tool_dialog);
	return_code =
		Element_point_tool_pop_up_dialog((struct Element_point_tool *)element_point_tool_void);
	LEAVE;

	return (return_code);
} /* Element_point_tool_bring_up_interactive_tool_dialog */

static struct Cmgui_image *Element_point_tool_get_icon(struct Colour *foreground, 
	struct Colour *background, void *element_point_tool_void)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Fetches an icon for the Element_point tool.
==============================================================================*/
{
	Display *display;
	Pixel background_pixel, foreground_pixel;
	Pixmap pixmap;
	struct Cmgui_image *image;
	struct Element_point_tool *element_point_tool;

	ENTER(Element_point_tool_get_icon);
	if ((element_point_tool=(struct Element_point_tool *)element_point_tool_void))
	{
		if (MrmOpenHierarchy_base64_string(element_point_tool_uidh,
			&element_point_tool_hierarchy,&element_point_tool_hierarchy_open))
		{
			display = element_point_tool->display;
			convert_Colour_to_Pixel(display, foreground, &foreground_pixel);
			convert_Colour_to_Pixel(display, background, &background_pixel);
			if (MrmSUCCESS == MrmFetchIconLiteral(element_point_tool_hierarchy,
				"element_point_tool_icon",DefaultScreenOfDisplay(display),display,
				foreground_pixel, background_pixel, &pixmap))
			{ 
				image = create_Cmgui_image_from_Pixmap(display, pixmap);
			}
			else
			{
				display_message(WARNING_MESSAGE, "Element_point_tool_get_icon.  "
					"Could not fetch widget");
				image = (struct Cmgui_image *)NULL;
			}			
		}
		else
		{
			display_message(WARNING_MESSAGE, "Element_point_tool_get_icon.  "
				"Could not open heirarchy");
			image = (struct Cmgui_image *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_tool_get_icon.  Invalid argument_point(s)");
		image = (struct Cmgui_image *)NULL;
	}
	LEAVE;

	return (image);
} /* Element_point_tool_get_icon */

/*
Global functions
----------------
*/

struct Element_point_tool *CREATE(Element_point_tool)(
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct Computed_field_package *computed_field_package,
	struct Graphical_material *rubber_band_material,
	struct User_interface *user_interface,
	struct Time_keeper *time_keeper,
	struct Execute_command *execute_command)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Creates an Element_point_tool with Interactive_tool in
<interactive_tool_manager>. Selects element/grid points in
<element_point_ranges_selection> in response to interactive_events.
==============================================================================*/
{
	Atom WM_DELETE_WINDOW;
	int init_widgets;
	MrmType element_point_tool_dialog_class;
	static MrmRegisterArg callback_list[]=
	{
		{"elem_pnt_tool_id_cmd_field_btn",(XtPointer)
			DIALOG_IDENTIFY(element_point_tool,command_field_button)},
		{"elem_pnt_tool_id_cmd_field_form",(XtPointer)
			DIALOG_IDENTIFY(element_point_tool,command_field_form)},
		{"elem_pnt_tool_cmd_field_btn_CB",
		 (XtPointer)Element_point_tool_command_field_button_CB}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"elem_pnt_tool_structure",(XtPointer)NULL}
	};
	struct Callback_data callback;
	struct Element_point_tool *element_point_tool;
	struct MANAGER(Computed_field) *computed_field_manager;

	ENTER(CREATE(Element_point_tool));
	if (interactive_tool_manager&&element_point_ranges_selection&&
		computed_field_package&&(computed_field_manager=
			Computed_field_package_get_computed_field_manager(computed_field_package))
		&&rubber_band_material&&user_interface&&execute_command)
	{
		if (MrmOpenHierarchy_base64_string(element_point_tool_uidh,
			&element_point_tool_hierarchy,&element_point_tool_hierarchy_open))
		{
			if (ALLOCATE(element_point_tool,struct Element_point_tool,1))
			{
				element_point_tool->execute_command=execute_command;
				element_point_tool->display = User_interface_get_display
				   (user_interface);
				element_point_tool->interactive_tool_manager=interactive_tool_manager;
				element_point_tool->element_point_ranges_selection=
					element_point_ranges_selection;
				element_point_tool->rubber_band_material=
					ACCESS(Graphical_material)(rubber_band_material);
				element_point_tool->user_interface=user_interface;
				element_point_tool->time_keeper = (struct Time_keeper *)NULL;
				if (time_keeper)
				{
					element_point_tool->time_keeper = ACCESS(Time_keeper)(time_keeper);
				}
				/* user settings */
				element_point_tool->command_field = (struct Computed_field *)NULL;
				/* interactive_tool */
				element_point_tool->interactive_tool=CREATE(Interactive_tool)(
					"element_point_tool","Element point tool",
					Interactive_tool_element_point_type_string,
					Element_point_tool_interactive_event_handler,
					Element_point_tool_get_icon,
					Element_point_tool_bring_up_interactive_tool_dialog,
					(Interactive_tool_destroy_tool_data_function *)NULL,
					(void *)element_point_tool);
				ADD_OBJECT_TO_MANAGER(Interactive_tool)(
					element_point_tool->interactive_tool,
					element_point_tool->interactive_tool_manager);
				element_point_tool->last_picked_element_point=
					(struct Element_point_ranges *)NULL;
				element_point_tool->last_interaction_volume=
					(struct Interaction_volume *)NULL;
				element_point_tool->rubber_band=(struct GT_object *)NULL;
				/* initialise widgets */
				element_point_tool->command_field_button=(Widget)NULL;
				element_point_tool->command_field_form=(Widget)NULL;
				element_point_tool->command_field_widget=(Widget)NULL;
				element_point_tool->widget=(Widget)NULL;
				element_point_tool->window_shell=(Widget)NULL;

				/* make the dialog shell */
				if (element_point_tool->window_shell=
					XtVaCreatePopupShell("Element point tool",
						topLevelShellWidgetClass,
						User_interface_get_application_shell(user_interface),
						XmNdeleteResponse,XmDO_NOTHING,
						XmNmwmDecorations,MWM_DECOR_ALL,
						XmNmwmFunctions,MWM_FUNC_ALL,
						/*XmNtransient,FALSE,*/
						XmNallowShellResize,False,
						XmNtitle,"Element point tool",
						NULL))
				{
					/* Set up window manager callback for close window message */
					WM_DELETE_WINDOW=XmInternAtom(XtDisplay(element_point_tool->window_shell),
						"WM_DELETE_WINDOW",False);
					XmAddWMProtocolCallback(element_point_tool->window_shell,
						WM_DELETE_WINDOW,Element_point_tool_close_CB,element_point_tool);
					/* Register the shell with the busy signal list */
					create_Shell_list_item(&(element_point_tool->window_shell),user_interface);
					/* register the callbacks */
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
						element_point_tool_hierarchy,callback_list,XtNumber(callback_list)))
					{
						/* assign and register the identifiers */
						identifier_list[0].value=(XtPointer)element_point_tool;
						if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
							element_point_tool_hierarchy,identifier_list,XtNumber(identifier_list)))
						{
							/* fetch element tool widgets */
							if (MrmSUCCESS==MrmFetchWidget(element_point_tool_hierarchy,
								"element_point_tool",element_point_tool->window_shell,
								&(element_point_tool->widget),&element_point_tool_dialog_class))
							{
								init_widgets=1;
								if (element_point_tool->command_field_widget =
									CREATE_CHOOSE_OBJECT_WIDGET(Computed_field)(
										element_point_tool->command_field_form,
										element_point_tool->command_field, computed_field_manager,
										Computed_field_has_string_value_type,
										(void *)NULL, user_interface))
								{
									callback.data = (void *)element_point_tool;
									callback.procedure = Element_point_tool_update_command_field;
									CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
										element_point_tool->command_field_widget, &callback);
								}
								else
								{
									init_widgets=0;
								}
								if (init_widgets)
								{
									XmToggleButtonGadgetSetState(
										element_point_tool->command_field_button,
										/*state*/False, /*notify*/False);
									XtSetSensitive(element_point_tool->command_field_widget,
										/*state*/False);
									XtManageChild(element_point_tool->widget);
									XtRealizeWidget(element_point_tool->window_shell);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"CREATE(Element_point_tool).  Could not init widgets");
									DESTROY(Element_point_tool)(&element_point_tool);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"CREATE(Element_point_tool).  Could not fetch element_point_tool");
								DESTROY(Element_point_tool)(&element_point_tool);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"CREATE(Element_point_tool).  Could not register identifiers");
							DESTROY(Element_point_tool)(&element_point_tool);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"CREATE(Element_point_tool).  Could not register callbacks");
						DESTROY(Element_point_tool)(&element_point_tool);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"CREATE(Element_point_tool).  Could not create Shell");
					DESTROY(Element_point_tool)(&element_point_tool);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"CREATE(Element_point_tool).  Not enough memory");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Element_point_tool).  Could not open hierarchy");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Element_point_tool).  Invalid argument(s)");
		element_point_tool=(struct Element_point_tool *)NULL;
	}
	LEAVE;

	return (element_point_tool);
} /* CREATE(Element_point_tool) */

int DESTROY(Element_point_tool)(
	struct Element_point_tool **element_point_tool_address)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Frees and deaccesses objects in the <element_point_tool> and deallocates the
structure itself.
==============================================================================*/
{
	struct Element_point_tool *element_point_tool;
	int return_code;

	ENTER(DESTROY(Element_point_tool));
	if (element_point_tool_address&&
		(element_point_tool= *element_point_tool_address))
	{
		REACCESS(Element_point_ranges)(
			&(element_point_tool->last_picked_element_point),
			(struct Element_point_ranges *)NULL);
		REMOVE_OBJECT_FROM_MANAGER(Interactive_tool)(
			element_point_tool->interactive_tool,
			element_point_tool->interactive_tool_manager);
		REACCESS(Interaction_volume)(
			&(element_point_tool->last_interaction_volume),
			(struct Interaction_volume *)NULL);
		REACCESS(GT_object)(&(element_point_tool->rubber_band),
			(struct GT_object *)NULL);
		DEACCESS(Graphical_material)(&(element_point_tool->rubber_band_material));
		if (element_point_tool->time_keeper)
		{
			DEACCESS(Time_keeper)(&(element_point_tool->time_keeper));
		}
		if (element_point_tool->window_shell)
		{
			destroy_Shell_list_item_from_shell(&(element_point_tool->window_shell),
				element_point_tool->user_interface);
			XtDestroyWidget(element_point_tool->window_shell);
		}
		DEALLOCATE(*element_point_tool_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Element_point_tool).  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* DESTROY(Element_point_tool) */

int Element_point_tool_pop_up_dialog(
	struct Element_point_tool *element_point_tool)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Pops up a dialog for editing settings of the Element_point_tool.
==============================================================================*/
{
	int return_code;

	ENTER(Element_point_tool_pop_up_dialog);
	if (element_point_tool)
	{
		XtPopup(element_point_tool->window_shell, XtGrabNone);
		/* make sure in addition that it is not shown as an icon */
		XtVaSetValues(element_point_tool->window_shell, XmNiconic, False, NULL);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_tool_pop_up_dialog.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_tool_pop_up_dialog */

int Element_point_tool_pop_down_dialog(
	struct Element_point_tool *element_point_tool)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Hides the dialog for editing settings of the Element_point_tool.
==============================================================================*/
{
	int return_code;

	ENTER(Element_point_tool_pop_down_dialog);
	if (element_point_tool)
	{
		XtPopdown(element_point_tool->window_shell);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_tool_pop_down_dialog.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_tool_pop_down_dialog */

struct Computed_field *Element_point_tool_get_command_field(
	struct Element_point_tool *element_point_tool)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Returns the command_field to be executed when the element is clicked on in the 
<element_point_tool>.
==============================================================================*/
{
	struct Computed_field *command_field;

	ENTER(Element_point_tool_get_command_field);
	if (element_point_tool)
	{
		command_field=element_point_tool->command_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_tool_get_command_field.  Invalid argument(s)");
		command_field=(struct Computed_field *)NULL;
	}
	LEAVE;

	return (command_field);
} /* Element_point_tool_get_command_field */

int Element_point_tool_set_command_field(
	struct Element_point_tool *element_point_tool,
	struct Computed_field *command_field)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Sets the command_field to be executed when the element is clicked on in the 
<element_point_tool>.
==============================================================================*/
{
	int field_set, return_code;

	ENTER(Element_point_tool_set_command_field);
	if (element_point_tool && ((!command_field) ||
		Computed_field_has_string_value_type(command_field, (void *)NULL)))
	{
		return_code = 1;
		if (command_field != element_point_tool->command_field)
		{
			element_point_tool->command_field = command_field;
			if (command_field)
			{
				CHOOSE_OBJECT_SET_OBJECT(Computed_field)(
					element_point_tool->command_field_widget,element_point_tool->command_field);
			}
			field_set = ((struct Computed_field *)NULL != command_field);
			XtVaSetValues(element_point_tool->command_field_button,
				XmNset, field_set, NULL);
			XtSetSensitive(element_point_tool->command_field_widget, field_set);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_tool_set_command_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_tool_set_command_field */
