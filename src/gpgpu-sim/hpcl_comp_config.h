/*
 * hpcl_comp_config.h
 *
 *  Created on: Jan 4, 2016
 *      Author: mumichang
 */

#ifndef HPCL_COMP_CONFIG_H_
#define HPCL_COMP_CONFIG_H_

#include "../option_parser.h"
#include <cassert>
#include <map>

struct hpcl_comp_config {

  hpcl_comp_config()
  {
    hpcl_comp_en = 0;
    hpcl_comp_algo = GLOBAL_PRIVATE;
    hpcl_comp_buffer_size = 1;
    hpcl_comp_word_size = 2;

    //added by kh(041216)
    hpcl_comp_noc_type = MESH;
    ///

    //added by kh(042516)
    hpcl_req_coal_en = 0;
    hpcl_req_coal_buffer_size = 1;
    hpcl_req_coal_lazy_inj_en = 0;
    ///

    //added by kh(042416)
    hpcl_rec_comp_en = 0;
    hpcl_rec_comp_algo = INTER_COMP_SAME_SM_PKT;
    hpcl_rec_comp_window = 1;
    hpcl_rec_comp_res = 2;
    hpcl_rec_comp_buffer_size = 1;
    hpcl_rec_comp_target = READ_REP_TO_SAME_SM;
    hpcl_inter_comp_schedule_policy = INTER_COMP_FIFO;
    hpcl_rec_comp_max_pkt_no = 2;
    ///

    //added by kh(060616)
    hpcl_trans_mode = NO_TRANS;
//    hpcl_dict_window_mode_2B = DICT_WINDOW_100;
//    hpcl_dict_window_mode_4B = DICT_WINDOW_100;
//    hpcl_dict_window_mode_8B = DICT_WINDOW_100;
    ///

    //added by kh(061216)
    hpcl_partial_comp_en = 0;
    ///

    //added by kh(070216)
    hpcl_dict_size_opt_en = 0;


    //added by kh(070516)
    hpcl_ext_lwm_data_type = ORG_DATA;

    //added by kh(070616)
    hpcl_dsc_lwm_opt_en = 0;
    hpcl_dsc_min_ds_size = 2;

    //added by kh(071316)
    hpcl_data_remap_function_list = NULL;
    hpcl_data_remap_en = 0;
    ///

    //added by kh(072216)
    hpcl_data_remap_level = 0;	//nibble-level data remapping

    //added by kh(072616)
    hpcl_float_approx_range = 0;

    //added by kh(073016)
    hpcl_max_sim_inst_no = 0;
    hpcl_dsc_max_ds_size = 128;
    hpcl_dsc_lwm_comp_cycle = 1;
    hpcl_dsc_lwm_decomp_cycle = 1;

    //added by kh(073116)
    hpcl_approx_en = 0;
    hpcl_cudasim_approx_en = 0;

    //added by kh(080316)
    hpcl_remap_data_use_only_en = 0;
    hpcl_data_remap_op_range = 0;


    //added by ab(080616)
    hpcl_approx_type =0;

    //added by kh(081516)
    hpcl_approx_memcpy_index_list = NULL;
    hpcl_float_det_type = HEURISTIC;
    ///

    //added by kh(082416)
    hpcl_cache_comp_en = 0;
    hpcl_comp_tag_size_multiplier = 2;
    hpcl_comp_cache_segment_size = 8;
    ///

    //added by kh(091416)
    hpcl_approx_err_compen_en = 0;
    ///

    //added by kh(092416)
    hpcl_l1d_cache_stat_mon_en = 0;
    ///

    //added by kh(092616)
    hpcl_adaptive_comp_en = 0;
    ///

    //added by kh(092616)
    hpcl_l1d_cache_stat_mon_max_inst = 0;
    hpcl_l1d_cache_miss_rate_thre = 0;
    ///

    //added by kh(110716)
    hpcl_adaptive_comp_threshold = 0.7;
    ///
		
	//added by kh(021617)
	hpcl_char_type_pct_th = 90;
	///

	//added by kh(021817)
	hpcl_char_preproc_en = 0;
	hpcl_char_symbol_conv_en = 0;
	hpcl_char_remap_extrabit_en = 0;
	///

	//added by kh(022217)
	hpcl_char_symbol_tbl = 0;
	///

	//added by kh(022317)
	hpcl_char_remap_2bit_en = 0;
	///

	//added by kh(030117)
	hpcl_char_remap_1bit_en = 0;
	hpcl_dyn_encoding_status_en = 0;
	///
	
	//added by kh(030217)
	hpcl_char_remap_1bit_swap_en = 0;
	///

  }

