/*
 * errorLUT.c
 *
 *  Created on: Mar 7, 2024
 *      Author: byo10
 */

#include "errorLUT.h"
#include "errors.h"

const error_struct _default_shutdown_ = {NULL,(ERR_SHUTDOWN_MASK),NULL,NULL};
const error_struct _default_suspend_ = {NULL,(ERR_SUSPEND_MASK),NULL,NULL};
const error_struct _default_limit_ = {NULL,(ERR_LIMP_MASK),NULL,NULL};

const error_struct* _error_LUT[]={
&_default_shutdown_,
&_default_suspend_,
&_default_limit_,
&_default_suspend_,
&_default_suspend_,
&_default_suspend_,
&_default_suspend_,
&_default_suspend_,
&_default_suspend_,
&_default_suspend_,
&_default_suspend_,
&_default_suspend_,
&_default_suspend_,
&_default_suspend_,
&_default_suspend_,
&_default_suspend_,
&_default_suspend_,
&_default_suspend_,
&_default_suspend_,
&_default_suspend_,
&_default_suspend_,
&_default_suspend_,
&_default_suspend_,
&_default_suspend_,
&_default_suspend_,
&_default_suspend_,
&_default_suspend_,
&_default_suspend_,
&_default_suspend_,
&_default_suspend_,
&_default_suspend_,
};








