/*
 * hpcl_comp_anal.cc
 *
 *  Created on: Mar 8, 2016
 *      Author: mumichang
 */

#include "hpcl_comp_anal.h"
#include <cassert>

hpcl_comp_anal::hpcl_comp_anal ()
{
  // TODO Auto-generated constructor stub

}

hpcl_comp_anal::~hpcl_comp_anal ()
{
  // TODO Auto-generated destructor stub
}


void hpcl_comp_anal::create(unsigned sub_partition_no)
{
  m_comp_table_hit_no = new hpcl_stat;
  m_comp_table_miss_no = new hpcl_stat;
//  m_read_reply_comp_ratio = new hpcl_stat;	//added by kh(041616)
//  m_ovr_avg_read_reply_comp_ratio = new hpcl_stat;	//added by kh(041616)
  m_ovr_avg_comp_table_hit_rate = new hpcl_stat;

  /*
  //added by kh(041616)
  m_fragment_rate_per_flit = new hpcl_stat;
  m_ovr_avg_fragment_rate_per_flit = new hpcl_stat;
  m_fragment_rate_per_packet = new hpcl_stat;
  m_ovr_avg_fragment_rate_per_packet = new hpcl_stat;
  m_comp_overhead_no = new hpcl_stat;
  m_ovr_avg_comp_overhead_rate = new hpcl_stat;
  ///
  */

  //added by kh(041816)
  m_org_flit_no = new hpcl_stat;
  m_ovr_avg_org_flit_no = new hpcl_stat;
  m_comp_flit_no = new hpcl_stat;
  m_ovr_avg_comp_flit_no = new hpcl_stat;
  ///

  //added by kh(041816)
  m_comp_0B_no = new hpcl_stat;	//added by kh(060216)
  m_comp_2B_no = new hpcl_stat;
  m_comp_4B_no = new hpcl_stat;
  m_comp_8B_no = new hpcl_stat;
  ///

  //added by kh(042116)
  m_word2B_zeros_rep_rate = new hpcl_stat;
  m_word2B_nonzeros_rep_rate = new hpcl_stat;
  m_word2B_uni_val_rate = new hpcl_stat;
  m_ovr_avg_word2B_zeros_rep_rate = new hpcl_stat;
  m_ovr_avg_word2B_nonzeros_rep_rate = new hpcl_stat;
  m_ovr_avg_word2B_uni_val_rate = new hpcl_stat;

  m_word4B_zeros_rep_rate = new hpcl_stat;
  m_word4B_nonzeros_rep_rate = new hpcl_stat;
  m_word4B_uni_val_rate = new hpcl_stat;
  m_ovr_avg_word4B_zeros_rep_rate = new hpcl_stat;
  m_ovr_avg_word4B_nonzeros_rep_rate = new hpcl_stat;
  m_ovr_avg_word4B_uni_val_rate = new hpcl_stat;

  m_word8B_zeros_rep_rate = new hpcl_stat;
  m_word8B_nonzeros_rep_rate = new hpcl_stat;
  m_word8B_uni_val_rate = new hpcl_stat;
  m_ovr_avg_word8B_zeros_rep_rate = new hpcl_stat;
  m_ovr_avg_word8B_nonzeros_rep_rate = new hpcl_stat;
  m_ovr_avg_word8B_uni_val_rate = new hpcl_stat;
  ///
/*
  //added by kh(042116)
  m_single_rep_to_same_sm_rate.resize(sub_partition_no, NULL);
  m_multi_rep_to_same_sm_rate.resize(sub_partition_no, NULL);
  m_no_rep_to_same_sm_rate.resize(sub_partition_no, NULL);
  for(unsigned i = 0; i < sub_partition_no; i++) {
    m_single_rep_to_same_sm_rate[i] = new hpcl_stat;
    m_multi_rep_to_same_sm_rate[i] = new hpcl_stat;
    m_no_rep_to_same_sm_rate[i] = new hpcl_stat;
  }
  m_ovr_avg_single_rep_to_same_sm_rate = new hpcl_stat;
  m_ovr_avg_multi_rep_to_same_sm_rate = new hpcl_stat;
  m_ovr_avg_no_rep_to_same_sm_rate = new hpcl_stat;

  m_multi_rep_to_same_sm_no = new hpcl_stat;
  m_ovr_avg_multi_rep_to_same_sm_no = new hpcl_stat;
  ///
*/
/*
  //added by kh(051916)
  m_inter_comp_packet_no = new hpcl_stat;
  m_intra_comp_packet_no = new hpcl_stat;
  m_ovr_avg_inter_comp_packet_rate = new hpcl_stat;
  m_inter_comp_matching_word_no = new hpcl_stat;
  m_ovr_avg_inter_comp_matching_word_no = new hpcl_stat;
  ///
*/
  //added by kh(052016)
  m_read_reply_comp_ratio_across_kernels = new hpcl_stat;
  m_inter_comp_packet_no_across_kernels = new hpcl_stat;
  m_intra_comp_packet_no_across_kernels = new hpcl_stat;
  m_inter_comp_matching_word_no_across_kernels = new hpcl_stat;
  m_fragment_rate_per_flit_across_kernels = new hpcl_stat;
  m_fragment_rate_per_packet_across_kernels = new hpcl_stat;
  ///

  //added by kh(060216)
  m_read_reply_comp_8B_ratio = new hpcl_stat;
  m_read_reply_comp_4B_ratio = new hpcl_stat;
  m_read_reply_comp_2B_ratio = new hpcl_stat;
  //m_read_reply_comp_1B_ratio = new hpcl_stat;
  ///

  //added by kh(060316)
  m_comp_data_ratio = new hpcl_stat;
  ///
/*
  //added by kh(060616)
  m_trans_comp_2B_win_no = new hpcl_stat;
  m_trans_comp_4B_win_no = new hpcl_stat;
  m_trans_comp_8B_win_no = new hpcl_stat;
  ///
*/

  //added by kh(060916)
  m_max_word_index_freq_2B.resize(64, NULL);
  m_max_word_index_freq_4B.resize(32, NULL);
  m_max_word_index_freq_8B.resize(16, NULL);
  m_max_word_index_freq_trans_2B.resize(64, NULL);
  m_max_word_index_freq_trans_4B.resize(32, NULL);
  m_max_word_index_freq_trans_8B.resize(16, NULL);

  for(unsigned i = 0; i < m_max_word_index_freq_2B.size(); i++) {
    m_max_word_index_freq_2B[i] = new hpcl_stat;
    m_max_word_index_freq_trans_2B[i] = new hpcl_stat;
  }

  for(unsigned i = 0; i < m_max_word_index_freq_4B.size(); i++) {
    m_max_word_index_freq_4B[i] = new hpcl_stat;
    m_max_word_index_freq_trans_4B[i] = new hpcl_stat;
  }

  for(unsigned i = 0; i < m_max_word_index_freq_8B.size(); i++) {
    m_max_word_index_freq_8B[i] = new hpcl_stat;
    m_max_word_index_freq_trans_8B[i] = new hpcl_stat;
  }
  ///

  //added by kh(061016)
  m_comp_data_ratio_segment_2B.resize(4, NULL);
  m_comp_data_ratio_segment_4B.resize(4, NULL);
  m_comp_data_ratio_segment_8B.resize(4, NULL);
  for(unsigned i = 0; i < m_comp_data_ratio_segment_2B.size(); i++) {
    m_comp_data_ratio_segment_2B[i] = new hpcl_stat;
    m_comp_data_ratio_segment_4B[i] = new hpcl_stat;
    m_comp_data_ratio_segment_8B[i] = new hpcl_stat;
  }

  m_no_comp_data_num_segment_2B.resize(4, NULL);
  m_no_comp_data_num_segment_4B.resize(4, NULL);
  m_no_comp_data_num_segment_8B.resize(4, NULL);
  for(unsigned i = 0; i < m_no_comp_data_num_segment_2B.size(); i++) {
    m_no_comp_data_num_segment_2B[i] = new hpcl_stat;
    m_no_comp_data_num_segment_4B[i] = new hpcl_stat;
    m_no_comp_data_num_segment_8B[i] = new hpcl_stat;
  }
  ///

  //added by kh(062516)
  m_rec_comp_intra_sm_read_rep_group_size = new hpcl_stat;
  m_rec_comp_inter_sm_read_rep_group_size = new hpcl_stat;
  ///

  //added by kh(062816)
  m_rec_comp_last_mf_not_found_cnt = new hpcl_stat;
  m_rec_comp_last_mf_comp_res_mistmatch_cnt = new hpcl_stat;
  m_rec_comp_last_mf_full_cnt = new hpcl_stat;
  ///

  //added by kh(070416)
  m_dsc_comp_0B_no = new hpcl_stat;
  m_dsc_comp_2B_no = new hpcl_stat;
  m_dsc_comp_4B_no = new hpcl_stat;
  m_dsc_comp_8B_no = new hpcl_stat;
  m_dsc_comp_16B_no = new hpcl_stat;
  m_dsc_comp_32B_no = new hpcl_stat;
  m_dsc_comp_64B_no = new hpcl_stat;
  m_dsc_comp_128B_no = new hpcl_stat;
  m_data_type0_no = new hpcl_stat;
  m_data_type1_no = new hpcl_stat;
  m_data_type2_no = new hpcl_stat;
  m_data_type3_no = new hpcl_stat;
  m_data_type4_no = new hpcl_stat;
  m_data_type5_no = new hpcl_stat;
  m_data_type6_no = new hpcl_stat;
  //added by kh(021817)
  m_data_type7_no = new hpcl_stat;
  ///

  m_ext_lwm_data_type0_no = new hpcl_stat;
  m_ext_lwm_data_type1_no = new hpcl_stat;
  m_ext_lwm_data_type2_no = new hpcl_stat;
  m_ext_lwm_data_type3_no = new hpcl_stat;
  m_ext_lwm_data_type4_no = new hpcl_stat;
  m_ext_lwm_data_type5_no = new hpcl_stat;
  m_ext_lwm_data_type6_no = new hpcl_stat;

  ///

  //added by kh(070716)
  m_read_req_no = new hpcl_stat;
  ///

  //added by kh(071216)
  m_dsm_comp_no = new hpcl_stat;
  m_lwm_comp_no = new hpcl_stat;
  m_no_comp_no = new hpcl_stat;
  m_dsm_comp_data_ratio = new hpcl_stat;
  m_lwm_comp_data_ratio = new hpcl_stat;
  ///

  //added by kh(071516)
  m_rec_comp_intra_sm_same_data_merge_no = new hpcl_stat;

  //added by kh(072316)
  m_dsm_comp_remap_op_case_no = new hpcl_stat;
  m_dsm_comp_remap_mask_op_no = new hpcl_stat;
  m_dsm_comp_remap_comp_op_no = new hpcl_stat;
  m_dsm_comp_remap_neighbor_const_delta_op_no = new hpcl_stat;
  m_dsm_comp_remap_delta_upto_1_op_no = new hpcl_stat;
  m_dsm_comp_remap_delta_upto_3_op_no = new hpcl_stat;
  m_dsm_comp_remap_comp_delta_upto_1_op_no = new hpcl_stat;
  m_dsm_comp_remap_comp_delta_upto_3_op_no = new hpcl_stat;

  //added by kh(072716)
  m_comp_packet_no = new hpcl_stat;

  //added by kh(072816)
  m_approx_data_no = new hpcl_stat;
  m_approx_exception_no = new hpcl_stat;

  //added by kh(030117)
  m_read_reply_cache_comp_ratio_across_kernels = new hpcl_stat;
  ///

}