  void init()
  {
  }

  void reg_options(class OptionParser * opp)
  {
    option_parser_register(opp, "-hpcl_comp_en", OPT_UINT32, &hpcl_comp_en,
			     "0: disable compression, 1: enable compression",
			     "0");

    option_parser_register(opp, "-hpcl_comp_algo", OPT_UINT32, &hpcl_comp_algo,
			   "0: global_table(private), 1: ",
			   "0");

    option_parser_register(opp, "-hpcl_comp_buffer_size", OPT_UINT32, &hpcl_comp_buffer_size,
  			   "1: default, ",
  			   "1");

    option_parser_register(opp, "-hpcl_comp_word_size", OPT_UINT32, &hpcl_comp_word_size,
  			   "2: default, ",
  			   "2");

    //added by kh(041216)
    option_parser_register(opp, "-hpcl_comp_noc_type", OPT_UINT32, &hpcl_comp_noc_type,
    			   "1: default, MESH",
    			   "1");

    option_parser_register(opp, "-hpcl_comp_ideal_comp_ratio", OPT_UINT32, &hpcl_comp_ideal_comp_ratio,
    			   "0: default, no compression",
    			   "0");
    ///

    //Recursive Compression
    option_parser_register(opp, "-hpcl_rec_comp_en", OPT_UINT32, &hpcl_rec_comp_en,
			     "0: disable rec compression, 1: enable rec compression",
			     "0");
    option_parser_register(opp, "-hpcl_rec_comp_algo", OPT_UINT32, &hpcl_rec_comp_algo,
    			   "0: default, INTER_COMP_SAME_SM_PKT",
    			   "0");
    option_parser_register(opp, "-hpcl_rec_comp_window", OPT_UINT32, &hpcl_rec_comp_window,
			   "1: default, rec compression Window",
			   "1");
    option_parser_register(opp, "-hpcl_rec_comp_res", OPT_UINT32, &hpcl_rec_comp_res,
			   "2: default recursive compression resolution",
			   "2");
    option_parser_register(opp, "-hpcl_rec_comp_buffer_size", OPT_UINT32, &hpcl_rec_comp_buffer_size,
  			   "1: default, ",
  			   "1");
    option_parser_register(opp, "-hpcl_rec_comp_target", OPT_UINT32, &hpcl_rec_comp_target,
  			   "0: READ_REP_TO_SAME_SM (default), 1: READ_REP_TO_ALL_SM",
  			   "0");
    option_parser_register(opp, "-hpcl_inter_comp_schedule_policy", OPT_UINT32, &hpcl_inter_comp_schedule_policy,
			       "0: default, INTER_COMP_FIFO",
			       "0");

    if(hpcl_comp_word_size == 2 && hpcl_rec_comp_res == 2) 	{}
    else if(hpcl_comp_word_size == 4 && hpcl_rec_comp_res == 4) {}
    else if(hpcl_comp_word_size == 8 && hpcl_rec_comp_res == 8) {}
    else if(hpcl_comp_word_size == 6 && (hpcl_rec_comp_res == 2 || hpcl_rec_comp_res == 4)) {}
    else if(hpcl_comp_word_size == 14 && (hpcl_rec_comp_res == 2 || hpcl_rec_comp_res == 4 || hpcl_rec_comp_res == 8)) {}
    else {
      printf("Error!: hpcl_comp_word_size %u, hpcl_rec_comp_res %u\n", hpcl_comp_word_size, hpcl_rec_comp_res);
      assert(0);
    }

    option_parser_register(opp, "-hpcl_rec_comp_max_pkt_no", OPT_UINT32, &hpcl_rec_comp_max_pkt_no,
      			   "Maximum recursively compressed pkts no",
      			   "2");
    ///

    //Request Coalescing
    option_parser_register(opp, "-hpcl_req_coal_en", OPT_UINT32, &hpcl_req_coal_en,
			 "0: disable req coalescing, 1: enable req coalescing",
			 "0");
    option_parser_register(opp, "-hpcl_req_coal_buffer_size", OPT_UINT32, &hpcl_req_coal_buffer_size,
  			   "1: default, ",
  			   "1");
    option_parser_register(opp, "-hpcl_req_coal_lazy_inj_en", OPT_UINT32, &hpcl_req_coal_lazy_inj_en,
    			 "0: disable lazy req injection, 1: enable lazy req injection",
    			 "0");
    ///


    //Data Remapping
    option_parser_register(opp, "-hpcl_trans_mode", OPT_UINT32, &hpcl_trans_mode,
			   "0: No Data Transformation",
			   "0");
    ///

    //LWM Optimization
    option_parser_register(opp, "-hpcl_partial_comp_en", OPT_UINT32, &hpcl_partial_comp_en,
			   "0: Partial Compression Disable",
			   "0");
    option_parser_register(opp, "-hpcl_dict_size_opt_en", OPT_UINT32, &hpcl_dict_size_opt_en,
    			   "0: Dict Encoding Size Opt Disable",
    			   "0");
    ///

    //added by kh(070516)
    option_parser_register(opp, "-hpcl_ext_lwm_data_type", OPT_UINT32, &hpcl_ext_lwm_data_type,
        			   "0: ORG_DATA, 1: REMAPPED_DATA_1, 2: REMAPPED_DATA_2, 3: OPT_DATA",
        			   "0");
    ///
    option_parser_register(opp, "-hpcl_dsc_lwm_opt_en", OPT_UINT32, &hpcl_dsc_lwm_opt_en,
            			   "0: dsc_lwm_opt disable, 1: enable",
            			   "0");

    option_parser_register(opp, "-hpcl_dsc_min_ds_size", OPT_UINT32, &hpcl_dsc_min_ds_size,
				   "dsc_min_ds_size",
				   "2");

    //added by kh(071216)
    option_parser_register(opp, "-hpcl_data_remap_function_list", OPT_CSTR, &hpcl_data_remap_function_list,
                              "Data Remapping Function Index List",
                              "1,2,3,4");

    option_parser_register(opp, "-hpcl_data_remap_en", OPT_UINT32, &hpcl_data_remap_en,
				   "hpcl_data_remap_en",
				   "0");

    //added by ABPD(061916)
    option_parser_register(opp, "-hpcl_sc2_mode", OPT_UINT32, &hpcl_sc2_mode,
 			     "0: sample, 1: encode",
 			     "0");

    option_parser_register(opp, "-hpcl_fph_mode", OPT_UINT32, &hpcl_fph_mode,
 			     "0: sample, 1: encode",
 			     "0");
    option_parser_register(opp, "-hpcl_fph_iter", OPT_INT64, &hpcl_fph_iter,
 			     "0: Number, 1: ",
 			     "0");
    option_parser_register(opp, "-hpcl_sc2_iter", OPT_INT64, &hpcl_sc2_iter,
			 "0: Number, 1: ",
			 "0");
 option_parser_register(opp, "-hpcl_fph_iter_sam", OPT_INT64, &hpcl_fph_iter_sam,
 			     "0: Number, 1: ",
 			     "0");
	option_parser_register(opp, "-hpcl_fph_iter_enc", OPT_INT64, &hpcl_fph_iter_enc,
 			     "0: Number, 1: ",
 			     "0");

    option_parser_register(opp, "-hpcl_sc2_iter_sam", OPT_INT64, &hpcl_sc2_iter_sam,
			 "0: Number, 1: ",
			 "0");
	option_parser_register(opp, "-hpcl_sc2_iter_enc", OPT_INT64, &hpcl_sc2_iter_enc,
			 "0: Number, 1: ",
			 "0");

    //added by kh(072016)
    option_parser_register(opp, "-hpcl_data_remap_op_en", OPT_UINT32, &hpcl_data_remap_op_en,
     			     "0: remap operator disable, 1: enable",
     			     "0");
    option_parser_register(opp, "-hpcl_dsc_lwm_rep_opt_en", OPT_UINT32, &hpcl_dsc_lwm_rep_opt_en,
     			     "0: representation opt disable, 1: enable",
     			     "0");
    ///

    option_parser_register(opp, "-hpcl_data_remap_level", OPT_UINT32, &hpcl_data_remap_level,
     			     "0: nibble-level remapping, 1: bit-level remapping",
     			     "0");

    //added by kh(072716)
    option_parser_register(opp, "-hpcl_float_approx_range", OPT_UINT32, &hpcl_float_approx_range,
     			     "0: no float number approx, X: number of nibbles for approx",
     			     "0");
    ///

    //added by kh(073016)
    option_parser_register(opp, "-hpcl_max_sim_inst_no", OPT_UINT64, &hpcl_max_sim_inst_no,
         			     "the maximum # of instructions",
         			     "0");
    option_parser_register(opp, "-hpcl_dsc_max_ds_size", OPT_UINT32, &hpcl_dsc_max_ds_size,
         			     "maximum segment size",
         			     "128");

    option_parser_register(opp, "-hpcl_dsc_lwm_comp_cycle", OPT_UINT32, &hpcl_dsc_lwm_comp_cycle,
             			     "maximum cycle number",
             			     "1");
    option_parser_register(opp, "-hpcl_dsc_lwm_decomp_cycle", OPT_UINT32, &hpcl_dsc_lwm_decomp_cycle,
                 			     "maximum cycle number",
                 			     "1");
    ///

    option_parser_register(opp, "-hpcl_approx_en", OPT_UINT32, &hpcl_approx_en,
                 			     "0: Approximation disable, 1: Approximation enable",
                 			     "0");
    option_parser_register(opp, "-hpcl_cudasim_approx_en", OPT_UINT32, &hpcl_cudasim_approx_en,
                     			     "0: CUDASIM Approximation disable, 1: CUDASIM Approximation enable",
                     			     "0");



    //added by kh(080316)
    option_parser_register(opp, "-hpcl_remap_data_use_only_en", OPT_UINT32, &hpcl_remap_data_use_only_en,
                 			     "Disable(0)/Enable(1) remapped data use only ",
                 			     "0");

    option_parser_register(opp, "-hpcl_data_remap_op_range", OPT_UINT32, &hpcl_data_remap_op_range,
                     			     "remap operation range",
                     			     "0");

    //added by ab(080616)
    option_parser_register(opp, "-hpcl_approx_type", OPT_UINT32, &hpcl_approx_type,
    			       "0: default, SET_ZERO",
    			       "0");
    ///


    //added by kh(081516)
    option_parser_register(opp, "-hpcl_approx_memcpy_index_list", OPT_CSTR, &hpcl_approx_memcpy_index_list,
                              "Approximable memcpy function index",
                              "");

    option_parser_register(opp, "-hpcl_float_det_type", OPT_UINT32, &hpcl_float_det_type,
        			       "0: HEURISTIC, 1: MEMSPACE",
        			       "0");
    ///


    //added by kh(082216)
    option_parser_register(opp, "-hpcl_cache_comp_en", OPT_UINT32, &hpcl_cache_comp_en,
				       "L1 cache compression",
				       "0");
    option_parser_register(opp, "-hpcl_comp_tag_size_multiplier", OPT_UINT32, &hpcl_comp_tag_size_multiplier,
            			       "TAG_WAY_NO/DATA_WAY_NO",
            			       "1");
    option_parser_register(opp, "-hpcl_comp_cache_segment_size", OPT_UINT32, &hpcl_comp_cache_segment_size,
				       "Segment Size in Data Array",
				       "8");
    ///


    //added by kh(091416)
    option_parser_register(opp, "-hpcl_approx_err_compen_en", OPT_UINT32, &hpcl_approx_err_compen_en,
				       "Approximation Error Compensation",
				       "0");
    ///


    //added by kh(092416)
    option_parser_register(opp, "-hpcl_l1d_cache_stat_mon_en", OPT_UINT32, &hpcl_l1d_cache_stat_mon_en,
				   "L1D cache stat monitor enable/disable",
				   "0");
    ///

    //added by kh(092416)
    option_parser_register(opp, "-hpcl_adaptive_comp_en", OPT_UINT32, &hpcl_adaptive_comp_en,
			       "adaptive L1D cache comp enable/disable",
			       "0");
    ///

    //added by kh(092616)
    option_parser_register(opp, "-hpcl_l1d_cache_stat_mon_max_inst", OPT_UINT32, &hpcl_l1d_cache_stat_mon_max_inst,
    			       "maximum # of instructions that turns off L1D cache stat monitoring",
    			       "0");
    option_parser_register(opp, "-hpcl_l1d_cache_miss_rate_thre", OPT_UINT32, &hpcl_l1d_cache_miss_rate_thre,
			       "maximum cache miss rate that turns off L1D cache compression",
			       "0");
    ///


    //added by kh(111716)
    option_parser_register(opp, "-hpcl_adaptive_comp_threshold", OPT_FLOAT, &hpcl_adaptive_comp_threshold,
  			   "0.7: default, ",
  			   "0.7");
    ///


	//added by kh(021617)
    option_parser_register(opp, "-hpcl_char_type_pct_th", OPT_UINT32, &hpcl_char_type_pct_th,
  			   "90: default, ",
  			   "90");
    ///

    //added by kh(021817)
	option_parser_register(opp, "-hpcl_char_preproc_en", OPT_UINT32, &hpcl_char_preproc_en,
				   "0: disable, ",
				   "0");
	option_parser_register(opp, "-hpcl_char_symbol_conv_en", OPT_UINT32, &hpcl_char_symbol_conv_en,
			   "0: disable, ",
			   "0");
	option_parser_register(opp, "-hpcl_char_remap_extrabit_en", OPT_UINT32, &hpcl_char_remap_extrabit_en,
				   "0: disable, ",
				   "0");
	///

	//added by kh(022217)
	option_parser_register(opp, "-hpcl_char_symbol_tbl", OPT_UINT32, &hpcl_char_symbol_tbl,
					   "0: disable, ",
					   "0");
	///

	//added by kh(022317)
	option_parser_register(opp, "-hpcl_char_remap_2bit_en", OPT_UINT32, &hpcl_char_remap_2bit_en,
					   "0: disable, ",
					   "0");
	///


	//added by kh(030117)
	option_parser_register(opp, "-hpcl_char_remap_1bit_en", OPT_UINT32, &hpcl_char_remap_1bit_en,
					   "0: disable, ",
					   "0");
	option_parser_register(opp, "-hpcl_dyn_encoding_status_en", OPT_UINT32, &hpcl_dyn_encoding_status_en,
						   "0: disable, ",
						   "0");
	///

	//added by kh(030217)
	option_parser_register(opp, "-hpcl_char_remap_1bit_swap_en", OPT_UINT32, &hpcl_char_remap_1bit_swap_en,
							   "0: disable, ",
							   "0");
	///

  }


