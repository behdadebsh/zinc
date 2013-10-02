/*******************************************************************************
FILE : fieldmatrixoperators.h

LAST MODIFIED : 17 June 2011

DESCRIPTION :
The public interface to the cmzn_fields that perform matrix operations.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDMATRIXOPERATORS_H__
#define CMZN_FIELDMATRIXOPERATORS_H__

#include "types/fieldid.h"
#include "types/fieldmoduleid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * Creates a field returning the scalar real determinant of a square matrix
 * source field. Only supports up to 3x3 matrix.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field  Field supplying square matrix up to 3x3. May only have
 * 1, 4 or 9 components.
 * @return  Newly created field.
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_determinant(
	cmzn_fieldmodule_id field_module, cmzn_field_id source_field);

/***************************************************************************//**
 * Creates a field returning the N eigenvalues of symmetric N*N component source
 * field.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field  N*N component square symmetric matrix field.
 * @return Newly created field
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_eigenvalues(
	cmzn_fieldmodule_id field_module,
	cmzn_field_id source_field);

/***************************************************************************//**
 * Creates a field returning the N, N-dimensional eigenvectors computed with the
 * source eigenvalues field. Sets the number of components equal to N*N, where
 * N is the number of components in the <eigenvalues_field>.
 *
 * @param field_module  Region field module which will own new field.
 * @param eigenvalues_field  Eigenvalues type field.
 * @return Newly created field
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_eigenvectors(
	cmzn_fieldmodule_id field_module,
	cmzn_field_id eigenvalues_field);

/***************************************************************************//**
 * Creates a field returning the inverse of N*N symmetric matrix valued source
 * field.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field  N*N component square symmetric matrix field.
 * @return Newly created field
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_matrix_invert(
	cmzn_fieldmodule_id field_module,
	cmzn_field_id source_field);

/***************************************************************************//**
 * Creates a field returning the values resulting from matrix multiplication
 * <source_field1> x <source_field2>, with <number_of_rows> rows in both
 * <source_field1> and the result. From the <number_of_rows> the columns in
 * <source_field1>, rows in <source_field2> and then columns in <source_field2>
 * are implied and checked.
 *
 * @param field_module  Region field module which will own new field.
 * @param number_of_rows  Number of rows N in source_field1 and result.
 * @param source_field1  N rows * M columns component matrix field 1.
 * @param source_field2  M rows * P columns component matrix field 2.
 * @return Newly created matrix with N*P components.
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_matrix_multiply(
	cmzn_fieldmodule_id field_module,
	int number_of_rows, cmzn_field_id source_field1,
	cmzn_field_id source_field2);

/***************************************************************************//**
 * Creates a projection field returning the result of a matrix multiplication
 * with perspective division on the source field vector. The source_field vector
 * is expanded to a homogeneous coordinate by appending a component of value 1,
 * which is multiplied by the projection_matrix_field, and the extra calculated
 * value resulting from the unit component is used to divide through each of the
 * other components to give a perspective projection in the resulting field.
 * The projection_matrix_field must have have a multiple of
 * (source_field->number_of_components + 1) components forming a matrix with
 * that many columns and the resulting (number_of_components + 1) rows. The
 * first values in the projection_matrix are across the first row, followed by
 * the next row and so on.
 * Hence a 4x4 matrix transforms a 3-component vector to a 3-component vector:
 * [x'] = [m1  m2  m3  m4 ][x]
 * [y'] = [m5  m6  m7  m8 ][y]
 * [z'] = [m9  m10 m11 m12][z]
 * [h'] = [m13 m14 m15 m16][1]
 * The resulting field returns 3 components [x'/h', y'/h', z'/h']
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field  Source vector field to project.
 * @param projection_matrix_field  Field supplying projection matrix.
 * @return  Newly created field.
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_projection(
	cmzn_fieldmodule_id field_module,
	cmzn_field_id source_field, cmzn_field_id projection_matrix_field);

/***************************************************************************//**
 * Creates a field returning the transpose of N*M matrix source_field.
 * The source_number_of_rows is specified; source_number_of_columns is computed
 * as source_field->number_of_components / <source_number_of_rows>;
 * this division must have no remainder.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_number_of_rows  Number of rows N in source_field.
 * @param source_field  N rows * M columns component matrix field.
 * @return Newly created M*N component transposed matrix field.
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_transpose(
	cmzn_fieldmodule_id field_module,
	int source_number_of_rows, cmzn_field_id source_field);

#ifdef __cplusplus
}
#endif

#endif
