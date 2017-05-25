/*
 * hpcl_comp_anal.h
 *
 *  Created on: Mar 8, 2016
 *      Author: mumichang
 */

#ifndef GPGPU_SIM_HPCL_COMP_ANAL_H_
#define GPGPU_SIM_HPCL_COMP_ANAL_H_

#include "hpcl_stat.h"
#include <iostream>
#include <vector>

//added by kh(061516)
#include "hpcl_user_define_stmts.h"
///

class hpcl_comp_anal
{
public:
  hpcl_comp_anal ();
  virtual
  ~hpcl_comp_anal ();

//added by kh(030816)
public:
  enum sample_type {
    GLOBAL_TABLE_HIT_NO = 0,
    GLOBAL_TABLE_MISS_NO,
    PACKET_COMP_RATIO,		//Flit-level Compression Ratio
    PACKET_COMP_8B_RATIO,		//Flit-level Compression Ratio only for Compressed Data with 8B resolution
    PACKET_COMP_4B_RATIO,		//Flit-level Compression Ratio only for Compressed Data with 4B resolution
    PACKET_COMP_2B_RATIO,		//Flit-level Compression Ratio only for Compressed Data with 2B resolution
    FRAGMENT_RATE_PER_FLIT,		//fragmentation rate in the last flit
    FRAGMENT_RATE_PER_PACKET,		//fragmentation rate over the packet
    COMP_OVERHEAD_NO,
    ORG_FLIT_NO,
    COMP_FLIT_NO,
    COMP_0B_NO,	//added by kh(060216)
    COMP_2B_NO,
    COMP_4B_NO,
    COMP_8B_NO,
    //added by kh(042116)
    WORD2B_ZEROS_REP_RATE,
    WORD2B_NONZEROS_REP_RATE,
    WORD2B_UNI_VAL_RATE,
    WORD4B_ZEROS_REP_RATE,
    WORD4B_NONZEROS_REP_RATE,
    WORD4B_UNI_VAL_RATE,
    WORD8B_ZEROS_REP_RATE,
    WORD8B_NONZEROS_REP_RATE,
    WORD8B_UNI_VAL_RATE,
    SINGLE_REP_TO_SAME_SM_RATE,
    MULTI_REP_TO_SAME_SM_RATE,
    NO_REP_TO_SAME_SM_RATE,
    MULTI_REP_TO_SAME_SM_NO,
    //added by kh(042216)
    WORD1B_ZEROS_REP_RATE_IN_UNCOMP_DATA,
    WORD1B_NONZEROS_REP_RATE_IN_UNCOMP_DATA,
    WORD1B_UNI_VAL_RATE_IN_UNCOMP_DATA,
    WORD1B_NONZEROS_REP_NO_IN_UNCOMP_DATA,
    //added by kh(042316)
    WORD1B_REDUND_RATE_INTER_READ_REPLIES,
    //added by kh(051916)
    INTER_COMP_PACKET_NO,
    INTRA_COMP_PACKET_NO,
    INTER_COMP_MATCHING_WORD_NO,
    //added by kh(060216)
    COMP_DATA_RATIO,		//Data-level Compression Ratio
//    TRANS_COMP_2B_WIN_NO,
//    TRANS_COMP_4B_WIN_NO,
//    TRANS_COMP_8B_WIN_NO,
    //added by kh(061016)
    MAX_WORD_INDEX_2B,
    MAX_WORD_INDEX_4B,
    MAX_WORD_INDEX_8B,
    MAX_WORD_INDEX_TRANS_2B,
    MAX_WORD_INDEX_TRANS_4B,
    MAX_WORD_INDEX_TRANS_8B,
    COMP_DATA_RATIO_SEGMENT_2B,
    COMP_DATA_RATIO_SEGMENT_4B,
    COMP_DATA_RATIO_SEGMENT_8B,
    NO_COMP_DATA_NUM_SEGMENT_2B,
    NO_COMP_DATA_NUM_SEGMENT_4B,
    NO_COMP_DATA_NUM_SEGMENT_8B,
    //added by kh(062516)
    REC_COMP_INTRA_SM_READ_REP_GROUP_SIZE,
    REC_COMP_INTER_SM_READ_REP_GROUP_SIZE,
    //added by kh(062816)
    //stat about waiting mfs for recursive compression
    REC_COMP_LAST_MF_NOT_FOUND,
    REC_COMP_LAST_MF_COMP_RES_MISMATCH,
    REC_COMP_LAST_MF_FULL,		//there is a last mf but a new mf cannot be merged since a last mf merges mfs up to maximum mfs
    //added by kh(070416)
    DSC_COMP_0B_NO,
    DSC_COMP_2B_NO,
    DSC_COMP_4B_NO,
    DSC_COMP_8B_NO,
    DSC_COMP_16B_NO,
    DSC_COMP_32B_NO,
    DSC_COMP_64B_NO,
    DSC_COMP_128B_NO,
    DATA_TYPE0_NO,	//original data
    DATA_TYPE1_NO,	//remapped data type 1
    DATA_TYPE2_NO,	//remapped data type 2
    DATA_TYPE3_NO,	//remapped data type 3
    DATA_TYPE4_NO,	//remapped data type 4
    DATA_TYPE5_NO,	//remapped data type 3
    DATA_TYPE6_NO,	//remapped data type 4