  enum comp_algo_type {
    GLOBAL_PRIVATE = 0,
    LOCAL_WORD_MATCHING,
    //added by abpd (042816)
    BDI_WORD_MATCHING,
    ABPD_LOCAL_WORD_MATCHING,
    CPACK_WORD_MATCHING,
    FPC_WORD_MATCHING,
    //added by khkim (051816)
    LOCAL_WORD_MATCHING2,
    //added by abpd (061916)
    SC2_WORD_MATCHING,
    FPH_WORD_MATCHING,
    BPC_WORD_MATCHING,
    //added by khkim (070416)
    DATA_SEG_MATCHING,
    DATA_SEG_LWM_MATCHING,		//Overlay the compressed segments
    DATA_SEG_LWM_HYBRID_SEQ,		//Select the best one among DSM and LWM
    DATA_SEG_LWM_HYBRID_PARL,		//Select the best one among DSM and LWM

	//added by kh(030117)
	DATA_SEG_MATCHING2,			//two patterns

    END_OF_COMP_ALGO
  };


  unsigned hpcl_comp_en;
  enum comp_algo_type hpcl_comp_algo;

  //added by kh(031916)
  unsigned hpcl_comp_buffer_size;
  unsigned hpcl_comp_word_size;


  //added by kh(041216)
  enum noc_type {
    MESH = 0,
    GLOBAL_CROSSBAR,
  };
  enum noc_type hpcl_comp_noc_type;

