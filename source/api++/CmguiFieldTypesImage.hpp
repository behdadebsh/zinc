/***************************************************************************//**
 * FILE : CmguiFieldTypesImage.hpp
 */
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
#ifndef __CMGUI_FIELD_TYPES_IMAGE_HPP__
#define __CMGUI_FIELD_TYPES_IMAGE_HPP__

extern "C" {
#include "api/cmiss_field_image.h"
}
#include "api++/CmguiField.hpp"

namespace Cmgui
{

class FieldImage : public Field
{
public:
	// takes ownership of C-style field reference
	FieldImage(Cmiss_field_image_id field_image_id) :
		Field(reinterpret_cast<Cmiss_field_id>(field_image_id))
	{ }

	// casting constructor: must check isValid()
	FieldImage(Field& field) :
		Field(reinterpret_cast<Cmiss_field_id>(Cmiss_field_cast_image(field.getId())))
	{	}

	int readFile(const char *fileName)
	{
		return Cmiss_field_image_read_file(
			reinterpret_cast<Cmiss_field_image_id>(id), fileName);
	}

	int writeFile(const char *fileName) const
	{
		return Cmiss_field_image_write_file(
			reinterpret_cast<Cmiss_field_image_id>(id), fileName);
	}

};

} // namespace Cmgui

#endif /* __CMGUI_FIELD_TYPES_IMAGE_HPP__ */