void hpcl_comp_anal::add_sample(enum sample_type type, double val, int id)
{
  if(type == GLOBAL_TABLE_HIT_NO)  	 	m_comp_table_hit_no->add_sample(val);
  else if(type == GLOBAL_TABLE_MISS_NO)  m_comp_table_miss_no->add_sample(val);
  else if(type == PACKET_COMP_RATIO) {
      //m_read_reply_comp_ratio->add_sample(val);
      //added by kh(052016)
      m_read_reply_comp_ratio_across_kernels->add_sample(val);
      //std::cout << "my_comp_ratio " << val << std::endl;
  }
  //added by kh(041616)
  else if(type == FRAGMENT_RATE_PER_FLIT)	{
      //m_fragment_rate_per_flit->add_sample(val);
      //added by kh(052016)
      m_fragment_rate_per_flit_across_kernels->add_sample(val);
  }
  else if(type == FRAGMENT_RATE_PER_PACKET)	{
      //m_fragment_rate_per_packet->add_sample(val);
      //added by kh(052016)
      m_fragment_rate_per_packet_across_kernels->add_sample(val);
  }
  //else if(type == COMP_OVERHEAD_NO)	 	m_comp_overhead_no->add_sample(val);
  //added by kh(041816)
  else if(type == ORG_FLIT_NO)		 	m_org_flit_no->add_sample(val);
  else if(type == COMP_FLIT_NO)			m_comp_flit_no->add_sample(val);
  else if(type == COMP_0B_NO)		 	m_comp_0B_no->add_sample(val);	//added by kh(060216)
  else if(type == COMP_2B_NO)		 	m_comp_2B_no->add_sample(val);
  else if(type == COMP_4B_NO)			m_comp_4B_no->add_sample(val);
  else if(type == COMP_8B_NO)		 	m_comp_8B_no->add_sample(val);
  //added by kh(042116)
  else if(type == WORD2B_ZEROS_REP_RATE)	m_word2B_zeros_rep_rate->add_sample(val);
  else if(type == WORD2B_NONZEROS_REP_RATE)	m_word2B_nonzeros_rep_rate->add_sample(val);
  else if(type == WORD2B_UNI_VAL_RATE)		m_word2B_uni_val_rate->add_sample(val);
  else if(type == WORD4B_ZEROS_REP_RATE)	m_word4B_zeros_rep_rate->add_sample(val);
  else if(type == WORD4B_NONZEROS_REP_RATE)	m_word4B_nonzeros_rep_rate->add_sample(val);
  else if(type == WORD4B_UNI_VAL_RATE)		m_word4B_uni_val_rate->add_sample(val);
  else if(type == WORD8B_ZEROS_REP_RATE)	m_word8B_zeros_rep_rate->add_sample(val);
  else if(type == WORD8B_NONZEROS_REP_RATE)	m_word8B_nonzeros_rep_rate->add_sample(val);
  else if(type == WORD8B_UNI_VAL_RATE)		m_word8B_uni_val_rate->add_sample(val);
  /*
  else if(type == SINGLE_REP_TO_SAME_SM_RATE)	m_single_rep_to_same_sm_rate[id]->add_sample(val);
  else if(type == MULTI_REP_TO_SAME_SM_RATE)	m_multi_rep_to_same_sm_rate[id]->add_sample(val);
  else if(type == NO_REP_TO_SAME_SM_RATE)	m_no_rep_to_same_sm_rate[id]->add_sample(val);
  else if(type == MULTI_REP_TO_SAME_SM_NO)	m_multi_rep_to_same_sm_no->add_sample(val);
  */
  //added by kh(042216)
//  else if(type == WORD1B_ZEROS_REP_RATE_IN_UNCOMP_DATA)		m_word1B_zeros_rep_rate_in_uncomp_data->add_sample(val);
//  else if(type == WORD1B_NONZEROS_REP_RATE_IN_UNCOMP_DATA)	m_word1B_nonzeros_rep_rate_in_uncomp_data->add_sample(val);
//  else if(type == WORD1B_UNI_VAL_RATE_IN_UNCOMP_DATA)		m_word1B_uni_val_rate_in_uncomp_data->add_sample(val);
//  else if(type == WORD1B_NONZEROS_REP_NO_IN_UNCOMP_DATA)	m_word1B_nonzeros_rep_no_in_uncomp_data->add_sample(val);
//  //added by kh(042316)
//  else if(type == WORD1B_REDUND_RATE_INTER_READ_REPLIES)	m_word1B_word1B_redund_rate_inter_read_replies->add_sample(val);
  //added by kh(051916)
  else if(type == INTER_COMP_PACKET_NO)	{
    //m_inter_comp_packet_no->add_sample(val);
    //added by kh(052016)
    m_inter_comp_packet_no_across_kernels->add_sample(val);
  }
  else if(type == INTRA_COMP_PACKET_NO)	{
    //m_intra_comp_packet_no->add_sample(val);
    //added by kh(052016)
    m_intra_comp_packet_no_across_kernels->add_sample(val);
  }
  else if(type == INTER_COMP_MATCHING_WORD_NO)	{
    //m_inter_comp_matching_word_no->add_sample(val);
    //added by kh(052016)
    m_inter_comp_matching_word_no_across_kernels->add_sample(val);
  }
  //added by kh(060216)
  else if(type == PACKET_COMP_8B_RATIO)	{
    m_read_reply_comp_8B_ratio->add_sample(val);
  }
  else if(type == PACKET_COMP_4B_RATIO)	{
    m_read_reply_comp_4B_ratio->add_sample(val);
  }
  else if(type == PACKET_COMP_2B_RATIO)	{
    m_read_reply_comp_2B_ratio->add_sample(val);
  }
//  else if(type == READ_REPLY_COMP_1B_RATIO)	{
//    m_read_reply_comp_1B_ratio->add_sample(val);
//  }
  //added by kh(060316)
  else if(type == COMP_DATA_RATIO) {
    m_comp_data_ratio->add_sample(val);
  }
/*
  //added by kh(060616)
  else if(type == TRANS_COMP_2B_WIN_NO) {
    m_trans_comp_2B_win_no->add_sample(val);
  }
  else if(type == TRANS_COMP_4B_WIN_NO) {
    m_trans_comp_4B_win_no->add_sample(val);
  }
  else if(type == TRANS_COMP_8B_WIN_NO) {
    m_trans_comp_8B_win_no->add_sample(val);
  }
*/
  //added by kh(061016)
  else if(type == MAX_WORD_INDEX_2B) {
    m_max_word_index_freq_2B[id]->add_sample(val);
  }
  else if(type == MAX_WORD_INDEX_4B) {
    m_max_word_index_freq_4B[id]->add_sample(val);
  }
  else if(type == MAX_WORD_INDEX_8B) {
    m_max_word_index_freq_8B[id]->add_sample(val);
  }
  else if(type == MAX_WORD_INDEX_TRANS_2B) {
    m_max_word_index_freq_trans_2B[id]->add_sample(val);
  }
  else if(type == MAX_WORD_INDEX_TRANS_4B) {
    m_max_word_index_freq_trans_4B[id]->add_sample(val);
  }
  else if(type == MAX_WORD_INDEX_TRANS_8B) {
    m_max_word_index_freq_trans_8B[id]->add_sample(val);
  }
  else if(type == COMP_DATA_RATIO_SEGMENT_2B) {
    m_comp_data_ratio_segment_2B[id]->add_sample(val);
  }
  else if(type == COMP_DATA_RATIO_SEGMENT_4B) {
    m_comp_data_ratio_segment_4B[id]->add_sample(val);
  }
  else if(type == COMP_DATA_RATIO_SEGMENT_8B) {
    m_comp_data_ratio_segment_8B[id]->add_sample(val);
  }
  else if(type == NO_COMP_DATA_NUM_SEGMENT_2B) {
    m_no_comp_data_num_segment_2B[id]->add_sample(val);
  }
  else if(type == NO_COMP_DATA_NUM_SEGMENT_4B) {
    m_no_comp_data_num_segment_4B[id]->add_sample(val);
  }
  else if(type == NO_COMP_DATA_NUM_SEGMENT_8B) {
    m_no_comp_data_num_segment_8B[id]->add_sample(val);
  }
  //added by kh(062516)
  else if(type == REC_COMP_INTRA_SM_READ_REP_GROUP_SIZE) {
    m_rec_comp_intra_sm_read_rep_group_size->add_sample(val);
  }
  else if(type == REC_COMP_INTER_SM_READ_REP_GROUP_SIZE) {
    m_rec_comp_inter_sm_read_rep_group_size->add_sample(val);
  }
  //added by kh(062816)
  else if(type == REC_COMP_LAST_MF_NOT_FOUND) {
    m_rec_comp_last_mf_not_found_cnt->add_sample(val);
  }
  else if(type == REC_COMP_LAST_MF_COMP_RES_MISMATCH) {
    m_rec_comp_last_mf_comp_res_mistmatch_cnt->add_sample(val);
  }
  else if(type == REC_COMP_LAST_MF_FULL) {
    m_rec_comp_last_mf_full_cnt->add_sample(val);
  }
  //added by kh(070416)
  else if(type == DSC_COMP_0B_NO) {
    m_dsc_comp_0B_no->add_sample(val);
  }
  else if(type == DSC_COMP_2B_NO) {
    m_dsc_comp_2B_no->add_sample(val);
  }
  else if(type == DSC_COMP_4B_NO) {
    m_dsc_comp_4B_no->add_sample(val);
  }
  else if(type == DSC_COMP_8B_NO) {
    m_dsc_comp_8B_no->add_sample(val);
  }
  else if(type == DSC_COMP_16B_NO) {
    m_dsc_comp_16B_no->add_sample(val);
  }
  else if(type == DSC_COMP_32B_NO) {
    m_dsc_comp_32B_no->add_sample(val);
  }
  else if(type == DSC_COMP_64B_NO) {
    m_dsc_comp_64B_no->add_sample(val);
  }
  else if(type == DSC_COMP_128B_NO) {
    m_dsc_comp_128B_no->add_sample(val);
  }
  else if(type == DATA_TYPE0_NO) {
    m_data_type0_no->add_sample(val);
  }
  else if(type == DATA_TYPE1_NO) {
    m_data_type1_no->add_sample(val);
  }
  else if(type == DATA_TYPE2_NO) {
    m_data_type2_no->add_sample(val);
  }
  else if(type == DATA_TYPE3_NO) {
    m_data_type3_no->add_sample(val);
  }
  else if(type == DATA_TYPE4_NO) {
    m_data_type4_no->add_sample(val);
  }
  else if(type == DATA_TYPE5_NO) {
    m_data_type5_no->add_sample(val);
  }
  else if(type == DATA_TYPE6_NO) {
    m_data_type6_no->add_sample(val);
  }
  else if(type == DATA_TYPE7_NO) {
    m_data_type7_no->add_sample(val);
  }
  else if(type == EXT_LWM_DATA_TYPE0_NO) {
    m_ext_lwm_data_type0_no->add_sample(val);
  }
  else if(type == EXT_LWM_DATA_TYPE1_NO) {
    m_ext_lwm_data_type1_no->add_sample(val);
  }
  else if(type == EXT_LWM_DATA_TYPE2_NO) {
    m_ext_lwm_data_type2_no->add_sample(val);
  }
  else if(type == EXT_LWM_DATA_TYPE3_NO) {
    m_ext_lwm_data_type3_no->add_sample(val);
  }
  else if(type == EXT_LWM_DATA_TYPE4_NO) {
    m_ext_lwm_data_type4_no->add_sample(val);
  }
  else if(type == EXT_LWM_DATA_TYPE5_NO) {
    m_ext_lwm_data_type5_no->add_sample(val);
  }
  else if(type == EXT_LWM_DATA_TYPE6_NO) {
    m_ext_lwm_data_type6_no->add_sample(val);
  }
  else if(type == READ_REQ_NO) {
      m_read_req_no->add_sample(val);
  }
  //added by kh(071216)
  else if(type == DSM_COMP_NO) {
    m_dsm_comp_no->add_sample(val);
  }
  else if(type == LWM_COMP_NO) {
    m_lwm_comp_no->add_sample(val);
  }
  else if(type == NO_COMP_NO) {
    m_no_comp_no->add_sample(val);
  }
  else if(type == DSM_COMP_DATA_RATIO) {
    m_dsm_comp_data_ratio->add_sample(val);
  }
  else if(type == LWM_COMP_DATA_RATIO) {
    m_lwm_comp_data_ratio->add_sample(val);
  }
  else if(type == REC_COMP_INTRA_SM_SAME_DATA_MERGE_NO) {
    m_rec_comp_intra_sm_same_data_merge_no->add_sample(val);
  }
  //added by kh(072316)
  else if(type == DSM_COMP_REMAP_OP_CASE_NO) {
    m_dsm_comp_remap_op_case_no->add_sample(val);
  }
  else if(type == DSM_COMP_REMAP_MASK_OP_NO) {
    m_dsm_comp_remap_mask_op_no->add_sample(val);
  }
  else if(type == DSM_COMP_REMAP_COMP_OP_NO) {
    m_dsm_comp_remap_comp_op_no->add_sample(val);
  }
  else if(type == DSM_COMP_REMAP_NEIGHBOR_CONST_DELTA_OP_NO) {
    m_dsm_comp_remap_neighbor_const_delta_op_no->add_sample(val);
  }
  else if(type == DSM_COMP_REMAP_DELTA_UPTO_1_OP_NO) {
    m_dsm_comp_remap_delta_upto_1_op_no->add_sample(val);
  }
  else if(type == DSM_COMP_REMAP_DELTA_UPTO_3_OP_NO) {
    m_dsm_comp_remap_delta_upto_3_op_no->add_sample(val);
  }
  else if(type == DSM_COMP_REMAP_COMP_DELTA_UPTO_1_OP_NO) {
    m_dsm_comp_remap_comp_delta_upto_1_op_no->add_sample(val);
  }
  else if(type == DSM_COMP_REMAP_COMP_DELTA_UPTO_3_OP_NO) {
    m_dsm_comp_remap_comp_delta_upto_3_op_no->add_sample(val);
  }
  else if(type == COMP_PACKET_NO) {
    m_comp_packet_no->add_sample(val);
  }
  else if(type == APPROX_DATA_NO) {
    m_approx_data_no->add_sample(val);
  }
  else if(type == APPROX_EXCEPTION_NO) {
    m_approx_exception_no->add_sample(val);
  }
  //added by kh(030117)
  else if(type == CACHE_COMP_RATIO) {
	m_read_reply_cache_comp_ratio_across_kernels->add_sample(val);
    //std::cout << "my_comp_ratio " << val << std::endl;
  }
  else {
   std::cout << "type " << type << std::endl;
    assert(0);
  }
}