  unsigned hpcl_comp_ideal_comp_ratio;		//ideal compression ratio: 0, 20, 40, 60
  ///

  //added by kh(042416)
  unsigned hpcl_req_coal_en;
  unsigned hpcl_req_coal_buffer_size;
  unsigned hpcl_req_coal_lazy_inj_en;
  //


  // Recursive Compression Configuration
  unsigned hpcl_rec_comp_en;
  unsigned hpcl_rec_comp_window;
  unsigned hpcl_rec_comp_res;	//2: 6: 2+4, 14: 2+4+8
  unsigned hpcl_rec_comp_buffer_size;
  unsigned hpcl_rec_comp_max_pkt_no;

  //added by kh(051816)
  enum inter_comp_algo_type {
    //BASIC_APPEND = 0,
    //INTER_COMP,
    //INTER_COMP_FB,		//compressed buffer resolution feedbacks the compression resolution for new compression
    //REC_COMP_ALGO1,
    INTER_COMP_SAME_SM_PKT = 0,	//compressing packets to the same sm only (algo1)
    INTER_COMP_DIFF_SM_PKT,	//merging same contents to diff sm only (algo2)
    INTER_COMP_SAME_DIFF_SM_PKT,	//mixture of algo1 and algo2, but algo1 has priority.
    INTER_COMP_DIFF_SAME_SM_PKT,	//mixture of algo1 and algo2, but algo2 has priority.

  };
  enum inter_comp_algo_type hpcl_rec_comp_algo;
  ///