	//added by kh(021817)
	DATA_TYPE7_NO,	//remapped data type 7	//char data type
	///

    EXT_LWM_DATA_TYPE0_NO,	//original data, DSC cannot compress, then LWM compress
    EXT_LWM_DATA_TYPE1_NO,	//remapped data type 1, DSC cannot compress, then LWM compress
    EXT_LWM_DATA_TYPE2_NO,	//remapped data type 2, DSC cannot compress, then LWM compress
    EXT_LWM_DATA_TYPE3_NO,	//remapped data type 3, DSC cannot compress, then LWM compress
    EXT_LWM_DATA_TYPE4_NO,	//remapped data type 4, DSC cannot compress, then LWM compress
    EXT_LWM_DATA_TYPE5_NO,	//remapped data type 5, DSC cannot compress, then LWM compress
    EXT_LWM_DATA_TYPE6_NO,	//remapped data type 6, DSC cannot compress, then LWM compress
    //added by kh(070716)
    READ_REQ_NO,
    //added by kh(071216)
    DSM_COMP_NO,	//frequencies that DSM is selected
    LWM_COMP_NO,	//frequencies that LWM is selected
    NO_COMP_NO,		//neither comp is selected
    DSM_COMP_DATA_RATIO,
    LWM_COMP_DATA_RATIO,
    //added by kh(071516)
    REC_COMP_INTRA_SM_SAME_DATA_MERGE_NO,

    //added by kh(072316)
    DSM_COMP_REMAP_OP_CASE_NO,
    DSM_COMP_REMAP_MASK_OP_NO,
    DSM_COMP_REMAP_COMP_OP_NO,
    DSM_COMP_REMAP_NEIGHBOR_CONST_DELTA_OP_NO,
    DSM_COMP_REMAP_DELTA_UPTO_1_OP_NO,
    DSM_COMP_REMAP_DELTA_UPTO_3_OP_NO,
    DSM_COMP_REMAP_COMP_DELTA_UPTO_1_OP_NO,
    DSM_COMP_REMAP_COMP_DELTA_UPTO_3_OP_NO,

    //added by kh(072616)
    COMP_PACKET_NO,
    //added by kh(072816)
    APPROX_DATA_NO,
    APPROX_EXCEPTION_NO,

	//added by kh(030117)
	CACHE_COMP_RATIO,		//Segment Size : 8byte
	//added by kh(030217)
	//CHAR_DATA_DIST,
  };

  void create(unsigned sub_partition_no);
  void add_sample(enum sample_type type, double val, int id=-1);
  hpcl_stat* m_comp_table_hit_no;
  hpcl_stat* m_comp_table_miss_no;
  hpcl_stat* m_ovr_avg_comp_table_hit_rate;

  void display(std::ostream & os = std::cout) const;
  void update_overall_stat();
  void clear();
  ///



//added by kh(032016)
private:
//  hpcl_stat* m_read_reply_comp_ratio;		//(flit no for original_packet - flit no for compressed packet)/flit no for original_packet*100
//  hpcl_stat* m_ovr_avg_read_reply_comp_ratio;

//added by kh(041616)
private:
/*
  hpcl_stat* m_fragment_rate_per_flit;			//unused space/flit size*100
  hpcl_stat* m_fragment_rate_per_packet;			//unused space/packet size*100
  hpcl_stat* m_ovr_avg_fragment_rate_per_flit;
  hpcl_stat* m_ovr_avg_fragment_rate_per_packet;
  hpcl_stat* m_comp_overhead_no;		//The # of the case where compressed Data > original Data
  hpcl_stat* m_ovr_avg_comp_overhead_rate;
*/
///


//added by kh(041816)
private:
  hpcl_stat* m_org_flit_no;
  hpcl_stat* m_ovr_avg_org_flit_no;
  hpcl_stat* m_comp_flit_no;
  hpcl_stat* m_ovr_avg_comp_flit_no;