void hpcl_comp_anal::display(std::ostream & os) const
{
  os << "====== hpcl_comp_anal ======" << std::endl;
  os << "gpu_tot_ovr_avg_comp_table_hit_rate = " << m_ovr_avg_comp_table_hit_rate->avg() << std::endl;
  //os << "gpu_tot_ovr_avg_read_reply_comp_ratio = " << m_ovr_avg_read_reply_comp_ratio->avg()*100 << " \%" << std::endl;
  /*
  os << "gpu_tot_ovr_avg_fragment_rate_per_flit = " << m_ovr_avg_fragment_rate_per_flit->avg()*100 << " \%" << std::endl;
  os << "gpu_tot_ovr_avg_fragment_rate_per_packet = " << m_ovr_avg_fragment_rate_per_packet->avg()*100 << " \%" << std::endl;
  os << "gpu_tot_ovr_avg_comp_overhead_rate = " << m_ovr_avg_comp_overhead_rate->avg() << std::endl;
  */

  //added by kh(041816)
  os << "gpu_tot_ovr_avg_org_flit_no = " << m_ovr_avg_org_flit_no->avg() << std::endl;		//added by kh(041816)
  os << "gpu_tot_ovr_avg_comp_flit_no = " << m_ovr_avg_comp_flit_no->avg() << std::endl;	//added by kh(041816)

  //deleted by kh(060216)
  //os << "gpu_tot_ovr_avg_comp_2B_rate = " << m_ovr_avg_comp_2B_rate->avg()*100 << " \%" << std::endl;	//added by kh(041816)
  //os << "gpu_tot_ovr_avg_comp_4B_rate = " << m_ovr_avg_comp_4B_rate->avg()*100 << " \%" << std::endl;	//added by kh(041816)
  //os << "gpu_tot_ovr_avg_comp_8B_rate = " << m_ovr_avg_comp_8B_rate->avg()*100 << " \%" << std::endl;	//added by kh(041816)

  //added by kh(060216)
  double total_comp_data_no = m_comp_0B_no->sum()+m_comp_2B_no->sum()+m_comp_4B_no->sum()+m_comp_8B_no->sum();
  //if(total_comp_data_no != 0) {
    os << "gpu_tot_ovr_avg_comp_2B_rate = " << m_comp_2B_no->sum()/total_comp_data_no*100 << " \%" << std::endl;
    os << "gpu_tot_ovr_avg_comp_4B_rate = " << m_comp_4B_no->sum()/total_comp_data_no*100 << " \%" << std::endl;
    os << "gpu_tot_ovr_avg_comp_8B_rate = " << m_comp_8B_no->sum()/total_comp_data_no*100 << " \%" << std::endl;
    os << "gpu_tot_ovr_avg_comp_0B_rate = " << m_comp_0B_no->sum()/total_comp_data_no*100 << " \%" << std::endl;
  //}
  ///

  //added by kh(042116)
  os << "gpu_tot_ovr_avg_word2B_zeros_rep_rate = " << m_ovr_avg_word2B_zeros_rep_rate->avg()*100 << " \%" << std::endl;
  os << "gpu_tot_ovr_avg_word2B_nonzeros_rep_rate = " << m_ovr_avg_word2B_nonzeros_rep_rate->avg()*100 << " \%" << std::endl;
  os << "gpu_tot_ovr_avg_word2B_uni_val_rate = " << m_ovr_avg_word2B_uni_val_rate->avg()*100 << " \%" << std::endl;
  os << "gpu_tot_ovr_avg_word4B_zeros_rep_rate = " << m_ovr_avg_word4B_zeros_rep_rate->avg()*100 << " \%" << std::endl;
  os << "gpu_tot_ovr_avg_word4B_nonzeros_rep_rate = " << m_ovr_avg_word4B_nonzeros_rep_rate->avg()*100 << " \%" << std::endl;
  os << "gpu_tot_ovr_avg_word4B_uni_val_rate = " << m_ovr_avg_word4B_uni_val_rate->avg()*100 << " \%" << std::endl;
  os << "gpu_tot_ovr_avg_word8B_zeros_rep_rate = " << m_ovr_avg_word8B_zeros_rep_rate->avg()*100 << " \%" << std::endl;
  os << "gpu_tot_ovr_avg_word8B_nonzeros_rep_rate = " << m_ovr_avg_word8B_nonzeros_rep_rate->avg()*100 << " \%" << std::endl;
  os << "gpu_tot_ovr_avg_word8B_uni_val_rate = " << m_ovr_avg_word8B_uni_val_rate->avg()*100 << " \%" << std::endl;
  /*
  os << "gpu_tot_ovr_avg_single_rep_to_same_sm_rate = " << m_ovr_avg_single_rep_to_same_sm_rate->avg()*100 << " \%" << std::endl;
  os << "gpu_tot_ovr_avg_multi_rep_to_same_sm_rate = " << m_ovr_avg_multi_rep_to_same_sm_rate->avg()*100 << " \%" << std::endl;
  os << "gpu_tot_ovr_avg_no_rep_to_same_sm_rate = " << m_ovr_avg_no_rep_to_same_sm_rate->avg()*100 << " \%" << std::endl;
  os << "gpu_tot_ovr_avg_multi_rep_to_same_sm_no = " << m_ovr_avg_multi_rep_to_same_sm_no->avg() << std::endl;
  */
  ///

  //added by kh(042216)
//  os << "gpu_tot_ovr_avg_word1B_zeros_rep_rate_in_uncomp_data = " << m_ovr_avg_word1B_zeros_rep_rate_in_uncomp_data->avg() << std::endl;
//  os << "gpu_tot_ovr_avg_word1B_nonzeros_rep_rate_in_uncomp_data = " << m_ovr_avg_word1B_nonzeros_rep_rate_in_uncomp_data->avg() << std::endl;
//  os << "gpu_tot_ovr_avg_word1B_uni_val_rate_in_uncomp_data = " << m_ovr_avg_word1B_uni_val_rate_in_uncomp_data->avg() << std::endl;
//  os << "gpu_tot_ovr_avg_word1B_nonzeros_rep_no_in_uncomp_data = " << m_ovr_avg_word1B_nonzeros_rep_no_in_uncomp_data->avg() << std::endl;
  ///

  //added by kh(042316)
//  os << "gpu_tot_ovr_avg_word1B_redund_rate_inter_read_replies = " << m_ovr_avg_word1B_word1B_redund_rate_inter_read_replies->avg() << std::endl;
  ///

  //added by kh(051916)
  //os << "gpu_tot_ovr_avg_inter_comp_packet_rate = " << m_ovr_avg_inter_comp_packet_rate->avg() << " \%" << std::endl;
  //os << "gpu_tot_ovr_avg_inter_comp_matching_word_no = " << m_ovr_avg_inter_comp_matching_word_no->avg() << std::endl;
  ///

  //added by kh(052016)
  os << "gpu_tot_packet_comp_ratio = " << m_read_reply_comp_ratio_across_kernels->avg()*100 << " \%" << std::endl;
  os << "gpu_tot_packet_comp_8B_ratio = " << m_read_reply_comp_8B_ratio->avg()*100 << " \%" << std::endl;
  os << "gpu_tot_packet_comp_4B_ratio = " << m_read_reply_comp_4B_ratio->avg()*100 << " \%" << std::endl;
  os << "gpu_tot_packet_comp_2B_ratio = " << m_read_reply_comp_2B_ratio->avg()*100 << " \%" << std::endl;

  double inter_comp_packet_rate = m_inter_comp_packet_no_across_kernels->sum() / (m_inter_comp_packet_no_across_kernels->sum()+m_intra_comp_packet_no_across_kernels->sum())*100;
  os << "gpu_tot_inter_comp_packet_rate = " << inter_comp_packet_rate << " \%" << std::endl;
  os << "gpu_tot_inter_comp_matching_word_no = " << m_inter_comp_matching_word_no_across_kernels->avg() << std::endl;
  os << "gpu_tot_fragment_rate_per_flit = " << m_fragment_rate_per_flit_across_kernels->avg()*100 << " \%" << std::endl;
  os << "gpu_tot_fragment_rate_per_packet = " << m_fragment_rate_per_packet_across_kernels->avg()*100 << " \%" << std::endl;
  os << "gpu_tot_inter_comp_packet_no = " << m_inter_comp_packet_no_across_kernels->sum() << std::endl;
  os << "gpu_tot_intra_comp_packet_no = " << m_intra_comp_packet_no_across_kernels->sum() << std::endl;
  os << "gpu_tot_avg_inter_comp_packet_no = " << m_inter_comp_packet_no_across_kernels->avg() << std::endl;
  ///

  //added by kh(060316)
  os << "gpu_tot_comp_data_ratio = " << m_comp_data_ratio->avg()*100 << " \%" << std::endl;
  os << "gpu_tot_comp_data_sample_no = " << m_comp_data_ratio->get_sample_no() << " samples" << std::endl;
  ///

  //added by kh(060616)
  /*
  os << "gpu_tot_tran_comp_2B_win_ratio = " << m_trans_comp_2B_win_no->sum()/m_comp_2B_no->sum()*100 << " \%" << std::endl;
  os << "gpu_tot_tran_comp_4B_win_ratio = " << m_trans_comp_4B_win_no->sum()/m_comp_4B_no->sum()*100 << " \%" << std::endl;
  os << "gpu_tot_tran_comp_8B_win_ratio = " << m_trans_comp_8B_win_no->sum()/m_comp_8B_no->sum()*100 << " \%" << std::endl;
  */
  ///

  //added by kh(061016)
  double max_word_no_2B = 0;
  double max_word_no_4B = 0;
  double max_word_no_8B = 0;
  double max_word_no_trans_2B = 0;
  double max_word_no_trans_4B = 0;
  double max_word_no_trans_8B = 0;

  for(unsigned i = 0; i < m_max_word_index_freq_2B.size(); i++) {
    max_word_no_2B += m_max_word_index_freq_2B[i]->sum();
    max_word_no_trans_2B += m_max_word_index_freq_trans_2B[i]->sum();
  }

  for(unsigned i = 0; i < m_max_word_index_freq_4B.size(); i++) {
    max_word_no_4B += m_max_word_index_freq_4B[i]->sum();
    max_word_no_trans_4B += m_max_word_index_freq_trans_4B[i]->sum();
  }

  for(unsigned i = 0; i < m_max_word_index_freq_8B.size(); i++) {
    max_word_no_8B += m_max_word_index_freq_8B[i]->sum();
    max_word_no_trans_8B += m_max_word_index_freq_trans_8B[i]->sum();
  }

  os << "gpu_tot_max_word_index_comp_2B = ";
  for(unsigned i = 0; i < m_max_word_index_freq_2B.size(); i++) {
    os << m_max_word_index_freq_2B[i]->sum()/max_word_no_2B << " ";
  }
  os << std::endl;

  os << "gpu_tot_max_word_index_comp_4B = ";
  for(unsigned i = 0; i < m_max_word_index_freq_4B.size(); i++) {
    os << m_max_word_index_freq_4B[i]->sum()/max_word_no_4B << " ";
  }
  os << std::endl;

  os << "gpu_tot_max_word_index_comp_8B = ";
  for(unsigned i = 0; i < m_max_word_index_freq_8B.size(); i++) {
    os << m_max_word_index_freq_8B[i]->sum()/max_word_no_8B << " ";
  }
  os << std::endl;

  os << "gpu_tot_max_word_index_trans_comp_2B = ";
  for(unsigned i = 0; i < m_max_word_index_freq_trans_2B.size(); i++) {
    os << m_max_word_index_freq_trans_2B[i]->sum()/max_word_no_trans_2B << " ";
  }
  os << std::endl;

  os << "gpu_tot_max_word_index_trans_comp_4B = ";
  for(unsigned i = 0; i < m_max_word_index_freq_trans_4B.size(); i++) {
    os << m_max_word_index_freq_trans_4B[i]->sum()/max_word_no_trans_4B << " ";
  }
  os << std::endl;

  os << "gpu_tot_max_word_index_trans_comp_8B = ";
  for(unsigned i = 0; i < m_max_word_index_freq_trans_8B.size(); i++) {
    os << m_max_word_index_freq_trans_8B[i]->sum()/max_word_no_trans_8B << " ";
  }
  os << std::endl;

  os << "gpu_tot_comp_2B_data_ratio_segments = ";
  for(unsigned i = 0; i < 4; i++) os << m_comp_data_ratio_segment_2B[i]->avg()*100 << " ";
  os << " \%" << std::endl;

  os << "gpu_tot_comp_4B_data_ratio_segments = ";
  for(unsigned i = 0; i < 4; i++) os << m_comp_data_ratio_segment_4B[i]->avg()*100 << " ";
  os << " \%" << std::endl;

  os << "gpu_tot_comp_8B_data_ratio_segments = ";
  for(unsigned i = 0; i < 4; i++) os << m_comp_data_ratio_segment_8B[i]->avg()*100 << " ";
  os << " \%" << std::endl;

  /*
  os << "gpu_tot_comp_2B_data_ratio_segment0 = " << m_comp_data_ratio_segment_2B[0]->avg()*100 << " \%" << std::endl;
  os << "gpu_tot_comp_2B_data_ratio_segment1 = " << m_comp_data_ratio_segment_2B[1]->avg()*100 << " \%" << std::endl;
  os << "gpu_tot_comp_2B_data_ratio_segment2 = " << m_comp_data_ratio_segment_2B[2]->avg()*100 << " \%" << std::endl;
  os << "gpu_tot_comp_2B_data_ratio_segment3 = " << m_comp_data_ratio_segment_2B[3]->avg()*100 << " \%" << std::endl;

  os << "gpu_tot_comp_4B_data_ratio_segment0 = " << m_comp_data_ratio_segment_4B[0]->avg()*100 << " \%" << std::endl;
  os << "gpu_tot_comp_4B_data_ratio_segment1 = " << m_comp_data_ratio_segment_4B[1]->avg()*100 << " \%" << std::endl;
  os << "gpu_tot_comp_4B_data_ratio_segment2 = " << m_comp_data_ratio_segment_4B[2]->avg()*100 << " \%" << std::endl;
  os << "gpu_tot_comp_4B_data_ratio_segment3 = " << m_comp_data_ratio_segment_4B[3]->avg()*100 << " \%" << std::endl;

  os << "gpu_tot_comp_8B_data_ratio_segment0 = " << m_comp_data_ratio_segment_8B[0]->avg()*100 << " \%" << std::endl;
  os << "gpu_tot_comp_8B_data_ratio_segment1 = " << m_comp_data_ratio_segment_8B[1]->avg()*100 << " \%" << std::endl;
  os << "gpu_tot_comp_8B_data_ratio_segment2 = " << m_comp_data_ratio_segment_8B[2]->avg()*100 << " \%" << std::endl;
  os << "gpu_tot_comp_8B_data_ratio_segment3 = " << m_comp_data_ratio_segment_8B[3]->avg()*100 << " \%" << std::endl;
  */
  os << "gpu_tot_no_comp_2B_data_ratio_segments = ";
  for(unsigned i = 0; i < 4; i++) os << m_no_comp_data_num_segment_2B[i]->sum()/m_comp_data_ratio_segment_2B[i]->get_sample_no()*100 << " ";
  os << " \%" << std::endl;

  os << "gpu_tot_no_comp_4B_data_ratio_segments = ";
  for(unsigned i = 0; i < 4; i++) os << m_no_comp_data_num_segment_4B[i]->sum()/m_comp_data_ratio_segment_4B[i]->get_sample_no()*100 << " ";
  os << " \%" << std::endl;

  os << "gpu_tot_no_comp_8B_data_ratio_segments = ";
  for(unsigned i = 0; i < 4; i++) os << m_no_comp_data_num_segment_8B[i]->sum()/m_comp_data_ratio_segment_8B[i]->get_sample_no()*100 << " ";
  os << " \%" << std::endl;

  /*
  os << "gpu_tot_no_comp_2B_data_ratio_segment0 = " << m_no_comp_data_num_segment_2B[0]->sum()/m_comp_data_ratio_segment_2B[0]->get_sample_no()*100 << " \%" << std::endl;
  os << "gpu_tot_no_comp_2B_data_ratio_segment1 = " << m_no_comp_data_num_segment_2B[1]->sum()/m_comp_data_ratio_segment_2B[1]->get_sample_no()*100 << " \%" << std::endl;
  os << "gpu_tot_no_comp_2B_data_ratio_segment2 = " << m_no_comp_data_num_segment_2B[2]->sum()/m_comp_data_ratio_segment_2B[2]->get_sample_no()*100 << " \%" << std::endl;
  os << "gpu_tot_no_comp_2B_data_ratio_segment3 = " << m_no_comp_data_num_segment_2B[3]->sum()/m_comp_data_ratio_segment_2B[3]->get_sample_no()*100 << " \%" << std::endl;

  os << "gpu_tot_no_comp_4B_data_ratio_segment0 = " << m_no_comp_data_num_segment_4B[0]->sum()/m_comp_data_ratio_segment_4B[0]->get_sample_no()*100 << " \%" << std::endl;
  os << "gpu_tot_no_comp_4B_data_ratio_segment1 = " << m_no_comp_data_num_segment_4B[1]->sum()/m_comp_data_ratio_segment_4B[1]->get_sample_no()*100 << " \%" << std::endl;
  os << "gpu_tot_no_comp_4B_data_ratio_segment2 = " << m_no_comp_data_num_segment_4B[2]->sum()/m_comp_data_ratio_segment_4B[2]->get_sample_no()*100 << " \%" << std::endl;
  os << "gpu_tot_no_comp_4B_data_ratio_segment3 = " << m_no_comp_data_num_segment_4B[3]->sum()/m_comp_data_ratio_segment_4B[3]->get_sample_no()*100 << " \%" << std::endl;

  os << "gpu_tot_no_comp_8B_data_ratio_segment0 = " << m_no_comp_data_num_segment_8B[0]->sum()/m_comp_data_ratio_segment_8B[0]->get_sample_no()*100 << " \%" << std::endl;
  os << "gpu_tot_no_comp_8B_data_ratio_segment1 = " << m_no_comp_data_num_segment_8B[1]->sum()/m_comp_data_ratio_segment_8B[1]->get_sample_no()*100 << " \%" << std::endl;
  os << "gpu_tot_no_comp_8B_data_ratio_segment2 = " << m_no_comp_data_num_segment_8B[2]->sum()/m_comp_data_ratio_segment_8B[2]->get_sample_no()*100 << " \%" << std::endl;
  os << "gpu_tot_no_comp_8B_data_ratio_segment3 = " << m_no_comp_data_num_segment_8B[3]->sum()/m_comp_data_ratio_segment_8B[3]->get_sample_no()*100 << " \%" << std::endl;
  */

  //added by kh(063016)
  //double rec_comp_intra_sm_read_rep_ratio = m_rec_comp_intra_sm_read_rep_group_size->sum()/total_comp_data_no*100;
  //double rec_comp_inter_sm_read_rep_ratio = m_rec_comp_inter_sm_read_rep_group_size->sum()/total_comp_data_no*100;
  //added by kh(071916)
  double rec_comp_intra_sm_read_rep_ratio = m_rec_comp_intra_sm_read_rep_group_size->sum()/m_comp_data_ratio->get_sample_no()*100;
  double rec_comp_inter_sm_read_rep_ratio = m_rec_comp_inter_sm_read_rep_group_size->sum()/m_comp_data_ratio->get_sample_no()*100;

  os << "m_comp_data_ratio_sample_no = " << m_comp_data_ratio->get_sample_no() << std::endl;

  os << "gpu_tot_avg_rec_comp_intra_sm_read_rep_ratio = ";
  os << rec_comp_intra_sm_read_rep_ratio << " \%" << std::endl;

  //if(total_comp_data_no > 0)	os << rec_comp_intra_sm_read_rep_ratio << " \%" << std::endl;
  //else				os << "0 \%" << std::endl;
  os << "gpu_tot_avg_rec_comp_intra_sm_read_rep_group_size = ";
  os << m_rec_comp_intra_sm_read_rep_group_size->avg() << std::endl;

  //added by kh(071516)
  os << "gpu_tot_avg_rec_comp_intra_sm_same_data_merge_no = ";
  os << m_rec_comp_intra_sm_same_data_merge_no->sum();
  os << std::endl;

  os << "gpu_tot_avg_rec_comp_intra_sm_same_data_merge_ratio = ";
  os << m_rec_comp_intra_sm_same_data_merge_no->sum()/m_rec_comp_intra_sm_read_rep_group_size->sum()*100 << " \%";
  os << std::endl;
  ///

  os << "gpu_tot_avg_rec_comp_inter_sm_read_rep_ratio = ";
  os << rec_comp_inter_sm_read_rep_ratio << " \%" << std::endl;
  //if(total_comp_data_no > 0)	os << rec_comp_inter_sm_read_rep_ratio << " \%" << std::endl;
  //else				os << "0 \%" << std::endl;
  os << "gpu_tot_avg_rec_comp_inter_sm_read_rep_group_size = ";
  os << m_rec_comp_inter_sm_read_rep_group_size->avg() << std::endl;
  ///

  //added by kh(062516)
  os << "gpu_tot_avg_rec_comp_read_rep_ratio = ";
  if(total_comp_data_no > 0)	os << (rec_comp_intra_sm_read_rep_ratio+rec_comp_inter_sm_read_rep_ratio) << " \%" << std::endl;
  else				os << "0 \%" << std::endl;
  os << "gpu_tot_avg_rec_comp_read_rep_group_size = ";
  double avg_rec_comp_group_size = m_rec_comp_intra_sm_read_rep_group_size->sum()+m_rec_comp_inter_sm_read_rep_group_size->sum();
  avg_rec_comp_group_size = avg_rec_comp_group_size/(m_rec_comp_intra_sm_read_rep_group_size->get_sample_no()+m_rec_comp_inter_sm_read_rep_group_size->get_sample_no());
  os << avg_rec_comp_group_size << std::endl;
  ///

  os << "gpu_tot_rec_comp_last_mf_not_found_ratio = ";
  os << m_rec_comp_last_mf_not_found_cnt->sum()/total_comp_data_no*100 << " \%" << std::endl;

  os << "gpu_tot_rec_comp_last_mf_comp_res_mistmatch_ratio = ";
  os << m_rec_comp_last_mf_comp_res_mistmatch_cnt->sum()/total_comp_data_no*100 << " \%" << std::endl;

  os << "gpu_tot_rec_comp_last_mf_full_cnt_ratio = ";
  os << m_rec_comp_last_mf_full_cnt->sum()/total_comp_data_no*100 << " \%" << std::endl;



  //added by kh(070416)
  double tot_dsc_comp_data_no = m_dsc_comp_0B_no->sum() + m_dsc_comp_2B_no->sum() + m_dsc_comp_4B_no->sum() + m_dsc_comp_8B_no->sum()+ m_dsc_comp_16B_no->sum() + m_dsc_comp_32B_no->sum() + m_dsc_comp_64B_no->sum() + m_dsc_comp_128B_no->sum();
  os << "gpu_tot_dsc_comp_0B_ratio = ";
  os << m_dsc_comp_0B_no->sum()/tot_dsc_comp_data_no*100 << " \%" << std::endl;
  os << "gpu_tot_dsc_comp_2B_ratio = ";
  os << m_dsc_comp_2B_no->sum()/tot_dsc_comp_data_no*100 << " \%" << std::endl;
  os << "gpu_tot_dsc_comp_4B_ratio = ";
  os << m_dsc_comp_4B_no->sum()/tot_dsc_comp_data_no*100 << " \%" << std::endl;
  os << "gpu_tot_dsc_comp_8B_ratio = ";
  os << m_dsc_comp_8B_no->sum()/tot_dsc_comp_data_no*100 << " \%" << std::endl;
  os << "gpu_tot_dsc_comp_16B_ratio = ";
  os << m_dsc_comp_16B_no->sum()/tot_dsc_comp_data_no*100 << " \%" << std::endl;
  os << "gpu_tot_dsc_comp_32B_ratio = ";
  os << m_dsc_comp_32B_no->sum()/tot_dsc_comp_data_no*100 << " \%" << std::endl;
  os << "gpu_tot_dsc_comp_64B_ratio = ";
  os << m_dsc_comp_64B_no->sum()/tot_dsc_comp_data_no*100 << " \%" << std::endl;
  os << "gpu_tot_dsc_comp_128B_ratio = ";
  os << m_dsc_comp_128B_no->sum()/tot_dsc_comp_data_no*100 << " \%" << std::endl;

  double tot_dsc_comp_data_type_no = m_data_type0_no->sum() + m_data_type1_no->sum() + m_data_type2_no->sum() + m_data_type3_no->sum() + m_data_type4_no->sum()  + m_data_type5_no->sum()  + m_data_type6_no->sum();
  //added by kh(021817)
  tot_dsc_comp_data_type_no += m_data_type7_no->sum();
  ///

  os << "gpu_tot_dsc_comp_data_type0_ratio = ";
  os << m_data_type0_no->sum() / tot_dsc_comp_data_type_no * 100 << std::endl;
  os << "gpu_tot_dsc_comp_data_type1_ratio = ";
  os << m_data_type1_no->sum() / tot_dsc_comp_data_type_no * 100 << std::endl;
  os << "gpu_tot_dsc_comp_data_type2_ratio = ";
  os << m_data_type2_no->sum() / tot_dsc_comp_data_type_no * 100 << std::endl;
  os << "gpu_tot_dsc_comp_data_type3_ratio = ";
  os << m_data_type3_no->sum() / tot_dsc_comp_data_type_no * 100 << std::endl;
  os << "gpu_tot_dsc_comp_data_type4_ratio = ";
  os << m_data_type4_no->sum() / tot_dsc_comp_data_type_no * 100 << std::endl;
  os << "gpu_tot_dsc_comp_data_type5_ratio = ";
  os << m_data_type5_no->sum() / tot_dsc_comp_data_type_no * 100 << std::endl;
  os << "gpu_tot_dsc_comp_data_type6_ratio = ";
  os << m_data_type6_no->sum() / tot_dsc_comp_data_type_no * 100 << std::endl;
  //added by kh(021817)
  os << "gpu_tot_dsc_comp_data_type7_ratio = ";
  os << m_data_type7_no->sum() / tot_dsc_comp_data_type_no * 100 << std::endl;
  ///

  //Best Data Type for LWM
  double tot_ext_lwm_data_no = m_ext_lwm_data_type0_no->sum() + m_ext_lwm_data_type1_no->sum() + m_ext_lwm_data_type2_no->sum() + m_ext_lwm_data_type3_no->sum() + m_ext_lwm_data_type4_no->sum() + m_ext_lwm_data_type5_no->sum() + m_ext_lwm_data_type6_no->sum();
  os << "gpu_tot_ext_lwm_comp_data_type0_ratio = ";
  os << m_ext_lwm_data_type0_no->sum() / tot_ext_lwm_data_no * 100 << std::endl;
  os << "gpu_tot_ext_lwm_comp_data_type1_ratio = ";
  os << m_ext_lwm_data_type1_no->sum() / tot_ext_lwm_data_no * 100 << std::endl;
  os << "gpu_tot_ext_lwm_comp_data_type2_ratio = ";
  os << m_ext_lwm_data_type2_no->sum() / tot_ext_lwm_data_no * 100 << std::endl;
  os << "gpu_tot_ext_lwm_comp_data_type3_ratio = ";
  os << m_ext_lwm_data_type3_no->sum() / tot_ext_lwm_data_no * 100 << std::endl;
  os << "gpu_tot_ext_lwm_comp_data_type4_ratio = ";
  os << m_ext_lwm_data_type4_no->sum() / tot_ext_lwm_data_no * 100 << std::endl;
  os << "gpu_tot_ext_lwm_comp_data_type5_ratio = ";
  os << m_ext_lwm_data_type5_no->sum() / tot_ext_lwm_data_no * 100 << std::endl;
  os << "gpu_tot_ext_lwm_comp_data_type6_ratio = ";
  os << m_ext_lwm_data_type6_no->sum() / tot_ext_lwm_data_no * 100 << std::endl;
  ///

  //added by kh(070716)
  os << "gpu_tot_read_req_no = ";
  os << m_read_req_no->sum() << std::endl;
  ///

  //added by kh(071216)
  double tot_comp_data_no = m_dsm_comp_no->sum() + m_lwm_comp_no->sum() + m_no_comp_no->sum();
  os << "gpu_tot_dsm_comp_util = ";
  os << m_dsm_comp_no->sum() / tot_comp_data_no * 100 << " \%" << std::endl;
  os << "gpu_tot_lwm_comp_util = ";
  os << m_lwm_comp_no->sum() / tot_comp_data_no * 100 << " \%" << std::endl;
  os << "gpu_tot_no_comp_util = ";
  os << m_no_comp_no->sum() / tot_comp_data_no * 100 << " \%" << std::endl;
  ///

  //For DSM-LWM Hybrid
  os << "gpu_tot_dsm_comp_data_ratio = ";
  os << m_dsm_comp_data_ratio->avg() * 100 << " \%" << std::endl;
  os << "gpu_tot_lwm_comp_data_ratio = ";
  os << m_lwm_comp_data_ratio->avg() * 100 << " \%" << std::endl;
  ///

  //added by kh(072416)
  os << "gpu_tot_dsm_comp_remap_op_ratio = " << m_dsm_comp_remap_op_case_no->sum()/m_dsm_comp_no->sum()*100 << " \%" << std::endl;
  double tot_used_op_no = m_dsm_comp_remap_mask_op_no->sum()
      + m_dsm_comp_remap_comp_op_no->sum()
      + m_dsm_comp_remap_neighbor_const_delta_op_no->sum()
      + m_dsm_comp_remap_delta_upto_1_op_no->sum() + m_dsm_comp_remap_delta_upto_3_op_no->sum()
      + m_dsm_comp_remap_comp_delta_upto_1_op_no->sum() + m_dsm_comp_remap_comp_delta_upto_3_op_no->sum();

  os << "gpu_tot_dsm_comp_remap_mask_op_ratio = " << m_dsm_comp_remap_mask_op_no->sum()/tot_used_op_no*100 << " \%" << std::endl;
  os << "gpu_tot_dsm_comp_remap_comp_op_ratio = " << m_dsm_comp_remap_comp_op_no->sum()/tot_used_op_no*100 << " \%" << std::endl;
  os << "gpu_tot_dsm_comp_remap_neighbor_const_delta_op_ratio = " << m_dsm_comp_remap_neighbor_const_delta_op_no->sum()/tot_used_op_no*100 << " \%" << std::endl;
  os << "gpu_tot_dsm_comp_remap_delta_upto_1_op_ratio = " << m_dsm_comp_remap_delta_upto_1_op_no->sum()/tot_used_op_no*100 << " \%" << std::endl;
  os << "gpu_tot_dsm_comp_remap_delta_upto_3_op_ratio = " << m_dsm_comp_remap_delta_upto_3_op_no->sum()/tot_used_op_no*100 << " \%" << std::endl;
  os << "gpu_tot_dsm_comp_remap_comp_delta_upto_1_op_ratio = " << m_dsm_comp_remap_comp_delta_upto_1_op_no->sum()/tot_used_op_no*100 << " \%" << std::endl;
  os << "gpu_tot_dsm_comp_remap_comp_delta_upto_3_op_ratio = " << m_dsm_comp_remap_comp_delta_upto_3_op_no->sum()/tot_used_op_no*100 << " \%" << std::endl;
  ///


  //added by kh(072616)
  os << "gpu_tot_comp_packet_no = " << m_comp_packet_no->sum() << std::endl;
  os << "gpu_tot_comp_packet_no_ratio = " << m_comp_packet_no->sum()/m_comp_data_ratio->get_sample_no()*100 << " \%" << std::endl;
  ///

  //added by kh(072816)
  os << "gpu_tot_approx_packet_no = " << m_approx_data_no->sum() << std::endl;
  os << "gpu_tot_approx_packet_no_ratio = " << m_approx_data_no->sum()/m_comp_data_ratio->get_sample_no()*100 << " \%" << std::endl;
  os << "gpu_tot_approx_exception_no = " << m_approx_exception_no->sum() << std::endl;
  ///

  //added by kh(030117)
  os << "gpu_tot_cache_comp_ratio = " << m_read_reply_cache_comp_ratio_across_kernels->avg() << std::endl;
  ///

  os << "===============================" << std::endl;
}