  enum rec_comp_target {
    READ_REP_TO_SAME_SM = 0,
    READ_REP_TO_ALL_SM,
  };
  enum rec_comp_target hpcl_rec_comp_target;

  //added by kh(052016)
  enum inter_comp_scheduling_type {
    INTER_COMP_FIFO = 0,
    INTER_COMP_MAX_REP_FIRST,
    INTER_COMP_MIN_REP_FIRST,
  };
  enum inter_comp_scheduling_type hpcl_inter_comp_schedule_policy;
  ///

  bool is_rec_comp_avail(unsigned comp_res) {
    bool ret = false;
    if(comp_res == 2) {
      if(hpcl_rec_comp_res == 2 || hpcl_rec_comp_res == 6 || hpcl_rec_comp_res == 14) ret = true;
      else ret = false;
    } else if(comp_res == 4) {
      if(hpcl_rec_comp_res == 4 || hpcl_rec_comp_res == 6 || hpcl_rec_comp_res == 14) ret = true;
      else ret = false;
    } else if(comp_res == 8) {
      if(hpcl_rec_comp_res == 8 || hpcl_rec_comp_res == 14) ret = true;
      else ret = false;
    } else assert(0);

    return ret;
  }

  //added by kh(060616)
  enum trans_mode {
    NO_TRANS = 0,
    POST_TRANS_UPTO_COMP_NO_BENEFIT,
    POST_TRANS_UPTO_COMP_2B,
    POST_TRANS_UPTO_COMP_4B,
    POST_TRANS_UPTO_COMP_8B,
    PRE_TRANS,
    PRE_TRANS_OPT,
  };
  enum trans_mode hpcl_trans_mode;//0: apply transform only to the no comp benefit case, 1: additionally to 2B lose case
  ///

