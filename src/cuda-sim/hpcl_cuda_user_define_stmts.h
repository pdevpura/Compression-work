/*
 * user-defined.h
 *
 *  Created on: Nov 19, 2014
 *      Author: mumichang
 */

#ifndef CUDA_SIM_USER_DEFINE_STMTS_H_
#define CUDA_SIM_USER_DEFINE_STMTS_H_

//#define DATA_REMAP_DEBUG	1
#ifdef DATA_REMAP_DEBUG
#define DATA_REMAP_DEBUG_PRINT(fmt, args...)    printf(fmt, ## args)
#else
#define DATA_REMAP_DEBUG_PRINT(fmt, args...)    /* Don't do anything in release builds */
#endif

//#define LWM_COMP_DEBUG	1
#ifdef LWM_COMP_DEBUG
#define LWM_COMP_DEBUG_PRINT(fmt, args...)    printf(fmt, ## args)
#else
#define LWM_COMP_DEBUG_PRINT(fmt, args...)    /* Don't do anything in release builds */
#endif


//#define DATA_SEG_COMP_DEBUG	1
#ifdef DATA_SEG_COMP_DEBUG
#define DATA_SEG_COMP_DEBUG_PRINT(fmt, args...)    printf(fmt, ## args)
#else
#define DATA_SEG_COMP_DEBUG_PRINT(fmt, args...)    /* Don't do anything in release builds */
#endif

//#define DATA_SEG_LWM_COMP_DEBUG	1
#ifdef DATA_SEG_LWM_COMP_DEBUG
#define DATA_SEG_LWM_COMP_DEBUG_PRINT(fmt, args...)    printf(fmt, ## args)
#else
#define DATA_SEG_LWM_COMP_DEBUG_PRINT(fmt, args...)    /* Don't do anything in release builds */
#endif


//#define ERROR_COMPEN_DEBUG 1


#endif /* GPGPU_SIM_USER_DEFINED_H_ */