void hpcl_comp_anal::update_overall_stat()
{
  m_ovr_avg_comp_table_hit_rate->add_sample(m_comp_table_hit_no->sum()/(m_comp_table_hit_no->sum()+m_comp_table_miss_no->sum())*100);
  /*
  //added by kh(050516)
  if(m_read_reply_comp_ratio->get_sample_no() == 0)
    m_ovr_avg_read_reply_comp_ratio->add_sample(0);
  else
    m_ovr_avg_read_reply_comp_ratio->add_sample(m_read_reply_comp_ratio->avg());
  */
  ///
  //std::cout << "##### m_read_reply_comp_ratio->avg() " << m_read_reply_comp_ratio->avg() << std::endl;

  /*
  //added by kh(041616)
  m_ovr_avg_fragment_rate_per_flit->add_sample(m_fragment_rate_per_flit->avg());
  m_ovr_avg_fragment_rate_per_packet->add_sample(m_fragment_rate_per_packet->avg());
  m_ovr_avg_comp_overhead_rate->add_sample(m_comp_overhead_no->sum()/m_read_reply_comp_ratio->get_sample_no()*100);
  ///
  */

  //added by kh(041816)
  m_ovr_avg_org_flit_no->add_sample(m_org_flit_no->avg());
  m_ovr_avg_comp_flit_no->add_sample(m_comp_flit_no->avg());

  //added by kh(042116)
  m_ovr_avg_word2B_zeros_rep_rate->add_sample(m_word2B_zeros_rep_rate->avg());
  m_ovr_avg_word2B_nonzeros_rep_rate->add_sample(m_word2B_nonzeros_rep_rate->avg());
  m_ovr_avg_word2B_uni_val_rate->add_sample(m_word2B_uni_val_rate->avg());
  m_ovr_avg_word4B_zeros_rep_rate->add_sample(m_word4B_zeros_rep_rate->avg());
  m_ovr_avg_word4B_nonzeros_rep_rate->add_sample(m_word4B_nonzeros_rep_rate->avg());
  m_ovr_avg_word4B_uni_val_rate->add_sample(m_word4B_uni_val_rate->avg());
  m_ovr_avg_word8B_zeros_rep_rate->add_sample(m_word8B_zeros_rep_rate->avg());
  m_ovr_avg_word8B_nonzeros_rep_rate->add_sample(m_word8B_nonzeros_rep_rate->avg());
  m_ovr_avg_word8B_uni_val_rate->add_sample(m_word8B_uni_val_rate->avg());

/*
  double ovr_avg_single_rep_rate = 0;
  double ovr_avg_multi_rep_rate = 0;
  double ovr_avg_no_rep_rate = 0;
  double cnt = 0;
  for(unsigned i = 0; i < m_single_rep_to_same_sm_rate.size(); i++) {
    if(m_single_rep_to_same_sm_rate[i]->get_sample_no() > 0) {	//to avoid nan
	ovr_avg_single_rep_rate += m_single_rep_to_same_sm_rate[i]->avg();
	ovr_avg_multi_rep_rate += m_multi_rep_to_same_sm_rate[i]->avg();
	ovr_avg_no_rep_rate += m_no_rep_to_same_sm_rate[i]->avg();
	cnt++;
    }
  }
  ovr_avg_single_rep_rate = ovr_avg_single_rep_rate / cnt;
  ovr_avg_multi_rep_rate = ovr_avg_multi_rep_rate / cnt;
  ovr_avg_no_rep_rate = ovr_avg_no_rep_rate / cnt;
  m_ovr_avg_single_rep_to_same_sm_rate->add_sample(ovr_avg_single_rep_rate);
  m_ovr_avg_multi_rep_to_same_sm_rate->add_sample(ovr_avg_multi_rep_rate);
  m_ovr_avg_no_rep_to_same_sm_rate->add_sample(ovr_avg_no_rep_rate);
  m_ovr_avg_multi_rep_to_same_sm_no->add_sample(m_multi_rep_to_same_sm_no->avg());
*/
  //std::cout << "ovr_avg_single_rep_rate " << ovr_avg_single_rep_rate << std::endl;
  //std::cout << "ovr_avg_multi_rep_rate " << ovr_avg_multi_rep_rate << std::endl;
  //std::cout << "ovr_avg_no_rep_rate " << ovr_avg_no_rep_rate << std::endl;
  ///

  //added by kh(042216)
//  m_ovr_avg_word1B_zeros_rep_rate_in_uncomp_data->add_sample(m_word1B_zeros_rep_rate_in_uncomp_data->avg());
//  m_ovr_avg_word1B_nonzeros_rep_rate_in_uncomp_data->add_sample(m_word1B_nonzeros_rep_rate_in_uncomp_data->avg());
//  m_ovr_avg_word1B_uni_val_rate_in_uncomp_data->add_sample(m_word1B_uni_val_rate_in_uncomp_data->avg());
//  m_ovr_avg_word1B_nonzeros_rep_no_in_uncomp_data->add_sample(m_word1B_nonzeros_rep_no_in_uncomp_data->avg());
  ///

  //added by kh(042316)
//  m_ovr_avg_word1B_word1B_redund_rate_inter_read_replies->add_sample(m_word1B_word1B_redund_rate_inter_read_replies->avg());
  ///

  //added by kh(051916)
  /*
  double avg_inter_comp_packet_rate = m_inter_comp_packet_no->sum() / (m_intra_comp_packet_no->sum()+m_inter_comp_packet_no->sum()) *100;
  m_ovr_avg_inter_comp_packet_rate->add_sample(avg_inter_comp_packet_rate);
  m_ovr_avg_inter_comp_matching_word_no->add_sample(m_inter_comp_matching_word_no->avg());
  ///
  */
  //std::cout << "intra_comp_packet_no = " << m_intra_comp_packet_no->sum() << std::endl;
  //std::cout << "inter_comp_packet_no = " << m_inter_comp_packet_no->sum() << std::endl;
  //std::cout << "avg_inter_comp_packet_rate = " << avg_inter_comp_packet_rate << std::endl;
}