  //added by kh(061216)
  unsigned hpcl_partial_comp_en;

  //added by kh(070216)
  unsigned hpcl_dict_size_opt_en;

  //added by kh(070516)
  enum ext_lwm_data_type {
    ORG_DATA = 0,
    REMAPPED_DATA_1,
    REMAPPED_DATA_2,
    REMAPPED_DATA_3,
    REMAPPED_DATA_4,
    REMAPPED_DATA_5,
    REMAPPED_DATA_6,
    OPT_DATA,
  };
  enum ext_lwm_data_type hpcl_ext_lwm_data_type;	//When DSC cannot compress


  unsigned hpcl_dsc_lwm_opt_en;	//When DSC can compress, try to find the optimal data type based on the
				//compressed data size after LWM.

  //an ideal comp is simulated when hpcl_ext_lwm_data_type is set to OPT_DATA, hpcl_dsc_lwm_opt_en is set to 1.
  unsigned hpcl_dsc_min_ds_size;

  //added by kh(071316)
  unsigned hpcl_data_remap_en;
  char* hpcl_data_remap_function_list;
  std::vector<unsigned> hpcl_data_remap_function;

  //added by kh(072016)
  unsigned hpcl_data_remap_op_en;
  unsigned hpcl_dsc_lwm_rep_opt_en;


