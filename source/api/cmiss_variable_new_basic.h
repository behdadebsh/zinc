/*******************************************************************************
FILE : api/cmiss_variable_new_basic.h

LAST MODIFIED : 8 September 2003

DESCRIPTION :
The public interface to the Cmiss_variable_new basic objects.
==============================================================================*/
#ifndef __API_CMISS_VARIABLE_NEW_BASIC_H__
#define __API_CMISS_VARIABLE_NEW_BASIC_H__

#include "api/cmiss_variable_new.h"

typedef float Scalar;

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_variable_new_id Cmiss_variable_new_scalar_create(char *name,Scalar value);
/*******************************************************************************
LAST MODIFIED : 8 September 2003

DESCRIPTION :
Creates a Cmiss_variable_new scalar with the supplied <name> and <value>.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_VARIABLE_NEW_BASIC_H__ */
