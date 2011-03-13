/*******************************************************************************
FILE : user_interface.h

LAST MODIFIED : 28 February 2005

DESCRIPTION :
Function definitions for the user interface.
???DB.  The main purpose is to have a graphical user interface, but the
	possibility of a command line should be kept in mind.
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
 * Portions created by the Initial Developer are Copyright (C) 2005
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
#if !defined (USER_INTERFACE_H)
#define USER_INTERFACE_H

#if defined (BUILD_WITH_CMAKE)
#include "configure/cmgui_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */

#if defined (MOTIF_USER_INTERFACE)
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Mrm/MrmPublic.h>
#endif /* defined (MOTIF_USER_INTERFACE) */
#if defined (WIN32_USER_INTERFACE) || defined (_MSC_VER)
//#define WINDOWS_LEAN_AND_MEAN
#if !defined (NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>
	/*???DB.  Contains lots of convenience macros */
#endif /* defined (WIN32_USER_INTERFACE) || defined (_MSC_VER) */
#if defined (GTK_USER_INTERFACE)
#include <gtk/gtk.h>
#endif /* defined (GTK_USER_INTERFACE) */
#if defined (CARBON_USER_INTERFACE)
#include <carbon/carbon.h>
#endif /* defined (CARBON_USER_INTERFACE) */
#include "general/machine.h"

/*
Global types
------------
*/
struct Event_dispatcher;

struct User_interface;
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/

#if defined (MOTIF_USER_INTERFACE)
typedef int (*Property_notify_callback)(XPropertyEvent *,void *,
	struct User_interface *);
#endif /* defined (MOTIF_USER_INTERFACE) */

/*
Global Functions
----------------
*/
#if defined (MOTIF_USER_INTERFACE)
int x_error_handler(Display *display, XErrorEvent *error);
/*******************************************************************************
LAST MODIFIED : 15 September 1999 

DESCRIPTION :
Responds to nonfatal XErrors and allows cmgui to continue.
==============================================================================*/
#endif /* defined (MOTIF_USER_INTERFACE) */

struct Shell_list_item *create_Shell_list_item(
#if defined (MOTIF_USER_INTERFACE)
	Widget *shell_address,
#endif /* defined (MOTIF_USER_INTERFACE) */
	struct User_interface *user_interface
	);
/*******************************************************************************
LAST MODIFIED : 25 March 1997

DESCRIPTION :
This function allocates memory for a shell list item, initializes the <shell>
field to the specified value and adds the item to the beginning of the shell
list.  It returns a pointer to the created item if successful and NULL if
unsuccessful.
???DB.  Move in with windowing macros ?
==============================================================================*/

int destroy_Shell_list_item(struct Shell_list_item **list_item);
/*******************************************************************************
LAST MODIFIED : 4 June 1999

DESCRIPTION :
This function removes the <list_item> from the shell list and frees the memory
for the <list_item>.  <*list_item> is set to NULL.

???SAB.  Seems unnessary to insist that all the windows keep track of the Shell
list item for the shell list, rather pass the respective shell and the list can
find the appropriate list object and remove it.
???DB.  Move in with windowing macros ?
==============================================================================*/