  //added by kh(072216)
  unsigned hpcl_data_remap_level;	//0: nibble-level remapping, 1: bit-level remapping


  //added by kh(072616)
  unsigned hpcl_float_approx_range;	//0: no approx, 1: 1 nibble, 2 : 2 nibble


  //added by kh(073016)
  unsigned long long hpcl_max_sim_inst_no;	//the maximum # of instructions to be simulated
  unsigned hpcl_dsc_max_ds_size;		//maximum segment size


  unsigned hpcl_dsc_lwm_comp_cycle;
  unsigned hpcl_dsc_lwm_decomp_cycle;


  //added by kh(073116)
  unsigned hpcl_approx_en;
  unsigned hpcl_cudasim_approx_en;


  //added by kh(080316)
  unsigned hpcl_remap_data_use_only_en;

  //added by kh(080316)
  unsigned hpcl_data_remap_op_range;	//0: all, n: first n groups


  //added by kh(081516)
  //std::vector<unsigned> hpcl_approx_memcpy_index;
  std::map<unsigned, unsigned> hpcl_approx_memcpy_index;
  char* hpcl_approx_memcpy_index_list;
  enum float_det_type {
    HEURISTIC = 0,
    MEMSPACE,
  };
  enum float_det_type hpcl_float_det_type;
  ///

  //added by kh(082416)
  unsigned hpcl_cache_comp_en;
  unsigned hpcl_comp_tag_size_multiplier;
  unsigned hpcl_comp_cache_segment_size;
  ///


  //added by ABPD(061916)
  unsigned hpcl_sc2_mode;
  unsigned hpcl_fph_mode;
  long long hpcl_fph_iter;
  long long hpcl_sc2_iter;
 long long hpcl_fph_iter_sam;
  long long hpcl_fph_iter_enc;
  long long hpcl_sc2_iter_sam;
  long long hpcl_sc2_iter_enc;

  // added by ab(080616)
  enum app_type {
    SET_ZERO=0,
    SET_AVG,
    SET_FREQ
  };
  unsigned hpcl_approx_type;

  //added by kh(091416)
  unsigned hpcl_approx_err_compen_en;	//error compensation mechanism


  //added by kh(092416)
  unsigned hpcl_l1d_cache_stat_mon_en;

  unsigned hpcl_adaptive_comp_en;		//adaptive mechanism enabling cache comp
  unsigned hpcl_l1d_cache_stat_mon_max_inst;	//maximum # of instructions that L1D cache stat needs to be monitored
  unsigned hpcl_l1d_cache_miss_rate_thre;	//maximum cache miss rate that turns off L1D cache compression


  //added by kh(110716)
  //unsigned hpcl_decomp_buffer_size;
  float hpcl_adaptive_comp_threshold;
  ///

  //added by kh(021617)
  unsigned hpcl_char_type_pct_th;			//character percentage
  ///

  //added by kh(021817)
  unsigned hpcl_char_preproc_en;			//preprocessing char data
  unsigned hpcl_char_symbol_conv_en;		//character symbol conversion
  unsigned hpcl_char_remap_extrabit_en;		//remapping data with extra 1bit
  ///

  //added by kh(022217)
  unsigned hpcl_char_symbol_tbl;			//preprocessing char data
  ///

  //added by kh(022317)
  unsigned hpcl_char_remap_2bit_en;			//remap based on 2bit.
  ///

  //added by kh(030117)
  unsigned hpcl_char_remap_1bit_en;			//remap based on 1bit.
  unsigned hpcl_dyn_encoding_status_en;		//dynamic encoding status enable. rather than showing encoding status for all segments
  ///

  //added by kh(030217)
  unsigned hpcl_char_remap_1bit_swap_en;	//swap 3rd and 2nd bit position, e.g. 0101 --> 0010


};




#endif /* HPCL_DIRECT_LINK_CONFIG_H_ */