  hpcl_stat* m_comp_0B_no;	//data tried to be compressed, but no data is reduced at all.
  hpcl_stat* m_comp_2B_no;
  hpcl_stat* m_comp_4B_no;
  hpcl_stat* m_comp_8B_no;
///

//added by kh(042116)
private:
  hpcl_stat* m_word2B_zeros_rep_rate;
  hpcl_stat* m_word2B_nonzeros_rep_rate;
  hpcl_stat* m_word2B_uni_val_rate;
  hpcl_stat* m_ovr_avg_word2B_zeros_rep_rate;
  hpcl_stat* m_ovr_avg_word2B_nonzeros_rep_rate;
  hpcl_stat* m_ovr_avg_word2B_uni_val_rate;

  hpcl_stat* m_word4B_zeros_rep_rate;
  hpcl_stat* m_word4B_nonzeros_rep_rate;
  hpcl_stat* m_word4B_uni_val_rate;
  hpcl_stat* m_ovr_avg_word4B_zeros_rep_rate;
  hpcl_stat* m_ovr_avg_word4B_nonzeros_rep_rate;
  hpcl_stat* m_ovr_avg_word4B_uni_val_rate;

  hpcl_stat* m_word8B_zeros_rep_rate;
  hpcl_stat* m_word8B_nonzeros_rep_rate;
  hpcl_stat* m_word8B_uni_val_rate;
  hpcl_stat* m_ovr_avg_word8B_zeros_rep_rate;
  hpcl_stat* m_ovr_avg_word8B_nonzeros_rep_rate;
  hpcl_stat* m_ovr_avg_word8B_uni_val_rate;

/*
  //distribution of replies in the compression buffer
  std::vector<hpcl_stat*> m_single_rep_to_same_sm_rate;
  std::vector<hpcl_stat*> m_multi_rep_to_same_sm_rate;
  std::vector<hpcl_stat*> m_no_rep_to_same_sm_rate;
  hpcl_stat* m_ovr_avg_single_rep_to_same_sm_rate;
  hpcl_stat* m_ovr_avg_multi_rep_to_same_sm_rate;
  hpcl_stat* m_ovr_avg_no_rep_to_same_sm_rate;
  hpcl_stat* m_multi_rep_to_same_sm_no;
  hpcl_stat* m_ovr_avg_multi_rep_to_same_sm_no;
///
*/

//added by kh(051916)
private:
  /*
  hpcl_stat* m_inter_comp_packet_no;
  hpcl_stat* m_intra_comp_packet_no;
  hpcl_stat* m_ovr_avg_inter_comp_packet_rate;
  hpcl_stat* m_inter_comp_matching_word_no;
  hpcl_stat* m_ovr_avg_inter_comp_matching_word_no;
  */
///

//added by kh(052016)
private:
  hpcl_stat* m_read_reply_comp_ratio_across_kernels;
  hpcl_stat* m_inter_comp_packet_no_across_kernels;
  hpcl_stat* m_intra_comp_packet_no_across_kernels;
  hpcl_stat* m_inter_comp_matching_word_no_across_kernels;
  hpcl_stat* m_fragment_rate_per_flit_across_kernels;			//unused space/flit size*100
  hpcl_stat* m_fragment_rate_per_packet_across_kernels;			//unused space/packet size*100
///

//added by kh(030117)
  hpcl_stat* m_read_reply_cache_comp_ratio_across_kernels;



//added by kh(060216)
private:
  hpcl_stat* m_read_reply_comp_8B_ratio;		//
  hpcl_stat* m_read_reply_comp_4B_ratio;		//
  hpcl_stat* m_read_reply_comp_2B_ratio;		//
  hpcl_stat* m_read_reply_comp_1B_ratio;		//

//added by kh(060316)
private:
  hpcl_stat* m_comp_data_ratio;		//This is compression ratio based on the number of bytes, not flits

//added by kh(060616)
/*
  hpcl_stat* m_trans_comp_2B_win_no;
  hpcl_stat* m_trans_comp_4B_win_no;
  hpcl_stat* m_trans_comp_8B_win_no;
*/

//added by kh(061016)
  std::vector<hpcl_stat*> m_max_word_index_freq_2B;
  std::vector<hpcl_stat*> m_max_word_index_freq_4B;
  std::vector<hpcl_stat*> m_max_word_index_freq_8B;
  std::vector<hpcl_stat*> m_max_word_index_freq_trans_2B;
  std::vector<hpcl_stat*> m_max_word_index_freq_trans_4B;
  std::vector<hpcl_stat*> m_max_word_index_freq_trans_8B;
///

//added by kh(061016)
  std::vector<hpcl_stat*> m_comp_data_ratio_segment_2B;
  std::vector<hpcl_stat*> m_comp_data_ratio_segment_4B;
  std::vector<hpcl_stat*> m_comp_data_ratio_segment_8B;
  std::vector<hpcl_stat*> m_no_comp_data_num_segment_2B;
  std::vector<hpcl_stat*> m_no_comp_data_num_segment_4B;
  std::vector<hpcl_stat*> m_no_comp_data_num_segment_8B;
///

//added by kh(062516)
  hpcl_stat* m_rec_comp_intra_sm_read_rep_group_size;
  hpcl_stat* m_rec_comp_inter_sm_read_rep_group_size;
///


//added by kh(062816)
  hpcl_stat* m_rec_comp_last_mf_not_found_cnt;
  hpcl_stat* m_rec_comp_last_mf_comp_res_mistmatch_cnt;
  hpcl_stat* m_rec_comp_last_mf_full_cnt;

//added by kh(070416)
  hpcl_stat* m_dsc_comp_0B_no;		//no compression benefit case
  hpcl_stat* m_dsc_comp_2B_no;
  hpcl_stat* m_dsc_comp_4B_no;
  hpcl_stat* m_dsc_comp_8B_no;
  hpcl_stat* m_dsc_comp_16B_no;
  hpcl_stat* m_dsc_comp_32B_no;
  hpcl_stat* m_dsc_comp_64B_no;
  hpcl_stat* m_dsc_comp_128B_no;