void hpcl_comp_anal::clear()
{
  m_comp_table_hit_no->clear();
  m_comp_table_miss_no->clear();

  /*
  m_read_reply_comp_ratio->clear();
  m_fragment_rate_per_flit->clear();
  m_fragment_rate_per_packet->clear();
  m_comp_overhead_no->clear();
  ///
  */

  //added by kh(041816)
  m_org_flit_no->clear();
  m_comp_flit_no->clear();
  //deleted by kh(060216)
  //to get stat across all kernels
  /*
  m_comp_2B_no->clear();
  m_comp_4B_no->clear();
  m_comp_8B_no->clear();
  */
  ///

  //added by kh(042116)
  m_word2B_zeros_rep_rate->clear();
  m_word2B_nonzeros_rep_rate->clear();
  m_word2B_uni_val_rate->clear();
  m_word4B_zeros_rep_rate->clear();
  m_word4B_nonzeros_rep_rate->clear();
  m_word4B_uni_val_rate->clear();
  m_word8B_zeros_rep_rate->clear();
  m_word8B_nonzeros_rep_rate->clear();
  m_word8B_uni_val_rate->clear();

  /*
  for(unsigned i = 0; i < m_single_rep_to_same_sm_rate.size(); i++) {
    m_single_rep_to_same_sm_rate[i]->clear();
    m_multi_rep_to_same_sm_rate[i]->clear();
    m_no_rep_to_same_sm_rate[i]->clear();
  }
  m_multi_rep_to_same_sm_no->clear();
  */
  ///

//  //added by kh(042216)
//  m_word1B_zeros_rep_rate_in_uncomp_data->clear();
//  m_word1B_nonzeros_rep_rate_in_uncomp_data->clear();
//  m_word1B_uni_val_rate_in_uncomp_data->clear();
//  m_word1B_nonzeros_rep_no_in_uncomp_data->clear();
//  ///
//
//  //added by kh(042316)
//  m_word1B_word1B_redund_rate_inter_read_replies->clear();
//  ///
/*
  //added by kh(051916)
  m_intra_comp_packet_no->clear();
  m_inter_comp_packet_no->clear();
  m_inter_comp_matching_word_no->clear();
  ///
*/

}
