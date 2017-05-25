/*
 * user-defined.h
 *
 *  Created on: Nov 19, 2014
 *      Author: mumichang
 */

#ifndef GPGPU_SIM_USER_DEFINE_STMTS_H_
#define GPGPU_SIM_USER_DEFINE_STMTS_H_


//added by kh(041516)
//#define WORD1B_ANALYSIS	1


//added by kh(060616)
//#define DATA_TRANSFORMATION_TEST 1
///

//#define DATA_TRANSFORMATION_WORSE_CASE_ANALYSIS 1




//#define COMP_WORD_ANALYSIS 1
//#define REQ_COAL_MODULE 1		//mem_fetch.h, gpu-cache.c, shader.cc



//#define GLOB_COMP_MODULE 1		//Inter-Sim need to be copied from old file


#define REC_COMP_MODULE 1



//#define MULTICAST_DEBUG	1

#ifdef MULTICAST_DEBUG
#define MCT_DEBUG_PRINT(fmt, args...)    printf(fmt, ## args)
#else
#define MCT_DEBUG_PRINT(fmt, args...)    /* Don't do anything in release builds */
#endif


#define COMP_DEBUG	1

#ifdef COMP_DEBUG
#define COMP_DEBUG_PRINT(fmt, args...)    printf(fmt, ## args)
#else
#define COMP_DEBUG_PRINT(fmt, args...)    /* Don't do anything in release builds */
#endif



//#define REC_COMP_DEBUG	1

#ifdef REC_COMP_DEBUG
#define REC_COMP_DEBUG_PRINT(fmt, args...)    printf(fmt, ## args)
#else
#define REC_COMP_DEBUG_PRINT(fmt, args...)    /* Don't do anything in release builds */
#endif


//#define ICNT_DEBUG	1

#ifdef ICNT_DEBUG
#define ICNT_DEBUG_PRINT(fmt, args...)    printf(fmt, ## args)
#else
#define ICNT_DEBUG_PRINT(fmt, args...)    /* Don't do anything in release builds */
#endif

//#define REC_ANAL_COMP_DEBUG	1

#ifdef REC_ANAL_COMP_DEBUG
#define REC_ANAL_COMP_DEBUG_PRINT(fmt, args...)    printf(fmt, ## args)
#else
#define REC_ANAL_COMP_DEBUG_PRINT(fmt, args...)    /* Don't do anything in release builds */
#endif


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

//added by kh(072016)
#define DATA_SEG_COMP_STATIC_RES  1
#define DATA_REMAP_OP 1


//#define CACHE_COMP_DEBUG	1
#ifdef CACHE_COMP_DEBUG
#define CACHE_COMP_DEBUG_PRINT(fmt, args...)    printf(fmt, ## args)
#else
#define CACHE_COMP_DEBUG_PRINT(fmt, args...)    /* Don't do anything in release builds */
#endif



//#define DATA_TYEP_CHECK_DEBUG	1
#ifdef DATA_TYEP_CHECK_DEBUG
#define DATA_TYEP_CHECK_DEBUG_PRINT(fmt, args...)    printf(fmt, ## args)
#else
#define DATA_TYEP_CHECK_DEBUG_PRINT(fmt, args...)    /* Don't do anything in release builds */
#endif



#endif /* GPGPU_SIM_USER_DEFINED_H_ */