#if defined (MOTIF_USER_INTERFACE)
int destroy_Shell_list_item_from_shell(Widget *shell,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 19 May 1998

DESCRIPTION :
This function removes the list_item which refers to <shell> from the shell list
and frees the memory for the <list_item>.
???DB.  Move in with windowing macros ?
==============================================================================*/
#endif /* defined (MOTIF_USER_INTERFACE) */

#if defined (MOTIF_USER_INTERFACE)
void destroy_window_shell(Widget widget,XtPointer list_item,
	XtPointer call_data);
/*******************************************************************************
LAST MODIFIED : 25 March 1997

DESCRIPTION :
This function removes the <list_item> from the shell list, frees the memory
for the <list_item> and sets <*(list_item->address)> to NULL.
???DB.  Move in with windowing macros ?
SAB This is probably not a good way to do this, requires XmNuserData to point
to the User_interface structure.  Prefer to have each window with it's own
structure which it the calls destroy_Shell_list_item
==============================================================================*/
#endif /* defined (MOTIF_USER_INTERFACE) */

int busy_cursor_on(
#if defined (MOTIF_USER_INTERFACE)
	Widget excluded_shell,
#endif /* defined (MOTIF_USER_INTERFACE) */
	struct User_interface *user_interface
	);
/*******************************************************************************
LAST MODIFIED : 29 April 1998

DESCRIPTION :
Switchs from the default cursor to the busy cursor for all shells except the
<excluded_shell>.
???DB.  Move in with windowing macros ?
==============================================================================*/

int busy_cursor_off(
#if defined (MOTIF_USER_INTERFACE)
	Widget excluded_shell,
#endif /* defined (MOTIF_USER_INTERFACE) */
	struct User_interface *user_interface
	);
/*******************************************************************************
LAST MODIFIED : 29 April 1998

DESCRIPTION :
Switchs from the busy cursor to the default cursor for all shells except the
<excluded_shell>.
???DB.  Move in with windowing macros ?
==============================================================================*/

#if !defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER)
struct User_interface *CREATE(User_interface)(int *argc_address, char **argv, 
	struct Event_dispatcher *event_dispatcher, const char *class_name, 
	const char *application_name);
#else /* !defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER) */
struct User_interface *CREATE(User_interface)(HINSTANCE current_instance,
	HINSTANCE previous_instance, LPSTR command_line,int initial_main_window_state,
	int *argc_address, char **argv, struct Event_dispatcher *event_dispatcher);
#endif /* !defined (WIN32_USER_INTERFACE && !defined (_MSC_VER)) */
/*******************************************************************************
LAST MODIFIED : 20 June 2002

DESCRIPTION :
Open the <user_interface>.
==============================================================================*/

int DESTROY(User_interface)(struct User_interface **user_interface);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/

int User_interface_end_application_loop(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 7 July 2000

DESCRIPTION :
==============================================================================*/

#if defined (WX_USER_INTERFACE)
int User_interface_wx_main_loop(void);
/*******************************************************************************
LAST MODIFIED : 8 November 2006

DESCRIPTION :
==============================================================================*/
#endif /* defined (WX_USER_INTERFACE) */

#if defined (MOTIF_USER_INTERFACE)
Widget User_interface_get_application_shell(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Returns the application shell widget
==============================================================================*/
#endif /* defined (MOTIF_USER_INTERFACE) */

#if defined (MOTIF_USER_INTERFACE)
Display *User_interface_get_display(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Returns the application shell widget
==============================================================================*/
#endif /* defined (MOTIF_USER_INTERFACE) */

#if defined (MOTIF_USER_INTERFACE)
int User_interface_get_screen_width(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Returns the application shell widget
==============================================================================*/
#endif /* defined (MOTIF_USER_INTERFACE) */

#if defined (MOTIF_USER_INTERFACE)
int User_interface_get_screen_height(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Returns the application shell widget
==============================================================================*/
#endif /* defined (MOTIF_USER_INTERFACE) */

#if defined (MOTIF_USER_INTERFACE) || defined (WIN32_USER_INTERFACE)
int User_interface_get_widget_spacing(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Returns the application shell widget
==============================================================================*/
#endif /* defined (MOTIF_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) */

#if defined (MOTIF_USER_INTERFACE)
XFontStruct *User_interface_get_normal_font(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Returns the application shell widget
==============================================================================*/
#endif /* defined (MOTIF_USER_INTERFACE) */

#if defined (MOTIF_USER_INTERFACE)
XmFontList User_interface_get_normal_fontlist(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Returns the application shell widget
==============================================================================*/
#endif /* defined (MOTIF_USER_INTERFACE) */

#if defined (MOTIF_USER_INTERFACE)
XmFontList User_interface_get_button_fontlist(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Returns the application shell widget
==============================================================================*/
#endif /* defined (MOTIF_USER_INTERFACE) */

#if defined (MOTIF_USER_INTERFACE)
Pixmap User_interface_get_no_cascade_pixmap(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 6 March 2002

DESCRIPTION :
Returns a pixmap to avoid large gaps on the right of cascade buttons (option menus)
==============================================================================*/
#endif /* defined (MOTIF_USER_INTERFACE) */

#if defined (WIN32_USER_INTERFACE)
HINSTANCE User_interface_get_instance(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 20 June 2002

DESCRIPTION :
Returns the application shell widget
==============================================================================*/
#endif /* defined (WIN32_USER_INTERFACE) */

#if defined (GTK_USER_INTERFACE)
GtkWidget *User_interface_get_main_window(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 9 July 2002

DESCRIPTION :
Returns the main window widget
==============================================================================*/
#endif /* defined (GTK_USER_INTERFACE) */

int User_interface_get_local_machine_name(struct User_interface *user_interface,
	char **name);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
If the local machine name is know ALLOCATES and returns a string containing that
name.
==============================================================================*/

struct Event_dispatcher *User_interface_get_event_dispatcher(
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/

int application_main_step(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 5 June 1998

DESCRIPTION :
Performs one step of the application_main_step update allowing the programmer
to execute the same main loop elsewhere under special conditions (i.e. waiting
for a response from a modal dialog).
==============================================================================*/

int
#if defined (WIN32_USER_INTERFACE)
	WINAPI
#endif /* defined (WIN32_USER_INTERFACE) */
	application_main_loop(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 30 May 1996

DESCRIPTION :
???DB.  Does the main window need to be passed to this ?
???DB.  Should we have our own "WINAPI" (size specifier) ?
==============================================================================*/

#if defined (MOTIF_USER_INTERFACE)
int set_property_notify_callback(struct User_interface *user_interface,
	Property_notify_callback property_notify_callback,void *property_notify_data,
	Widget widget);
/*******************************************************************************
LAST MODIFIED : 18 November 1997

DESCRIPTION :
Sets the <property_notify_callback> for the <user_interface>.  This is used for
communication with other applications.
???DB.  At present only one (not a list)
==============================================================================*/
#endif /* defined (MOTIF_USER_INTERFACE) */

#if defined (MOTIF_USER_INTERFACE)
int MrmOpenHierarchy_binary_string(char *binary_string, int string_length,
	MrmHierarchy *hierarchy,int *hierarchy_open);
/*******************************************************************************
LAST MODIFIED : 23 December 2005

DESCRIPTION :
This wrapper allows the passing of the <binary_string> which contains a uid file.
<string_length> is required as the binary string may contain NULL characters.
This function writes it to a temporary file which is read using the normal 
MrmOpenHierarchy.
This allows the uid binaries to be kept inside the executable rather than bound
at run time!
If <*hierarchy_open> then 1 is returned, otherwise the full <uid_file_names> are
constructed and the <hierarchy> for those files opened.  1 is returned for
success and 0 for failure.
==============================================================================*/
#endif /* defined (MOTIF_USER_INTERFACE) */

#if defined (MOTIF_USER_INTERFACE)
int MrmOpenHierarchy_binary_multiple_strings(int number_of_strings, 
	char **binary_strings, int *string_lengths,
	MrmHierarchy *hierarchy,int *hierarchy_open);
/*******************************************************************************
LAST MODIFIED : 23 December 2005

DESCRIPTION :
This wrapper allows the passing of an array of <binary_strings> which are 
intended to contain a uid files.  
The <string_lengths> are required as the binary string may contain NULL characters.
This function writes a temporary file which is read using the normal 
MrmOpenHierarchy.
This allows the uid binaries to be kept inside the executable rather than bound
at run time!
If <*hierarchy_open> then 1 is returned, otherwise the full <uid_file_names> are
constructed and the <hierarchy> for those files opened.  1 is returned for
success and 0 for failure.
==============================================================================*/
#endif /* defined (MOTIF_USER_INTERFACE) */

#if defined (MOTIF_USER_INTERFACE)
int install_accelerators(Widget widget, Widget top_widget);
/*******************************************************************************
LAST MODIFIED : 24 December 1998

DESCRIPTION :
This travels down the widget tree from <widget> and installs all
accelerators in any subwidgets of <top_widget> in every appropriate subwidget of
<widget>.
==============================================================================*/
#endif /* defined (MOTIF_USER_INTERFACE) */
#endif /* !defined (USER_INTERFACE_H) */
