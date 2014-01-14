/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include "zinctestsetup.hpp"
#include <zinc/core.h>
#include <zinc/field.h>
#include <zinc/fieldcache.h>
#include <zinc/fieldcomposite.h>
#include <zinc/fieldconstant.h>

#include "zinctestsetupcpp.hpp"
#include <zinc/field.hpp>
#include <zinc/fieldcache.hpp>
#include <zinc/fieldcomposite.hpp>
#include <zinc/fieldconstant.hpp>

TEST(cmzn_fieldmodule_create_field_component, valid_args)
{
	ZincTestSetup zinc;

	const double values[] = { 2.0, 4.0, 6.0 };
	const int component_index = 2;
	cmzn_field_id f1 = cmzn_fieldmodule_create_field_constant(zinc.fm, 3, values);
	EXPECT_NE((cmzn_field_id)0, f1);

	cmzn_field_component_id notComponent = cmzn_field_cast_component(f1);
	EXPECT_EQ((cmzn_field_component_id)0, notComponent);

	cmzn_field_id f2 = cmzn_fieldmodule_create_field_component(zinc.fm, f1, component_index);
	EXPECT_NE((cmzn_field_id)0, f2);

	cmzn_field_component_id isComponent = cmzn_field_cast_component(f2);
	EXPECT_NE((cmzn_field_component_id)0, isComponent);
	cmzn_field_component_destroy(&isComponent);

	cmzn_fieldcache_id cache = cmzn_fieldmodule_create_fieldcache(zinc.fm);
	double value = 0.0;
	EXPECT_EQ(CMZN_OK, cmzn_field_evaluate_real(f2, cache, 1, &value));
	EXPECT_EQ(values[component_index - 1], value);
	cmzn_fieldcache_destroy(&cache);

	cmzn_field_destroy(&f1);
	cmzn_field_destroy(&f2);
}

TEST(cmzn_fieldmodule_create_field_component, invalid_args)
{
	ZincTestSetup zinc;
	const double values[] = { 2.0, 4.0, 6.0 };
	const int component_index = 2;
	cmzn_field_id f1 = cmzn_fieldmodule_create_field_constant(zinc.fm, 3, values);
	EXPECT_NE((cmzn_field_id)0, f1);

	EXPECT_EQ((cmzn_field_id)0, cmzn_fieldmodule_create_field_component(0, f1, component_index));
	EXPECT_EQ((cmzn_field_id)0, cmzn_fieldmodule_create_field_component(zinc.fm, 0, component_index));
	EXPECT_EQ((cmzn_field_id)0, cmzn_fieldmodule_create_field_component(zinc.fm, f1, 0));
	EXPECT_EQ((cmzn_field_id)0, cmzn_fieldmodule_create_field_component(zinc.fm, f1, 4));

	cmzn_field_destroy(&f1);
}

TEST(zincFieldModule_createComponent, valid_args)
{
	ZincTestSetupCpp zinc;

	const double values[] = { 2.0, 4.0, 6.0 };
	const int component_index = 2;
	FieldConstant f1 = zinc.fm.createFieldConstant(3, values);
	EXPECT_TRUE(f1.isValid());

	FieldComponent notComponent = f1.castComponent();
	EXPECT_FALSE(notComponent.isValid());

	FieldComponent f2 = zinc.fm.createFieldComponent(f1, component_index);
	EXPECT_TRUE(f2.isValid());

	FieldComponent isComponent = f2.castComponent();
	EXPECT_TRUE(isComponent.isValid());

	Fieldcache cache = zinc.fm.createFieldcache();
	double value = 0.0;
	EXPECT_EQ(CMZN_OK, f2.evaluateReal(cache, 1, &value));
	EXPECT_EQ(values[component_index - 1], value);
}

TEST(zincFieldComponent_setComponentIndex, valid_args)
{
	ZincTestSetupCpp zinc;

	const double values[] = { 2.0, 4.0, 6.0 };
	int component_index = 2;
	FieldConstant f1 = zinc.fm.createFieldConstant(3, values);
	EXPECT_TRUE(f1.isValid());

	FieldComponent f2 = zinc.fm.createFieldComponent(f1, component_index);
	EXPECT_TRUE(f2.isValid());

	Fieldcache cache = zinc.fm.createFieldcache();
	double value = 0.0;
	EXPECT_EQ(CMZN_OK, f2.evaluateReal(cache, 1, &value));
	EXPECT_EQ(values[component_index - 1], value);

	int temp_component_index = f2.getComponentIndex();
	EXPECT_EQ(component_index, temp_component_index);

	component_index = 3;

	EXPECT_EQ(CMZN_OK, f2.setComponentIndex(component_index));

	EXPECT_EQ(CMZN_OK, f2.evaluateReal(cache, 1, &value));
	EXPECT_EQ(values[component_index - 1], value);
}

TEST(zincFieldModule_createComponent, invalid_args)
{
	ZincTestSetupCpp zinc;

	const double values[] = { 2.0, 4.0, 6.0 };
	const int component_index = 2;
	FieldConstant f1 = zinc.fm.createFieldConstant(3, values);
	EXPECT_TRUE(f1.isValid());

	EXPECT_FALSE(zinc.fm.createFieldComponent(Field(), component_index).isValid());
	EXPECT_FALSE(zinc.fm.createFieldComponent(f1, 0).isValid());
	EXPECT_FALSE(zinc.fm.createFieldComponent(f1, 4).isValid());
}