  hpcl_stat* m_data_type0_no;
  hpcl_stat* m_data_type1_no;
  hpcl_stat* m_data_type2_no;
  hpcl_stat* m_data_type3_no;
  hpcl_stat* m_data_type4_no;
  hpcl_stat* m_data_type5_no;
  hpcl_stat* m_data_type6_no;


//added by kh(071817)
  hpcl_stat* m_data_type7_no;
///

  hpcl_stat* m_ext_lwm_data_type0_no;
  hpcl_stat* m_ext_lwm_data_type1_no;
  hpcl_stat* m_ext_lwm_data_type2_no;
  hpcl_stat* m_ext_lwm_data_type3_no;
  hpcl_stat* m_ext_lwm_data_type4_no;
  hpcl_stat* m_ext_lwm_data_type5_no;
  hpcl_stat* m_ext_lwm_data_type6_no;

  hpcl_stat* m_read_req_no;

//added by kh(071216)
  hpcl_stat* m_dsm_comp_no;
  hpcl_stat* m_lwm_comp_no;
  hpcl_stat* m_no_comp_no;
  hpcl_stat* m_dsm_comp_data_ratio;
  hpcl_stat* m_lwm_comp_data_ratio;
///

//added by kh(071516)
  hpcl_stat* m_rec_comp_intra_sm_same_data_merge_no;

//added by kh(072316)
  hpcl_stat* m_dsm_comp_remap_op_case_no;
  hpcl_stat* m_dsm_comp_remap_mask_op_no;
  hpcl_stat* m_dsm_comp_remap_comp_op_no;
  hpcl_stat* m_dsm_comp_remap_neighbor_const_delta_op_no;

  hpcl_stat* m_dsm_comp_remap_delta_upto_1_op_no;
  hpcl_stat* m_dsm_comp_remap_delta_upto_3_op_no;
  hpcl_stat* m_dsm_comp_remap_comp_delta_upto_1_op_no;
  hpcl_stat* m_dsm_comp_remap_comp_delta_upto_3_op_no;
///


//added by kh(072716)
  hpcl_stat* m_comp_packet_no;

//added by kh(072816)
  hpcl_stat* m_approx_data_no;
  hpcl_stat* m_approx_exception_no;

};

#endif /* GPGPU_SIM_HPCL_COMP_ANAL_H_ */
