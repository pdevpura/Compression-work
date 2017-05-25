/*
 * hpcl_comp_lwm_aux.h
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#ifndef HPCL_COMP_LWM_AUX_H_
#define HPCL_COMP_LWM_AUX_H_

//#include "hpcl_comp_lwm_pl_proc.h"
//#include "hpcl_comp_pl_data.h"

#include <vector>
#include <cassert>
#include "hpcl_dict.h"

//added by kh(061016)
#include "hpcl_comp_anal.h"
extern hpcl_comp_anal* g_hpcl_comp_anal;
///

//added by kh(061316)
extern unsigned long long  gpu_sim_cycle;
extern unsigned long long  gpu_tot_sim_cycle;
///

#include <cmath>
#include <map>

#include "hpcl_user_define_stmts.h"

//-hpcl_trans_mode should be 5 for the following test
//#define UNCOMPRESSED_2B_DATA_SEGMENT_ANALYSIS 1

#include "hpcl_comp_config.h"
extern hpcl_comp_config g_hpcl_comp_config;


class hpcl_comp_lwm_aux {

public:
  hpcl_comp_lwm_aux();
  ~hpcl_comp_lwm_aux();

public:
  void select_best_compressor(std::vector<unsigned>& comp_size, unsigned& min_comp_res, unsigned& min_comp_size);
  bool is_data_compressed(mem_fetch* mf, unsigned min_comp_res);

  //added by kh(061016)
  void get_stat_for_max_word_index(mem_fetch* mf);
  //void get_stat_for_comp_data_ratio_segment(mem_fetch* mf, unsigned min_comp_res);
  ///

  //added by kh(062316)
  void get_stat_for_compression_perf(mem_fetch* mf);

  void delete_local_dict_info(mem_fetch* mf);
  ///

  void clean_rec_comp_info(mem_fetch* mf);
  int compute_final_packet_size(mem_fetch* mf);

//private:
  //std::map<unsigned,int> mf_hist;

  //added by kh(063016)
  //void display_local_dict_info(mem_fetch* mf);

  //added by kh(070416)
  void sub_overlay_comp_data(mem_fetch* mf, enum mem_fetch::COMP_DATA_TYPE data_type, unsigned& min_lwm_bits, unsigned& min_lwm_res, bool debug=false);
  void overlay_comp_data(mem_fetch* mf, enum mem_fetch::COMP_DATA_TYPE data_type, unsigned& min_part_comp_bits, unsigned& min_comp_res, enum mem_fetch::COMP_DATA_TYPE& min_data_type);
  void get_LWM_comp_result(mem_fetch* mf, enum mem_fetch::COMP_DATA_TYPE data_type, unsigned& min_lwm_bits, unsigned& min_lwm_res, bool debug);
  ///

  //added by kh(071216)
  double get_redund_nibble_rate_for_LWM(mem_fetch* mf);
  double get_redund_nibble_rate_for_DSM(mem_fetch* mf);
  ///


};

hpcl_comp_lwm_aux::hpcl_comp_lwm_aux()	{

}

hpcl_comp_lwm_aux::~hpcl_comp_lwm_aux() {

}

void hpcl_comp_lwm_aux::select_best_compressor(std::vector<unsigned>& comp_size, unsigned& min_comp_res, unsigned& min_comp_size) {

  min_comp_res = 2;
  min_comp_size = comp_size[0];
  for(int i = 1; i < comp_size.size(); i++) {
    if(comp_size[i] < min_comp_size) {
      min_comp_size = comp_size[i];
      if(i == 1)	min_comp_res = 4;
      else if(i == 2)	min_comp_res = 8;
      else		assert(0);
    }
  }
}


bool hpcl_comp_lwm_aux::is_data_compressed(mem_fetch* mf, unsigned min_comp_res)
{
  bool ret = true;
  hpcl_dict<unsigned short>* loc_dict_2B = NULL;
  hpcl_dict<unsigned int>* loc_dict_4B = NULL;
  hpcl_dict<unsigned long long>* loc_dict_8B = NULL;
  if(min_comp_res == 2) {
    loc_dict_2B = (hpcl_dict<unsigned short>*) mf->get_loc_dict(min_comp_res);
    if(loc_dict_2B->get_word_no_with_multi_freq() == 0)	ret = false;
  } else if(min_comp_res == 4) {
    loc_dict_4B = (hpcl_dict<unsigned int>*) mf->get_loc_dict(min_comp_res);
    if(loc_dict_4B->get_word_no_with_multi_freq() == 0)	ret = false;
  } else if(min_comp_res == 8) {
    loc_dict_8B = (hpcl_dict<unsigned long long>*) mf->get_loc_dict(min_comp_res);
    if(loc_dict_8B->get_word_no_with_multi_freq() == 0)	ret = false;
  }

  return ret;
}


void hpcl_comp_lwm_aux::get_stat_for_max_word_index(mem_fetch* mf)
{
  hpcl_dict<unsigned short>* loc_dict_2B = (hpcl_dict<unsigned short>*) mf->get_loc_dict(2);
  hpcl_dict<unsigned int>* loc_dict_4B = (hpcl_dict<unsigned int>*) mf->get_loc_dict(4);
  hpcl_dict<unsigned long long>* loc_dict_8B = (hpcl_dict<unsigned long long>*) mf->get_loc_dict(8);
  hpcl_dict<unsigned short>* trans_loc_dict_2B = (hpcl_dict<unsigned short>*) mf->get_trans_loc_dict(2);
  hpcl_dict<unsigned int>* trans_loc_dict_4B = (hpcl_dict<unsigned int>*) mf->get_trans_loc_dict(4);
  hpcl_dict<unsigned long long>* trans_loc_dict_8B = (hpcl_dict<unsigned long long>*) mf->get_trans_loc_dict(8);

  int loc_dict_2B_max_word_index = loc_dict_2B->get_last_max_word_loc();
  int loc_dict_4B_max_word_index = loc_dict_4B->get_last_max_word_loc();
  int loc_dict_8B_max_word_index = loc_dict_8B->get_last_max_word_loc();
  int loc_dict_trans_2B_max_word_index = trans_loc_dict_2B->get_last_max_word_loc();
  int loc_dict_trans_4B_max_word_index = trans_loc_dict_4B->get_last_max_word_loc();
  int loc_dict_trans_8B_max_word_index = trans_loc_dict_8B->get_last_max_word_loc();

  /*
  assert(loc_dict_2B_max_word_index >= 0);
  assert(loc_dict_4B_max_word_index >= 0);
  assert(loc_dict_8B_max_word_index >= 0);
  assert(loc_dict_trans_2B_max_word_index >= 0);
  assert(loc_dict_trans_4B_max_word_index >= 0);
  assert(loc_dict_trans_8B_max_word_index >= 0);
   */

  if(loc_dict_2B_max_word_index >= 0)		g_hpcl_comp_anal->add_sample(hpcl_comp_anal::MAX_WORD_INDEX_2B, 1, loc_dict_2B_max_word_index);
  if(loc_dict_4B_max_word_index >= 0)		g_hpcl_comp_anal->add_sample(hpcl_comp_anal::MAX_WORD_INDEX_4B, 1, loc_dict_4B_max_word_index);
  if(loc_dict_8B_max_word_index >= 0)		g_hpcl_comp_anal->add_sample(hpcl_comp_anal::MAX_WORD_INDEX_8B, 1, loc_dict_8B_max_word_index);
  if(loc_dict_trans_2B_max_word_index >= 0)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::MAX_WORD_INDEX_TRANS_2B, 1, loc_dict_trans_2B_max_word_index);
  if(loc_dict_trans_4B_max_word_index >= 0)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::MAX_WORD_INDEX_TRANS_4B, 1, loc_dict_trans_4B_max_word_index);
  if(loc_dict_trans_8B_max_word_index >= 0)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::MAX_WORD_INDEX_TRANS_8B, 1, loc_dict_trans_8B_max_word_index);

}

#ifdef old
void hpcl_comp_lwm_aux::get_stat_for_comp_data_ratio_segment(mem_fetch* mf, unsigned min_comp_res)
{
  //unsigned data_size = mf->get_data_size();
  /*
  printf("Data(%03u) = ", mf->get_real_data_size());
  for(int j = mf->get_data_size()-1; j >= 0 ; j--) {
    printf("%02x ", mf->get_real_data(j));
  }
  printf("\n");
  */

  hpcl_dict<unsigned short>* loc_dict_2B = (hpcl_dict<unsigned short>*) mf->get_loc_dict(2);
  hpcl_dict<unsigned int>* loc_dict_4B = (hpcl_dict<unsigned int>*) mf->get_loc_dict(4);
  hpcl_dict<unsigned long long>* loc_dict_8B = (hpcl_dict<unsigned long long>*) mf->get_loc_dict(8);

  if(min_comp_res == 2) {
    double comp_2B_data_segment0_ratio = loc_dict_2B->get_comp_data_segment_ratio(0);
    double comp_2B_data_segment1_ratio = loc_dict_2B->get_comp_data_segment_ratio(1);
    double comp_2B_data_segment2_ratio = loc_dict_2B->get_comp_data_segment_ratio(2);
    double comp_2B_data_segment3_ratio = loc_dict_2B->get_comp_data_segment_ratio(3);
    /*
    assert(comp_2B_data_ratio_segment0 != -1);
    assert(comp_2B_data_ratio_segment1 != -1);
    assert(comp_2B_data_ratio_segment2 != -1);
    assert(comp_2B_data_ratio_segment3 != -1);
    */

    if(comp_2B_data_segment0_ratio != -1)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_DATA_RATIO_SEGMENT_2B, comp_2B_data_segment0_ratio, 0);
    if(comp_2B_data_segment1_ratio != -1)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_DATA_RATIO_SEGMENT_2B, comp_2B_data_segment1_ratio, 1);
    if(comp_2B_data_segment2_ratio != -1)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_DATA_RATIO_SEGMENT_2B, comp_2B_data_segment2_ratio, 2);
    if(comp_2B_data_segment3_ratio != -1)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_DATA_RATIO_SEGMENT_2B, comp_2B_data_segment3_ratio, 3);

    int no_comp_2B_data_segment0_ratio = loc_dict_2B->get_no_comp_data_segment_ratio(0);
    int no_comp_2B_data_segment1_ratio = loc_dict_2B->get_no_comp_data_segment_ratio(1);
    int no_comp_2B_data_segment2_ratio = loc_dict_2B->get_no_comp_data_segment_ratio(2);
    int no_comp_2B_data_segment3_ratio = loc_dict_2B->get_no_comp_data_segment_ratio(3);

    if(no_comp_2B_data_segment0_ratio != -1)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::NO_COMP_DATA_NUM_SEGMENT_2B, no_comp_2B_data_segment0_ratio, 0);
    if(no_comp_2B_data_segment1_ratio != -1)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::NO_COMP_DATA_NUM_SEGMENT_2B, no_comp_2B_data_segment1_ratio, 1);
    if(no_comp_2B_data_segment2_ratio != -1)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::NO_COMP_DATA_NUM_SEGMENT_2B, no_comp_2B_data_segment2_ratio, 2);
    if(no_comp_2B_data_segment3_ratio != -1)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::NO_COMP_DATA_NUM_SEGMENT_2B, no_comp_2B_data_segment3_ratio, 3);


    #ifdef UNCOMPRESSED_2B_DATA_SEGMENT_ANALYSIS
    static unsigned uncomp_data_count = 30;
    bool found = false;
    if(no_comp_2B_data_segment0_ratio == 1
    || no_comp_2B_data_segment1_ratio == 1
    || no_comp_2B_data_segment2_ratio == 1
    || no_comp_2B_data_segment3_ratio == 1) {
      found = true;
    }
    if(found == true) {
      printf("mf %u (SM %u) Mem_Addr: %016llx Size: %u has uncompressed data segment at %llu\n",
	     mf->get_request_uid(), mf->get_tpc(), mf->get_addr(), mf->get_data_size(), (gpu_tot_sim_cycle+gpu_sim_cycle));

  //if(no_comp_2B_data_segment0_ratio == 1) {
      printf("\tTransSeg0 (No Comp %d): ", no_comp_2B_data_segment0_ratio);
      for(int j = 0; j < 32 && 32 <= mf->get_data_size() ; j++)	printf("%02x ", mf->get_real_data(j));
      printf("\n");
  //}

  //if(no_comp_2B_data_segment1_ratio == 1) {
      printf("\tTransSeg1 (No Comp %d): ", no_comp_2B_data_segment1_ratio);
      for(int j = 32; j < 64 && 64 <= mf->get_data_size(); j++)	printf("%02x ", mf->get_real_data(j));
      printf("\n");
  //}

  //if(no_comp_2B_data_segment2_ratio == 1) {
      printf("\tTransSeg2 (No Comp %d): ", no_comp_2B_data_segment2_ratio);
      for(int j = 64; j < 96 && 96 <= mf->get_data_size(); j++)	printf("%02x ", mf->get_real_data(j));
      printf("\n");
  //}

  //if(no_comp_2B_data_segment3_ratio == 1) {
      printf("\tTransSeg3 (No Comp %d): ", no_comp_2B_data_segment3_ratio);
      for(int j = 96; j < 128 && 128 <= mf->get_data_size(); j++)	printf("%02x ", mf->get_real_data(j));
      printf("\n");
  //}

      printf("\tOriginal Data:\n");

      printf("\tSeg0: ");
      for(int j = 0; j < 32 && 32 <= mf->get_data_size(); j++)	printf("%02x ", mf->get_trans_data(j));
      printf("\n");
      printf("\tSeg1: ");
      for(int j = 32; j < 64 && 64 <= mf->get_data_size(); j++)	printf("%02x ", mf->get_trans_data(j));
      printf("\n");
      printf("\tSeg2: ");
      for(int j = 64; j < 96 && 96 <= mf->get_data_size(); j++)	printf("%02x ", mf->get_trans_data(j));
      printf("\n");
      printf("\tSeg3: ");
      for(int j = 96; j < 128 && 128 <= mf->get_data_size(); j++)	printf("%02x ", mf->get_trans_data(j));
      printf("\n");

      uncomp_data_count--;
      if(uncomp_data_count == 0) {
	assert(0);
      }
    }

    #endif






    /*
    printf("comp_2B_data_ratio: seg0 %3.2f, seg1 %3.2f, seg2 %3.2f, seg3 %3.2f\n",
	   comp_2B_data_segment0_ratio, comp_2B_data_segment1_ratio, comp_2B_data_segment2_ratio, comp_2B_data_segment3_ratio);

    printf("no_comp_2B_data_segment_ratio: seg0 %d, seg1 %d, seg2 %d, seg3 %d\n",
	   no_comp_2B_data_segment0_ratio, no_comp_2B_data_segment1_ratio, no_comp_2B_data_segment2_ratio, no_comp_2B_data_segment3_ratio);
    */

  }

  if(min_comp_res == 4) {
    double comp_4B_data_segment0_ratio = loc_dict_4B->get_comp_data_segment_ratio(0);
    double comp_4B_data_segment1_ratio = loc_dict_4B->get_comp_data_segment_ratio(1);
    double comp_4B_data_segment2_ratio = loc_dict_4B->get_comp_data_segment_ratio(2);
    double comp_4B_data_segment3_ratio = loc_dict_4B->get_comp_data_segment_ratio(3);

    /*
    assert(comp_4B_data_segment0_ratio != -1);
    assert(comp_4B_data_segment1_ratio != -1);
    assert(comp_4B_data_segment2_ratio != -1);
    assert(comp_4B_data_segment3_ratio != -1);
    */

    if(comp_4B_data_segment0_ratio != -1) 	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_DATA_RATIO_SEGMENT_4B, comp_4B_data_segment0_ratio, 0);
    if(comp_4B_data_segment1_ratio != -1)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_DATA_RATIO_SEGMENT_4B, comp_4B_data_segment1_ratio, 1);
    if(comp_4B_data_segment2_ratio != -1)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_DATA_RATIO_SEGMENT_4B, comp_4B_data_segment2_ratio, 2);
    if(comp_4B_data_segment3_ratio != -1)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_DATA_RATIO_SEGMENT_4B, comp_4B_data_segment3_ratio, 3);

    int no_comp_4B_data_segment0_ratio = loc_dict_4B->get_no_comp_data_segment_ratio(0);
    int no_comp_4B_data_segment1_ratio = loc_dict_4B->get_no_comp_data_segment_ratio(1);
    int no_comp_4B_data_segment2_ratio = loc_dict_4B->get_no_comp_data_segment_ratio(2);
    int no_comp_4B_data_segment3_ratio = loc_dict_4B->get_no_comp_data_segment_ratio(3);

    if(no_comp_4B_data_segment0_ratio != -1)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::NO_COMP_DATA_NUM_SEGMENT_4B, no_comp_4B_data_segment0_ratio, 0);
    if(no_comp_4B_data_segment1_ratio != -1)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::NO_COMP_DATA_NUM_SEGMENT_4B, no_comp_4B_data_segment1_ratio, 1);
    if(no_comp_4B_data_segment2_ratio != -1)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::NO_COMP_DATA_NUM_SEGMENT_4B, no_comp_4B_data_segment2_ratio, 2);
    if(no_comp_4B_data_segment3_ratio != -1)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::NO_COMP_DATA_NUM_SEGMENT_4B, no_comp_4B_data_segment3_ratio, 3);

    /*
    printf("comp_4B_data_ratio: seg0 %3.2f, seg1 %3.2f, seg2 %3.2f, seg3 %3.2f\n",
	   comp_4B_data_segment0_ratio, comp_4B_data_segment1_ratio, comp_4B_data_segment2_ratio, comp_4B_data_segment3_ratio);

    printf("no_comp_4B_data_segment_ratio: seg0 %d, seg1 %d, seg2 %d, seg3 %d\n",
	   no_comp_4B_data_segment0_ratio, no_comp_4B_data_segment1_ratio, no_comp_4B_data_segment2_ratio, no_comp_4B_data_segment3_ratio);
    */

  }

  if(min_comp_res == 8) {
    double comp_8B_data_segment0_ratio = loc_dict_8B->get_comp_data_segment_ratio(0);
    double comp_8B_data_segment1_ratio = loc_dict_8B->get_comp_data_segment_ratio(1);
    double comp_8B_data_segment2_ratio = loc_dict_8B->get_comp_data_segment_ratio(2);
    double comp_8B_data_segment3_ratio = loc_dict_8B->get_comp_data_segment_ratio(3);

    /*
    assert(comp_8B_data_ratio_segment0 != -1);
    assert(comp_8B_data_ratio_segment1 != -1);
    assert(comp_8B_data_ratio_segment2 != -1);
    assert(comp_8B_data_ratio_segment3 != -1);
    */

    if(comp_8B_data_segment0_ratio != -1)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_DATA_RATIO_SEGMENT_8B, comp_8B_data_segment0_ratio, 0);
    if(comp_8B_data_segment1_ratio != -1)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_DATA_RATIO_SEGMENT_8B, comp_8B_data_segment1_ratio, 1);
    if(comp_8B_data_segment2_ratio != -1)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_DATA_RATIO_SEGMENT_8B, comp_8B_data_segment2_ratio, 2);
    if(comp_8B_data_segment3_ratio != -1)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_DATA_RATIO_SEGMENT_8B, comp_8B_data_segment3_ratio, 3);

    int no_comp_8B_data_segment0_ratio = loc_dict_8B->get_no_comp_data_segment_ratio(0);
    int no_comp_8B_data_segment1_ratio = loc_dict_8B->get_no_comp_data_segment_ratio(1);
    int no_comp_8B_data_segment2_ratio = loc_dict_8B->get_no_comp_data_segment_ratio(2);
    int no_comp_8B_data_segment3_ratio = loc_dict_8B->get_no_comp_data_segment_ratio(3);

    if(no_comp_8B_data_segment0_ratio != -1)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::NO_COMP_DATA_NUM_SEGMENT_8B, no_comp_8B_data_segment0_ratio, 0);
    if(no_comp_8B_data_segment1_ratio != -1)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::NO_COMP_DATA_NUM_SEGMENT_8B, no_comp_8B_data_segment1_ratio, 1);
    if(no_comp_8B_data_segment2_ratio != -1)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::NO_COMP_DATA_NUM_SEGMENT_8B, no_comp_8B_data_segment2_ratio, 2);
    if(no_comp_8B_data_segment3_ratio != -1)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::NO_COMP_DATA_NUM_SEGMENT_8B, no_comp_8B_data_segment3_ratio, 3);

    /*
    printf("comp_8B_data_ratio: seg0 %3.2f, seg1 %3.2f, seg2 %3.2f, seg3 %3.2f\n",
	   comp_8B_data_segment0_ratio, comp_8B_data_segment1_ratio, comp_8B_data_segment2_ratio, comp_8B_data_segment3_ratio);

    printf("no_comp_8B_data_segment_ratio: seg0 %d, seg1 %d, seg2 %d, seg3 %d\n",
	   no_comp_8B_data_segment0_ratio, no_comp_8B_data_segment1_ratio, no_comp_8B_data_segment2_ratio, no_comp_8B_data_segment3_ratio);
    */
  }
}
#endif

void hpcl_comp_lwm_aux::get_stat_for_compression_perf(mem_fetch* mf)
{
  if(!mf)				return;
  if(mf->get_type() != READ_REPLY)	return;
  if(mf->get_real_data_size() == 0) 	return;

/*
  if(mf->get_real_data_size() == 0) {
    std::cout << "mf " << mf->get_request_uid() << " " << mf->get_access_type() << std::endl;
  }
  assert(mf->get_real_data_size() > 0);
  if(mf->get_access_type() == INST_ACC_R) {
    printf("mf->get_real_data_size() = %u\n", mf->get_real_data_size());
    mf->print_data(mem_fetch::ORG_DATA);
    mf->print_data(mem_fetch::REMAPPED_DATA_1);
    mf->print_data(mem_fetch::REMAPPED_DATA_2);
  }
  assert(mf->get_access_type() != INST_ACC_R);
*/

  //add statistics
  //added by kh(062316)
  int response_size = mf->get_final_packet_size();
  assert(response_size > 0);
  ///

#ifdef old
  unsigned org_response_size = mf->size();
  unsigned _flit_size = 32;
  double n_org_flits = org_response_size / _flit_size + ((org_response_size % _flit_size)? 1:0);


  double comp_data_bits1 = (mf->get_rec_comp_data_bits() > 0) ? mf->get_rec_comp_data_bits() : mf->get_comp_data_bits();
  double comp_packet_size = ceil(comp_data_bits1/8.0) + mf->get_ctrl_size();
  double n_new_flits = comp_packet_size / _flit_size + (((unsigned)comp_packet_size % _flit_size)? 1:0);
  //REC_ANAL_COMP_DEBUG_PRINT("mf %u | merging | comp_ratio %3.2f \%\n", mf->get_request_uid(), comp_data_ratio*100);
  g_hpcl_comp_anal->add_sample(hpcl_comp_anal::PACKET_COMP_RATIO, (n_org_flits-n_new_flits)/n_org_flits);
  printf("mf %u | comp_packet_size %f | n_org_flits %f | n_new_flits %f | packet_comp_ratio %f\n", mf->get_request_uid(), comp_packet_size, n_org_flits, n_new_flits, (n_org_flits-n_new_flits)/n_org_flits);

  //added by kh(072616)
  //calculate the number of flits of merged mfs
  if(mf->has_merged_mem_fetch() == true)	{
    std::vector<mem_fetch*>& merged_mfs = mf->get_merged_mfs();
    for(unsigned j = 0; j < merged_mfs.size(); j++) {
      mem_fetch* merged_mf = merged_mfs[j];
      unsigned org_merged_mf_response_size = merged_mf->size();
      unsigned n_org_merged_mf_flits = org_merged_mf_response_size / _flit_size + ((org_merged_mf_response_size % _flit_size)? 1:0);

      double comp_data_bits = (merged_mf->get_rec_comp_data_bits() > 0) ? merged_mf->get_rec_comp_data_bits() : merged_mf->get_comp_data_bits();
      double comp_packet_size = ceil(comp_data_bits/8.0) + merged_mf->get_ctrl_size()/2;
      double n_new_merged_mf_flits = comp_packet_size / _flit_size + (((unsigned)comp_packet_size % _flit_size)? 1:0);

      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::PACKET_COMP_RATIO, (n_org_merged_mf_flits-n_new_merged_mf_flits)/n_org_merged_mf_flits);
      printf("inter_sm merged mf %u | comp_packet_size %f | n_org_flits %f | n_new_flits %f | packet_comp_ratio %f\n",
	     merged_mf->get_request_uid(), comp_packet_size, n_org_merged_mf_flits, n_new_merged_mf_flits,
	     (n_org_merged_mf_flits-n_new_merged_mf_flits)/n_org_merged_mf_flits);

    }
  }
  ///

  //added by kh(072616)
  if(mf->get_merged_mf_to_same_SM_no() > 0) {
    for(unsigned i = 0; i < mf->get_merged_mf_to_same_SM_no(); i++) {
      mem_fetch* merged_mf = mf->get_merged_mf_to_same_SM(i);
      unsigned org_merged_mf_response_size = merged_mf->size();
      unsigned n_org_merged_mf_flits = org_merged_mf_response_size / _flit_size + ((org_merged_mf_response_size % _flit_size)? 1:0);

      double comp_data_bits = (merged_mf->get_rec_comp_data_bits() > 0) ? merged_mf->get_rec_comp_data_bits() : merged_mf->get_comp_data_bits();
      double comp_packet_size = ceil(comp_data_bits/8.0) + merged_mf->get_ctrl_size()/2;
      double n_new_merged_mf_flits = comp_packet_size / _flit_size + (((unsigned)comp_packet_size % _flit_size)? 1:0);

      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::PACKET_COMP_RATIO, (n_org_merged_mf_flits-n_new_merged_mf_flits)/n_org_merged_mf_flits);

      printf("intra_sm merged mf %u | comp_packet_size %f | n_org_flits %f | n_new_flits %f | packet_comp_ratio %f\n",
	     merged_mf->get_request_uid(), comp_packet_size, n_org_merged_mf_flits, n_new_merged_mf_flits,
	     (n_org_merged_mf_flits-n_new_merged_mf_flits)/n_org_merged_mf_flits);

    }
  }
  ///
#endif

  unsigned org_response_size = mf->size();
  unsigned _flit_size = 32;
  double n_org_flits = org_response_size / _flit_size + ((org_response_size % _flit_size)? 1:0);

  //added by kh(042316)
  //calculate the number of flits of merged mfs
  if(mf->has_merged_mem_fetch() == true)	{
    std::vector<mem_fetch*>& merged_mfs = mf->get_merged_mfs();
    for(unsigned j = 0; j < merged_mfs.size(); j++) {
      unsigned org_merged_mf_response_size = merged_mfs[j]->size();
      unsigned n_org_merged_mf_flits = org_merged_mf_response_size / _flit_size + ((org_merged_mf_response_size % _flit_size)? 1:0);
      n_org_flits += n_org_merged_mf_flits;
    }
  }
  ///

  //added by kh(062916)
  if(mf->get_merged_mf_to_same_SM_no() > 0) {
    for(unsigned i = 0; i < mf->get_merged_mf_to_same_SM_no(); i++) {
      unsigned org_merged_mf_response_size = mf->get_merged_mf_to_same_SM(i)->size();
      unsigned n_org_merged_mf_flits = org_merged_mf_response_size / _flit_size + ((org_merged_mf_response_size % _flit_size)? 1:0);
      n_org_flits += n_org_merged_mf_flits;
    }
  }
  ///

  double n_comp_flits = response_size / _flit_size + ((response_size % _flit_size)? 1:0);
  double comp_ratio = (double)(n_org_flits-n_comp_flits) / n_org_flits;
  //if(mf->get_type() != CTRL_MSG) {
  g_hpcl_comp_anal->add_sample(hpcl_comp_anal::PACKET_COMP_RATIO, comp_ratio);


  //added by kh(030117)
  double org_data_byte1 = mf->get_data_size();
  double comp_data_byte1 = (mf->get_rec_comp_data_bits() > 0) ? mf->get_rec_comp_data_bits() : mf->get_comp_data_bits();
  comp_data_byte1 = ceil(comp_data_byte1/8.0);
  double org_data_segment_no = org_data_byte1/8.0;	//segment: 8B
  double comp_data_segment_no = comp_data_byte1/8.0;
  //printf("segment_comp_ratio = %f\n", (org_data_segment_no-comp_data_segment_no)/org_data_segment_no*100);
  double cache_comp_ratio = (org_data_segment_no-comp_data_segment_no)/org_data_segment_no*100;
  /*
  if(cache_comp_ratio < 0) {
	  printf("org_data_byte1 %f org_data_segment_no %f\n", org_data_byte1, org_data_segment_no);
	  printf("comp_data_byte1 %f comp_data_segment_no %f\n", comp_data_byte1, comp_data_segment_no);
  }
  assert(cache_comp_ratio > 0);
  g_hpcl_comp_anal->add_sample(hpcl_comp_anal::CACHE_COMP_RATIO, cache_comp_ratio);
  */
  ///

  //added by kh(022117)
  if(g_hpcl_comp_config.hpcl_char_preproc_en == 1) {

	  //added by kh (021817)
	  if( mf->get_chartype() == true ) {	//Char Data

		/*
		//static int counter = 0;
		printf("packet_comp_ratio = %f\n", comp_ratio*100);

		double org_data_byte = mf->get_data_size();
		double comp_data_byte = (mf->get_rec_comp_data_bits() > 0) ? mf->get_rec_comp_data_bits() : mf->get_comp_data_bits();
		comp_data_byte = ceil(comp_data_byte/8.0);
		double org_data_segment_no = org_data_byte/8.0;	//segment: 8B
		double comp_data_segment_no = comp_data_byte/8.0;
		printf("segment_comp_ratio = %f\n", (org_data_segment_no-comp_data_segment_no)/org_data_segment_no*100);
	
	
		printf("mf %u addr 0x%llx Char Data(%03u) = ", mf->get_request_uid(), mf->get_addr(), mf->get_real_data_size());
		//for(int i = mf->get_data_size()-1; i >= 0 ; i--) {
		for(int i = 0; i < mf->get_data_size(); i++) {
			printf("%02x", mf->get_real_data(i));
		}
		printf("\n");

		std::vector<unsigned char>& remapped_cache_data = mf->get_txt_trans_data_ptr();
		printf("RemappedChar Data(%03u) = ", remapped_cache_data.size());
		//for(int i = remapped_cache_data.size()-1; i >= 0 ; i--) {
		for(int i = 0; i < remapped_cache_data.size(); i++) {
			printf("%02x", remapped_cache_data[i]);
		}
		printf("\n");

//		counter++;
//		if(counter >= 50)	exit (EXIT_SUCCESS);
		*/

		/*
		if(comp_ratio < 0.1) {

			printf("comp_ratio = %f, char_type_pct = %f\n", comp_ratio*100, g_hpcl_comp->get_char_type_pct(mf));
			printf("Char Data(%03u) = ", mf->get_real_data_size());
			for(int i = mf->get_data_size()-1; i >= 0 ; i--) {
				printf("%02x", mf->get_real_data(i));
			}
			printf("\n");


			std::vector<unsigned char>& remapped_cache_data = mf->get_txt_trans_data_ptr();
			printf("RemappedChar Data(%03u) = ", remapped_cache_data.size());
			for(int i = remapped_cache_data.size()-1; i >= 0 ; i--) {
				printf("%02x", remapped_cache_data[i]);
			}
			printf("\n");

		}
		*/



	  } else {
		/*
	    static int counter = 0;

	    if(comp_ratio < 0.1) {

	    	printf("comp_ratio = %f, char_type_pct = %f\n", comp_ratio*100, g_hpcl_comp->get_char_type_pct(mf));
			printf("[%d] Char Data(%03u) = ", counter, mf->get_real_data_size());
			for(int i = mf->get_data_size()-1; i >= 0 ; i--) {
				printf("%02x", mf->get_real_data(i));
			}
			printf("\n");
			counter++;
			if(counter >= 100)	exit (EXIT_SUCCESS);
	    }
		*/
	  }
  }
  ///

  //added by kh(072616)
  //measure the number of compressed packet
  //to see the reduction of packet numbers after inter-packet compression
  g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_PACKET_NO, 1);



/*
  //added by kh(060216)
  //Measure compression ratio according to compression resolution
  if(g_hpcl_comp_config.hpcl_comp_en == 1) {
    unsigned min_comp_res = mf->get_comp_res();
    if(min_comp_res == 2)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::PACKET_COMP_2B_RATIO, comp_ratio);
    else if(min_comp_res == 4)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::PACKET_COMP_4B_RATIO, comp_ratio);
    else if(min_comp_res == 8)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::PACKET_COMP_8B_RATIO, comp_ratio);
  }
*/
  //added by kh(060316)
  //Measure compression ratio per reply without considering flit.
  double org_data_bits = mf->get_data_size()*8;
  double comp_data_bits = (mf->get_rec_comp_data_bits() > 0) ? mf->get_rec_comp_data_bits() : mf->get_comp_data_bits();
  //double comp_data_ratio = (org_data_bits-comp_data_bits)/org_data_bits;
  //added by kh(071516)
  double org_data_byte = mf->get_data_size();
  double comp_data_byte = ceil(comp_data_bits/8.0);
  double comp_data_ratio = (org_data_byte-comp_data_byte)/org_data_byte;



  REC_ANAL_COMP_DEBUG_PRINT("mf %u | merging | comp_ratio %3.2f \%\n", mf->get_request_uid(), comp_data_ratio*100);
  g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_DATA_RATIO, comp_data_ratio);

//  std::map<unsigned,int>::iterator it_test = mf_hist.find(mf->get_request_uid());
//  assert(it_test == mf_hist.end());
//  mf_hist[mf->get_request_uid()] = 0;

  for(unsigned j = 0; j < mf->get_merged_mf_no(); j++) {
    org_data_bits = mf->get_merged_mf(j)->get_data_size()*8;
    comp_data_bits = (mf->get_merged_mf(j)->get_rec_comp_data_bits() > 0) ? mf->get_merged_mf(j)->get_rec_comp_data_bits() : mf->get_merged_mf(j)->get_comp_data_bits();
    //comp_data_ratio = (org_data_bits-comp_data_bits)/org_data_bits;
    //added by kh(071516)
    org_data_byte = mf->get_merged_mf(j)->get_data_size()*8;
    comp_data_byte = ceil(comp_data_bits/8.0);
    comp_data_ratio = (org_data_byte-comp_data_byte)/org_data_byte;
    ///
    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_DATA_RATIO, comp_data_ratio);

    REC_ANAL_COMP_DEBUG_PRINT("mf %u | merged | comp_ratio %3.2f \%\n", mf->get_merged_mf(j)->get_request_uid(), comp_data_ratio*100);

//    std::map<unsigned,int>::iterator it_test = mf_hist.find(mf->get_merged_mf(j)->get_request_uid());
//    assert(it_test == mf_hist.end());
//    mf_hist[mf->get_merged_mf(j)->get_request_uid()] = 0;

  }
  for(unsigned j = 0; j < mf->get_merged_mf_to_same_SM_no(); j++) {
    org_data_bits = mf->get_merged_mf_to_same_SM(j)->get_data_size()*8;
    comp_data_bits = (mf->get_merged_mf_to_same_SM(j)->get_rec_comp_data_bits() > 0) ? mf->get_merged_mf_to_same_SM(j)->get_rec_comp_data_bits() : mf->get_merged_mf_to_same_SM(j)->get_comp_data_bits();
    //comp_data_ratio = (org_data_bits-comp_data_bits)/org_data_bits;

    //added by kh(071516)
    org_data_byte = mf->get_merged_mf_to_same_SM(j)->get_data_size();
    comp_data_byte = ceil(comp_data_bits/8.0);
    comp_data_ratio = (org_data_byte-comp_data_byte)/org_data_byte;
    ///

    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_DATA_RATIO, comp_data_ratio);

    REC_ANAL_COMP_DEBUG_PRINT("mf %u | merged | comp_ratio %3.2f \%\n", mf->get_merged_mf_to_same_SM(j)->get_request_uid(), comp_data_ratio*100);

//    std::map<unsigned,int>::iterator it_test = mf_hist.find(mf->get_merged_mf_to_same_SM(j)->get_request_uid());
//      assert(it_test == mf_hist.end());
//      mf_hist[mf->get_merged_mf_to_same_SM(j)->get_request_uid()] = 0;
  }
  ///

  //added by kh(070416)
  /*
  int dsc_comp_res = mf->get_dsc_comp_res();
  if(dsc_comp_res >= 0) {
    if(dsc_comp_res == 0) 	 g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DSC_COMP_0B_NO, 1);
    else if(dsc_comp_res == 2)   g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DSC_COMP_2B_NO, 1);
    else if(dsc_comp_res == 4)   g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DSC_COMP_4B_NO, 1);
    else if(dsc_comp_res == 8)   g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DSC_COMP_8B_NO, 1);
    else if(dsc_comp_res == 16)  g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DSC_COMP_16B_NO, 1);
    else if(dsc_comp_res == 32)  g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DSC_COMP_32B_NO, 1);
    else if(dsc_comp_res == 64)  g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DSC_COMP_64B_NO, 1);
    else if(dsc_comp_res == 128) g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DSC_COMP_128B_NO, 1);


    if(dsc_comp_res >= g_hpcl_comp_config.hpcl_dsc_min_ds_size && dsc_comp_res <= 128) {
      mem_fetch::COMP_DATA_TYPE type = mf->get_dsc_comp_data_type();
      if(type == mem_fetch::ORG_DATA)			g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DATA_TYPE0_NO, 1);
      else if(type == mem_fetch::REMAPPED_DATA_1)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DATA_TYPE1_NO, 1);
      else if(type == mem_fetch::REMAPPED_DATA_2)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DATA_TYPE2_NO, 1);
      else if(type == mem_fetch::REMAPPED_DATA_3)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DATA_TYPE3_NO, 1);
      else if(type == mem_fetch::REMAPPED_DATA_4)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DATA_TYPE4_NO, 1);
      else if(type == mem_fetch::REMAPPED_DATA_5)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DATA_TYPE5_NO, 1);
      else if(type == mem_fetch::REMAPPED_DATA_6) {
	  //printf("REMAPPED_DATA_6 is winner!!!!!!\n");
	  //mf->print_data_all(2);
	  g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DATA_TYPE6_NO, 1);
      }
      else {
	printf("type %d\n", type);
	assert(0);
      }
    }
  }
  */

  mem_fetch::COMP_ALGO_TYPE algo_type = mf->get_comp_algo_type();
  if(algo_type == mem_fetch::DSM_COMP) {
    int dsc_comp_res = mf->get_dsc_comp_res();
    assert(dsc_comp_res > 0);

    if(dsc_comp_res == 2)   	 g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DSC_COMP_2B_NO, 1);
    else if(dsc_comp_res == 4)   g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DSC_COMP_4B_NO, 1);
    else if(dsc_comp_res == 8)   g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DSC_COMP_8B_NO, 1);
    else if(dsc_comp_res == 16)  g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DSC_COMP_16B_NO, 1);
    else if(dsc_comp_res == 32)  g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DSC_COMP_32B_NO, 1);
    else if(dsc_comp_res == 64)  g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DSC_COMP_64B_NO, 1);
    else if(dsc_comp_res == 128) g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DSC_COMP_128B_NO, 1);

    mem_fetch::COMP_DATA_TYPE type = mf->get_dsc_comp_data_type();
    if(type == mem_fetch::ORG_DATA)		g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DATA_TYPE0_NO, 1);
    else if(type == mem_fetch::REMAPPED_DATA_1)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DATA_TYPE1_NO, 1);
    else if(type == mem_fetch::REMAPPED_DATA_2)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DATA_TYPE2_NO, 1);
    else if(type == mem_fetch::REMAPPED_DATA_3)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DATA_TYPE3_NO, 1);
    else if(type == mem_fetch::REMAPPED_DATA_4)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DATA_TYPE4_NO, 1);
    else if(type == mem_fetch::REMAPPED_DATA_5)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DATA_TYPE5_NO, 1);
    else if(type == mem_fetch::REMAPPED_DATA_6) {
		//printf("REMAPPED_DATA_6 is winner!!!!!!\n");
		//mf->print_data_all(2);
		g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DATA_TYPE6_NO, 1);
    }
    //added by kh(021817)
    else if(type == mem_fetch::CHAR_DATA) {
    	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DATA_TYPE7_NO, 1);
    }
    ///
    else {
      printf("type %d\n", type);
      assert(0);
    }

    //added by kh(072416)
    if(type > mem_fetch::ORG_DATA) {

      //added by (021817)
	  if(g_hpcl_comp_config.hpcl_char_preproc_en == 0) {

		  int data_type_index = mf->get_remapped_data_type_index(type);
		  unsigned remap_op_bits = mf->get_remap_op_bits(data_type_index);
		  if(remap_op_bits > (mf->get_real_data_size()/8)) {
			g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DSM_COMP_REMAP_OP_CASE_NO, 1);

			std::vector<enum mem_fetch::REMAP_OP_TYPE>& remap_op_type = mf->get_remap_op_type(data_type_index);
			for(int i = 0; i < remap_op_type.size(); i++) {
			  if(remap_op_type[i] == mem_fetch::MASK_OP)
				g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DSM_COMP_REMAP_MASK_OP_NO, 1);
			  else if(remap_op_type[i] == mem_fetch::NEIGHBOR_CONST_DELTA_OP)
				g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DSM_COMP_REMAP_NEIGHBOR_CONST_DELTA_OP_NO, 1);
			  else if(remap_op_type[i] == mem_fetch::COMP_OP)
				g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DSM_COMP_REMAP_COMP_OP_NO, 1);
			  else if(remap_op_type[i] == mem_fetch::DELTA_UPTO_1_OP)
				g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DSM_COMP_REMAP_DELTA_UPTO_1_OP_NO, 1);
			  else if(remap_op_type[i] == mem_fetch::DELTA_UPTO_3_OP)
				g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DSM_COMP_REMAP_DELTA_UPTO_3_OP_NO, 1);
			  else if(remap_op_type[i] == mem_fetch::COMP_DELTA_UPTO_1_OP)
				g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DSM_COMP_REMAP_COMP_DELTA_UPTO_1_OP_NO, 1);
			  else if(remap_op_type[i] == mem_fetch::COMP_DELTA_UPTO_3_OP)
				g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DSM_COMP_REMAP_COMP_DELTA_UPTO_3_OP_NO, 1);
			}

			//printf("REMAPPED OP added for mf %u\n", mf->get_request_uid());
		  } else if(remap_op_bits == (mf->get_real_data_size()/8)) {
			//printf("REMAPPED OP not added for mf %u\n", mf->get_request_uid());
		  }


	      //added by kh(072816)
	      if(mf->get_approx_op(data_type_index) > 0) {
	    	  g_hpcl_comp_anal->add_sample(hpcl_comp_anal::APPROX_DATA_NO, 1);
	      }

	      if(mf->get_approx_exception(data_type_index) > 0) {
	    	  g_hpcl_comp_anal->add_sample(hpcl_comp_anal::APPROX_EXCEPTION_NO, 1);
	      }
	      ///

	  }

    }
    ///




  } else if(algo_type == mem_fetch::LWM_COMP) {

    mem_fetch::COMP_DATA_TYPE type = mf->get_dsc_comp_data_type();
    int lwm_comp_res = mf->get_comp_res();

    if(type == mem_fetch::ORG_DATA) {
      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE0_NO, 1);
    } else if(type == mem_fetch::REMAPPED_DATA_1) {
      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE1_NO, 1);
    } else if(type == mem_fetch::REMAPPED_DATA_2) {
      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE2_NO, 1);
    } else if(type == mem_fetch::REMAPPED_DATA_3) {
      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE3_NO, 1);
    } else if(type == mem_fetch::REMAPPED_DATA_4) {
      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE4_NO, 1);
    } else if(type == mem_fetch::REMAPPED_DATA_5) {
      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE5_NO, 1);
    } else if(type == mem_fetch::REMAPPED_DATA_6) {
      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE6_NO, 1);
    } else assert(0);

    assert(lwm_comp_res > 0);
    //printf("LWM_COMP is winner!!!!!! comp_res %d data_type %d\n", lwm_comp_res, type);
    if(lwm_comp_res == 2) {
      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_2B_NO, 1);
      //mf->print_data_all(2);
    } else if(lwm_comp_res == 4) {
      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_4B_NO, 1);
      //mf->print_data_all(4);
    } else if(lwm_comp_res == 8) {
      g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_8B_NO, 1);
      //mf->print_data_all(8);
    }









  }


  double comp_data_ratio_algo = (double)(mf->get_real_data_size()*8-mf->get_comp_data_bits())/(mf->get_real_data_size()*8);
  if(algo_type == mem_fetch::DSM_COMP) {
    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DSM_COMP_NO, 1);
    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::DSM_COMP_DATA_RATIO, comp_data_ratio_algo);
    //std::cout << "comp_data_ratio " << comp_data_ratio << std::endl;
  } else if(algo_type == mem_fetch::LWM_COMP) {
    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::LWM_COMP_NO, 1);
    /*
    assert(min_comp_res > 0);
    if(min_comp_res == 2)		g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_2B_NO, 1);
    else if(min_comp_res == 4)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_4B_NO, 1);
    else if(min_comp_res == 8)	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_8B_NO, 1);
    else 				assert(0);
    */
    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::LWM_COMP_DATA_RATIO, comp_data_ratio_algo);
  } else if(algo_type == mem_fetch::NO_COMP) {
    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::NO_COMP_NO, 1);
  }



  ///






#ifdef old
    //added by kh(060316)
    //Measure compression ratio per reply without considering flit.
    double org_data_size = mf->get_data_size();
    double comp_data_size = mf->get_comp_data_size();
    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_DATA_RATIO, (org_data_size-comp_data_size)/org_data_size);
    if(mf->has_merged_mem_fetch() == true) {
      std::vector<mem_fetch*>& merged_mfs = mf->get_merged_mfs();
      for(unsigned j = 0; j < merged_mfs.size(); j++) {
	double org_merged_mf_data_size = merged_mfs[j]->get_data_size();
	unsigned merged_mf_comp_data_size = 0;
	if(g_hpcl_comp_config.hpcl_inter_comp_algo == hpcl_comp_config::BASIC_APPEND) {
	  merged_mf_comp_data_size = merged_mfs[j]->get_comp_data_size();
	} else {
	  merged_mf_comp_data_size = ceil(merged_mfs[j]->get_inter_comp_data_bits()/8.0);
	}
	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_DATA_RATIO, (org_merged_mf_data_size-merged_mf_comp_data_size)/org_merged_mf_data_size);

	/*
	printf("mf %u, INTRA_COMP_DATA_RATIO: %f, INTER_COMP_DATA_RATIO: %f, intra_comp_data: %u, inter_comp_data: %u, inter_comp_data_bits: %u\n",
	       merged_mfs[j]->get_request_uid(),
	       (org_merged_mf_data_size-merged_mfs[j]->get_comp_data_size())/org_merged_mf_data_size,
	       (org_merged_mf_data_size-merged_mf_comp_data_size)/org_merged_mf_data_size,
	       merged_mfs[j]->get_comp_data_size(),
	       merged_mf_comp_data_size,
	       merged_mfs[j]->get_inter_comp_data_bits());
	*/

      }
    }
    ///
#endif

  //}
  ///

  #ifdef old
  //added by kh(041616)
  double fragment_rate_per_flit = (double)(n_comp_flits*32 - response_size)/32;
  //added by kh(050216)
  if(mf->has_merged_mem_fetch() == true)	{
      double old_fragment_rate_per_flit = fragment_rate_per_flit;
      fragment_rate_per_flit = fragment_rate_per_flit/(mf->get_merged_mfs().size()+1);
	//std::cout << " old_fragment_rate_per_flit " << old_fragment_rate_per_flit;
	//std::cout << " fragment_rate_per_flit " << fragment_rate_per_flit;
	//std:cout << " mf->get_merged_mfs().size()+1 " << mf->get_merged_mfs().size()+1;
	//std::cout << std::endl;
	//assert(0);
  }
  ///
  g_hpcl_comp_anal->add_sample(hpcl_comp_anal::FRAGMENT_RATE_PER_FLIT, fragment_rate_per_flit);


  double fragment_rate_per_packet = (double)(n_comp_flits*32 - response_size)/(n_comp_flits*32);
  //added by kh(042316)
  //since multiple packets are merged, we average fragment_rate_per_packet by all number of mfs.
  if(mf->has_merged_mem_fetch() == true)	{
	double old_fragment_rate_per_packet = fragment_rate_per_packet;
	fragment_rate_per_packet = fragment_rate_per_packet/(mf->get_merged_mfs().size()+1);
//		      std::cout << " old_fragment_rate_per_packet " << old_fragment_rate_per_packet;
//		      std::cout << " fragment_rate_per_packet " << fragment_rate_per_packet;
//		      std:cout << " mf->get_merged_mfs().size()+1 " << mf->get_merged_mfs().size()+1;
//		      std::cout << std::endl;
	//assert(0);
  }
  ///

  g_hpcl_comp_anal->add_sample(hpcl_comp_anal::FRAGMENT_RATE_PER_PACKET, fragment_rate_per_packet);

  //std::cout << " n_comp_flits*32 " << n_comp_flits*32 << " response_size " << response_size;
  //std::cout << " fragment_rate_per_flit " << fragment_rate_per_flit;
  //std::cout << " fragment_rate_per_packet " << fragment_rate_per_packet << std::endl;
  if(n_comp_flits > n_org_flits) {
	  g_hpcl_comp_anal->add_sample(hpcl_comp_anal::COMP_OVERHEAD_NO, 1);
  }
  ///
  #endif

}

void hpcl_comp_lwm_aux::delete_local_dict_info(mem_fetch* mf)
{
  //added by kh(051916)
  //delete a local dictionary
  hpcl_dict<unsigned short>* loc_dict_2B = (hpcl_dict<unsigned short>*) mf->get_loc_dict(2);
  hpcl_dict<unsigned int>* loc_dict_4B = (hpcl_dict<unsigned int>*) mf->get_loc_dict(4);
  hpcl_dict<unsigned long long>* loc_dict_8B = (hpcl_dict<unsigned long long>*) mf->get_loc_dict(8);
  if(loc_dict_2B)	{	delete loc_dict_2B; mf->set_loc_dict(2, NULL);}
  if(loc_dict_4B)	{	delete loc_dict_4B; mf->set_loc_dict(4, NULL);}
  if(loc_dict_8B)	{	delete loc_dict_8B; mf->set_loc_dict(8, NULL);}

  for(unsigned k = 0; k < mf->get_trans_loc_dict_no(); k++) {
    hpcl_dict<unsigned short>* trans_loc_dict_2B = (hpcl_dict<unsigned short>*) mf->get_trans_loc_dict(2,k);
    hpcl_dict<unsigned int>* trans_loc_dict_4B = (hpcl_dict<unsigned int>*) mf->get_trans_loc_dict(4,k);
    hpcl_dict<unsigned long long>* trans_loc_dict_8B = (hpcl_dict<unsigned long long>*) mf->get_trans_loc_dict(8,k);
    if(trans_loc_dict_2B)	{	delete trans_loc_dict_2B; mf->set_trans_loc_dict(NULL,2,k);	}
    if(trans_loc_dict_4B)	{	delete trans_loc_dict_4B; mf->set_trans_loc_dict(NULL,4,k);	}
    if(trans_loc_dict_8B)	{	delete trans_loc_dict_8B; mf->set_trans_loc_dict(NULL,8,k);	}
  }

  std::vector<mem_fetch*>& merged_mfs = mf->get_merged_mfs();
  for(unsigned j = 0; j < merged_mfs.size(); j++) {
    hpcl_dict<unsigned short>* _loc_dict_2B = (hpcl_dict<unsigned short>*) merged_mfs[j]->get_loc_dict(2);
    hpcl_dict<unsigned int>* _loc_dict_4B = (hpcl_dict<unsigned int>*) merged_mfs[j]->get_loc_dict(4);
    hpcl_dict<unsigned long long>* _loc_dict_8B = (hpcl_dict<unsigned long long>*) merged_mfs[j]->get_loc_dict(8);
    if(_loc_dict_2B)	{	delete _loc_dict_2B; merged_mfs[j]->set_loc_dict(2, NULL);}
    if(_loc_dict_4B)	{	delete _loc_dict_4B; merged_mfs[j]->set_loc_dict(4, NULL);}
    if(_loc_dict_8B)	{	delete _loc_dict_8B; merged_mfs[j]->set_loc_dict(8, NULL);}

    for(unsigned k = 0; k < merged_mfs[j]->get_trans_loc_dict_no(); k++) {
      hpcl_dict<unsigned short>* trans_loc_dict_2B = (hpcl_dict<unsigned short>*) merged_mfs[j]->get_trans_loc_dict(2,k);
      hpcl_dict<unsigned int>* trans_loc_dict_4B = (hpcl_dict<unsigned int>*) merged_mfs[j]->get_trans_loc_dict(4,k);
      hpcl_dict<unsigned long long>* trans_loc_dict_8B = (hpcl_dict<unsigned long long>*) merged_mfs[j]->get_trans_loc_dict(8,k);
      if(trans_loc_dict_2B)	{	delete trans_loc_dict_2B; merged_mfs[j]->set_trans_loc_dict(NULL,2,k);	}
      if(trans_loc_dict_4B)	{	delete trans_loc_dict_4B; merged_mfs[j]->set_trans_loc_dict(NULL,4,k);	}
      if(trans_loc_dict_8B)	{	delete trans_loc_dict_8B; merged_mfs[j]->set_trans_loc_dict(NULL,8,k);	}
    }
  }
  ///

  for(unsigned j = 0; j < mf->get_merged_mf_to_same_SM_no(); j++) {
    mem_fetch* merged_mf = mf->get_merged_mf_to_same_SM(j);
    hpcl_dict<unsigned short>* _loc_dict_2B = (hpcl_dict<unsigned short>*) merged_mf->get_loc_dict(2);
    hpcl_dict<unsigned int>* _loc_dict_4B = (hpcl_dict<unsigned int>*) merged_mf->get_loc_dict(4);
    hpcl_dict<unsigned long long>* _loc_dict_8B = (hpcl_dict<unsigned long long>*) merged_mf->get_loc_dict(8);
    if(_loc_dict_2B)	{	delete _loc_dict_2B; merged_mf->set_loc_dict(2, NULL);}
    if(_loc_dict_4B)	{	delete _loc_dict_4B; merged_mf->set_loc_dict(4, NULL);}
    if(_loc_dict_8B)	{	delete _loc_dict_8B; merged_mf->set_loc_dict(8, NULL);}

    for(unsigned k = 0; k < merged_mf->get_trans_loc_dict_no(); k++) {
      hpcl_dict<unsigned short>* trans_loc_dict_2B = (hpcl_dict<unsigned short>*) merged_mf->get_trans_loc_dict(2,k);
      hpcl_dict<unsigned int>* trans_loc_dict_4B = (hpcl_dict<unsigned int>*) merged_mf->get_trans_loc_dict(4,k);
      hpcl_dict<unsigned long long>* trans_loc_dict_8B = (hpcl_dict<unsigned long long>*) merged_mf->get_trans_loc_dict(8,k);
      if(trans_loc_dict_2B)	{	delete trans_loc_dict_2B; merged_mf->set_trans_loc_dict(NULL,2,k);	}
      if(trans_loc_dict_4B)	{	delete trans_loc_dict_4B; merged_mf->set_trans_loc_dict(NULL,4,k);	}
      if(trans_loc_dict_8B)	{	delete trans_loc_dict_8B; merged_mf->set_trans_loc_dict(NULL,8,k);	}
    }
  }

}

void hpcl_comp_lwm_aux::clean_rec_comp_info(mem_fetch* mf)
{

#ifdef old
  //added by kh(062316)
  if(mf->has_merged_mem_fetch() == true) {

    if(g_hpcl_comp_config.hpcl_rec_comp_algo > hpcl_comp_config::BASIC_APPEND) {

      //added by kh(060416)
      //clear found flag
      //mfs are recursively compressed, but due to the availability of NI buffer, it cannot be sent.
      //need to clear all tags related to the recursive compression, because the recursive comp is
      //re-executed next time again
      unsigned comp_res = mf->get_comp_res();
      hpcl_dict<unsigned short>* loc_dict_2B = NULL;
      hpcl_dict<unsigned int>* loc_dict_4B = NULL;
      hpcl_dict<unsigned long long>* loc_dict_8B = NULL;
      std::vector<mem_fetch*>& merged_mfs = mf->get_merged_mfs();
      for(unsigned j = 0; j < merged_mfs.size(); j++) {
	    if(comp_res == 2) {
	      loc_dict_2B = (hpcl_dict<unsigned short>*) merged_mfs[j]->get_loc_dict(comp_res);
	      unsigned word_no = loc_dict_2B->get_word_no();
	      for(unsigned k = 0; k < word_no; k++)	loc_dict_2B->clear_found_flag(k);
	    } else if(comp_res == 4) {
	      loc_dict_4B = (hpcl_dict<unsigned int>*) merged_mfs[j]->get_loc_dict(comp_res);
	      unsigned word_no = loc_dict_4B->get_word_no();
	      for(unsigned k = 0; k < word_no; k++)	loc_dict_4B->clear_found_flag(k);
	    } else if(comp_res == 8) {
	      loc_dict_8B = (hpcl_dict<unsigned long long>*) merged_mfs[j]->get_loc_dict(comp_res);
	      unsigned word_no = loc_dict_8B->get_word_no();
	      for(unsigned k = 0; k < word_no; k++)	loc_dict_8B->clear_found_flag(k);
	    }
	    else assert(0);
      }
      ////

    }

    mf->del_merged_mfs();

  }
#endif

}

int hpcl_comp_lwm_aux::compute_final_packet_size(mem_fetch* mf)
{
  int response_size = -1;
  if(mf->get_real_data_size() > 0) {

    response_size = mf->get_ctrl_size()*8 + mf->get_comp_data_bits();
    //if(mf->has_merged_mem_fetch() == true)	{
    if(mf->get_merged_mf_no() > 0 || mf->get_merged_mf_to_same_SM_no() > 0)	{
      response_size += mf->get_rec_comp_merged_pkt_bits();
    }
    response_size = (int) ceil(response_size/8.0);



#ifdef old
    response_size = mf->get_ctrl_size()+mf->get_comp_data_size();
    //added by kh(042316)
    //add the response size for merged mem_fetches
    if(mf->has_merged_mem_fetch() == true)	{
      if(g_hpcl_comp_config.hpcl_inter_comp_algo == hpcl_comp_config::BASIC_APPEND) {
	    response_size += mf->get_comp_size_merged_mem_fetch();
      } else if(g_hpcl_comp_config.hpcl_inter_comp_algo == hpcl_comp_config::INTER_COMP
	    || g_hpcl_comp_config.hpcl_inter_comp_algo == hpcl_comp_config::INTER_COMP_FB) {
	    //response_size = mf->get_ctrl_size()*8+mf->get_inter_comp_data_bits();
	    response_size += mf->get_comp_size_merged_mem_fetch();
	    //response_size = ceil((double)response_size/8.0);

	    DEBUG_PRINT("mf %u response_size %u\n", mf->get_request_uid(), response_size);


	    //std::cout << "[1] mf " << mf->get_request_uid() << " my_org_response_size " << (mf->get_ctrl_size()+mf->get_comp_data_size()) << " response_size " << response_size << std::endl;
      } else assert(0);
    }
#endif

  } else {

    response_size = mf->get_is_write()?mf->get_ctrl_size():mf->size();
    //mf->print(stdout);
    COMP_DEBUG_PRINT("%llu | WRITE | mf %u has ctrl_size %u\n", (gpu_sim_cycle+gpu_tot_sim_cycle), mf->get_request_uid(), mf->get_ctrl_size());
    if(mf->get_type()==CTRL_MSG)
    {
      //set 8 bytes control message single word update
      //response_size=(mf->get_comp_data_size());
      response_size = 8;
      //cout<<"Abhishek Response "<<response_size<<endl;
    }
    COMP_DEBUG_PRINT("%llu | WRITE | mf %u has response_size %d\n", (gpu_sim_cycle+gpu_tot_sim_cycle), mf->get_request_uid(), response_size);


  }

  return response_size;
}

/*
void hpcl_comp_lwm_aux::display_local_dict_info(mem_fetch* mf)
{
  if(mf->get_comp_res() == 2) {
    ((hpcl_dict<unsigned short>*) mf->get_loc_dict(2))->print();
  } else if(mf->get_comp_res() == 4) {
    ((hpcl_dict<unsigned int>*) mf->get_loc_dict(4))->print();
  } else if(mf->get_comp_res() == 8) {
    ((hpcl_dict<unsigned long long>*) mf->get_loc_dict(8))->print();
  } else assert(0);
}
*/


void hpcl_comp_lwm_aux::get_LWM_comp_result(mem_fetch* mf, enum mem_fetch::COMP_DATA_TYPE data_type, unsigned& min_lwm_bits, unsigned& min_lwm_res, bool debug)
{
  std::vector<std::vector<double>*> lwm_comp_ds_bits(3,NULL);
  std::vector<unsigned> comp_ds_bits(3,0);
  std::vector<unsigned> org_ds_bits(3,0);

  if(debug) DATA_SEG_LWM_COMP_DEBUG_PRINT("get_LWM_comp_result: MF %u\n", mf->get_request_uid());

  lwm_comp_ds_bits[0] = &mf->get_comp_ds_bits(2, data_type);
  lwm_comp_ds_bits[1] = &mf->get_comp_ds_bits(4, data_type);
  lwm_comp_ds_bits[2] = &mf->get_comp_ds_bits(8, data_type);
  for(int i = 0; i < mf->get_comp_ds_status_size(); i++) {
    //if(mf->get_comp_ds_status(i) == 0) {
      for(int j = 0; j < lwm_comp_ds_bits.size(); j++) {
	comp_ds_bits[j] += lwm_comp_ds_bits[j]->at(i);
	org_ds_bits[j] += (g_hpcl_comp_config.hpcl_dsc_min_ds_size*8);
      }
    //  if(debug) DATA_SEG_LWM_COMP_DEBUG_PRINT("\tDS %d - no compressed\n", i);
    //} else {
    //  if(debug) DATA_SEG_LWM_COMP_DEBUG_PRINT("\tDS %d - compressed\n", i);
    //}
  }

  if(debug) {
    DATA_SEG_LWM_COMP_DEBUG_PRINT("LWM(2B) = ");
    for(int i = 0; i < lwm_comp_ds_bits[0]->size(); i++) {
      DATA_SEG_LWM_COMP_DEBUG_PRINT("%3.0f ", lwm_comp_ds_bits[0]->at(i));
    }
    DATA_SEG_LWM_COMP_DEBUG_PRINT("\n");

    DATA_SEG_LWM_COMP_DEBUG_PRINT("LWM(4B) = ");
    for(int i = 0; i < lwm_comp_ds_bits[1]->size(); i++) {
      DATA_SEG_LWM_COMP_DEBUG_PRINT("%3.0f ", lwm_comp_ds_bits[1]->at(i));
    }
    DATA_SEG_LWM_COMP_DEBUG_PRINT("\n");

    DATA_SEG_LWM_COMP_DEBUG_PRINT("LWM(8B) = ");
    for(int i = 0; i < lwm_comp_ds_bits[2]->size(); i++) {
      DATA_SEG_LWM_COMP_DEBUG_PRINT("%3.0f ", lwm_comp_ds_bits[2]->at(i));
    }
    DATA_SEG_LWM_COMP_DEBUG_PRINT("\n");
  }
  unsigned min_comp_ds_bits = comp_ds_bits[0];
  unsigned min_comp_res = 2;
  for(int i = 1; i < comp_ds_bits.size(); i++) {
    if(min_comp_ds_bits > comp_ds_bits[i]) {
    min_comp_ds_bits = comp_ds_bits[i];
    if(i == 1) 	min_comp_res = 4;
    else if(i == 2) min_comp_res = 8;
    }
  }

  //if compressed data is larger than original data, give up the compression.
  //Usually no compression happens in this case.
  if(org_ds_bits[0] < min_comp_ds_bits) {
    min_comp_ds_bits = org_ds_bits[0];
    min_comp_ds_bits += 1;	//1-bit indicates compressed or not
    min_comp_res = 0;
  }


  min_lwm_bits = min_comp_ds_bits;
  min_lwm_res = min_comp_res;

  if(debug) DATA_SEG_LWM_COMP_DEBUG_PRINT("min_lwm_bits %u min_lwm_res %u\n", min_lwm_bits, min_lwm_res);

}

void hpcl_comp_lwm_aux::sub_overlay_comp_data(mem_fetch* mf, enum mem_fetch::COMP_DATA_TYPE data_type, unsigned& min_lwm_bits, unsigned& min_lwm_res, bool debug)
{
  std::vector<std::vector<double>*> lwm_comp_ds_bits(3,NULL);
  std::vector<unsigned> partial_comp_ds_bits(3,0);
  std::vector<unsigned> partial_org_ds_bits(3,0);

  if(debug) DATA_SEG_LWM_COMP_DEBUG_PRINT("sub_overlay_comp_data: MF %u\n", mf->get_request_uid());

  lwm_comp_ds_bits[0] = &mf->get_comp_ds_bits(2, data_type);
  lwm_comp_ds_bits[1] = &mf->get_comp_ds_bits(4, data_type);
  lwm_comp_ds_bits[2] = &mf->get_comp_ds_bits(8, data_type);
  for(int i = 0; i < mf->get_comp_ds_status_size(); i++) {
    if(mf->get_comp_ds_status(i, data_type) == 0) {
      for(int j = 0; j < lwm_comp_ds_bits.size(); j++) {
	partial_comp_ds_bits[j] += lwm_comp_ds_bits[j]->at(i);
	partial_org_ds_bits[j] += (g_hpcl_comp_config.hpcl_dsc_min_ds_size*8);
      }
      if(debug) DATA_SEG_LWM_COMP_DEBUG_PRINT("\tDS %d - no compressed\n", i);
    } else {
      if(debug) DATA_SEG_LWM_COMP_DEBUG_PRINT("\tDS %d - compressed\n", i);
    }
  }

  if(debug) {
    DATA_SEG_LWM_COMP_DEBUG_PRINT("LWM(2B) = ");
    for(int i = 0; i < lwm_comp_ds_bits[0]->size(); i++) {
      DATA_SEG_LWM_COMP_DEBUG_PRINT("%3.0f ", lwm_comp_ds_bits[0]->at(i));
    }
    DATA_SEG_LWM_COMP_DEBUG_PRINT("\n");

    DATA_SEG_LWM_COMP_DEBUG_PRINT("LWM(4B) = ");
    for(int i = 0; i < lwm_comp_ds_bits[1]->size(); i++) {
      DATA_SEG_LWM_COMP_DEBUG_PRINT("%3.0f ", lwm_comp_ds_bits[1]->at(i));
    }
    DATA_SEG_LWM_COMP_DEBUG_PRINT("\n");

    DATA_SEG_LWM_COMP_DEBUG_PRINT("LWM(8B) = ");
    for(int i = 0; i < lwm_comp_ds_bits[2]->size(); i++) {
      DATA_SEG_LWM_COMP_DEBUG_PRINT("%3.0f ", lwm_comp_ds_bits[2]->at(i));
    }
    DATA_SEG_LWM_COMP_DEBUG_PRINT("\n");
  }
  unsigned min_partial_comp_ds_bits = partial_comp_ds_bits[0];
  unsigned min_comp_res = 2;
  for(int i = 1; i < partial_comp_ds_bits.size(); i++) {
    if(min_partial_comp_ds_bits > partial_comp_ds_bits[i]) {
    min_partial_comp_ds_bits = partial_comp_ds_bits[i];
    if(i == 1) 	min_comp_res = 4;
    else if(i == 2) min_comp_res = 8;
    }
  }

  //if compressed data is larger than original data, give up the compression.
  //Usually no compression happens in this case.
  if(partial_org_ds_bits[0] < min_partial_comp_ds_bits) {
    min_partial_comp_ds_bits = partial_org_ds_bits[0];
    min_partial_comp_ds_bits += 1;	//1-bit indicates compressed or not
    min_comp_res = 0;
  }


  min_lwm_bits = min_partial_comp_ds_bits;
  min_lwm_res = min_comp_res;

  if(debug) DATA_SEG_LWM_COMP_DEBUG_PRINT("min_lwm_bits %u min_lwm_res %u\n", min_lwm_bits, min_lwm_res);

}

void hpcl_comp_lwm_aux::overlay_comp_data(mem_fetch* mf, enum mem_fetch::COMP_DATA_TYPE data_type, unsigned& min_part_comp_bits, unsigned& min_comp_res, enum mem_fetch::COMP_DATA_TYPE& min_data_type)
{
  //enum mem_fetch::COMP_DATA_TYPE data_type = mf->get_dsc_comp_data_type();

  if(data_type > mem_fetch::NO_DATA_TYPE) {

    unsigned min_lwm_part_comp_bits = 0;
    unsigned min_lwm_comp_res = 0;
    sub_overlay_comp_data(mf, data_type, min_lwm_part_comp_bits, min_lwm_comp_res, true);

    min_part_comp_bits = min_lwm_part_comp_bits;
    min_comp_res = min_lwm_comp_res;
    min_data_type = data_type;

    //Testing
    /*
    #ifdef DATA_SEG_LWM_COMP_DEBUG
    printf("Selected Data Type %d\n", data_type);
    mf->print_data(data_type);
    unsigned final_comp_bits = min_part_comp_bits + mf->get_dsc_compressed_bits_only();
    printf("DSC_LWM comp_bits = %u\n", final_comp_bits);

    unsigned min_comp_bits = 0;
    unsigned min_comp_res = 0;
    mf->print_data(mem_fetch::ORG_DATA);
    get_LWM_comp_result(mf, mem_fetch::ORG_DATA, min_comp_bits, min_comp_res, true);
    printf("LWM comp_bits with ORG Data = %u, comp_res = %u\n", min_comp_bits, min_comp_res);

    if(min_comp_bits < final_comp_bits) {
      printf("LWM is better than DSC_LWM!!!!!!\n");
    }
    #endif
    */

  } else if(data_type == mem_fetch::NO_DATA_TYPE) {

      if(g_hpcl_comp_config.hpcl_ext_lwm_data_type == hpcl_comp_config::OPT_DATA) {

	vector<unsigned> lwm_part_comp_bits(3,0);
	vector<unsigned> lwm_comp_res(3,0);

	sub_overlay_comp_data(mf, mem_fetch::ORG_DATA, lwm_part_comp_bits[0], lwm_comp_res[0], true);
	sub_overlay_comp_data(mf, mem_fetch::REMAPPED_DATA_1, lwm_part_comp_bits[1], lwm_comp_res[1], true);
	sub_overlay_comp_data(mf, mem_fetch::REMAPPED_DATA_2, lwm_part_comp_bits[2], lwm_comp_res[2], true);

	unsigned min_lwm_part_comp_bits = lwm_part_comp_bits[0];
	unsigned min_lwm_comp_res = lwm_comp_res[0];
	enum mem_fetch::COMP_DATA_TYPE min_lwm_data_type = mem_fetch::ORG_DATA;
	for(int i = 1; i < lwm_part_comp_bits.size(); i++) {
	  if(min_lwm_part_comp_bits > lwm_part_comp_bits[i]) {
	    min_lwm_part_comp_bits = lwm_part_comp_bits[i];
	    min_lwm_comp_res = lwm_comp_res[i];
	    if(i == 1) 	min_lwm_data_type = mem_fetch::REMAPPED_DATA_1;
	    else if(i == 2) min_lwm_data_type = mem_fetch::REMAPPED_DATA_2;
	  }
	}

	min_part_comp_bits = min_lwm_part_comp_bits;
	min_comp_res = min_lwm_comp_res;
	min_data_type = min_lwm_data_type;


	if(min_data_type == mem_fetch::ORG_DATA) {
	  g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE0_NO, 1);
	  DATA_SEG_LWM_COMP_DEBUG_PRINT("mf %u, comp_res %u, ORG_DATA_BEST\n", mf->get_request_uid(), min_comp_res);
	} else if(min_data_type == mem_fetch::REMAPPED_DATA_1) {
	  g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE1_NO, 1);
	  DATA_SEG_LWM_COMP_DEBUG_PRINT("mf %u, comp_res %u, REMAPPED_DATA1_BEST\n", mf->get_request_uid(), min_comp_res);
	} else if(min_data_type == mem_fetch::REMAPPED_DATA_2) {
	  g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE2_NO, 1);
	  DATA_SEG_LWM_COMP_DEBUG_PRINT("mf %u, comp_res %u, REMAPPED_DATA2_BEST\n", mf->get_request_uid(), min_comp_res);
	} else if(min_data_type == mem_fetch::REMAPPED_DATA_3) {
	  g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE3_NO, 1);
	  DATA_SEG_LWM_COMP_DEBUG_PRINT("mf %u, comp_res %u, REMAPPED_DATA3_BEST\n", mf->get_request_uid(), min_comp_res);
	} else if(min_data_type == mem_fetch::REMAPPED_DATA_4) {
	  g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE4_NO, 1);
	  DATA_SEG_LWM_COMP_DEBUG_PRINT("mf %u, comp_res %u, REMAPPED_DATA4_BEST\n", mf->get_request_uid(), min_comp_res);
	} else if(min_data_type == mem_fetch::REMAPPED_DATA_5) {
	  g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE5_NO, 1);
	  DATA_SEG_LWM_COMP_DEBUG_PRINT("mf %u, comp_res %u, REMAPPED_DATA5_BEST\n", mf->get_request_uid(), min_comp_res);
	} else if(min_data_type == mem_fetch::REMAPPED_DATA_6) {
	  g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE6_NO, 1);
	  DATA_SEG_LWM_COMP_DEBUG_PRINT("mf %u, comp_res %u, REMAPPED_DATA6_BEST\n", mf->get_request_uid(), min_comp_res);
	} else assert(0);

	#ifdef DATA_SEG_LWM_COMP_DEBUG
	mf->print_data(mem_fetch::ORG_DATA);
	mf->print_data(mem_fetch::REMAPPED_DATA_1);
	mf->print_data(mem_fetch::REMAPPED_DATA_2);
	#endif

      } else if(g_hpcl_comp_config.hpcl_ext_lwm_data_type == hpcl_comp_config::ORG_DATA) {
	sub_overlay_comp_data(mf, mem_fetch::ORG_DATA, min_part_comp_bits, min_comp_res, true);
	min_data_type = mem_fetch::ORG_DATA;
	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE0_NO, 1);
      } else if(g_hpcl_comp_config.hpcl_ext_lwm_data_type == hpcl_comp_config::REMAPPED_DATA_1) {
	sub_overlay_comp_data(mf, mem_fetch::REMAPPED_DATA_1, min_part_comp_bits, min_comp_res, true);
	min_data_type = mem_fetch::REMAPPED_DATA_1;
	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE1_NO, 1);
      } else if(g_hpcl_comp_config.hpcl_ext_lwm_data_type == hpcl_comp_config::REMAPPED_DATA_2) {
	sub_overlay_comp_data(mf, mem_fetch::REMAPPED_DATA_2, min_part_comp_bits, min_comp_res, true);
	min_data_type = mem_fetch::REMAPPED_DATA_2;
	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE2_NO, 1);
      } else if(g_hpcl_comp_config.hpcl_ext_lwm_data_type == hpcl_comp_config::REMAPPED_DATA_3) {
	sub_overlay_comp_data(mf, mem_fetch::REMAPPED_DATA_3, min_part_comp_bits, min_comp_res, true);
	min_data_type = mem_fetch::REMAPPED_DATA_3;
	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE3_NO, 1);
      } else if(g_hpcl_comp_config.hpcl_ext_lwm_data_type == hpcl_comp_config::REMAPPED_DATA_4) {
	sub_overlay_comp_data(mf, mem_fetch::REMAPPED_DATA_4, min_part_comp_bits, min_comp_res, true);
	min_data_type = mem_fetch::REMAPPED_DATA_4;
	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE4_NO, 1);
      } else if(g_hpcl_comp_config.hpcl_ext_lwm_data_type == hpcl_comp_config::REMAPPED_DATA_5) {
	sub_overlay_comp_data(mf, mem_fetch::REMAPPED_DATA_5, min_part_comp_bits, min_comp_res, true);
	min_data_type = mem_fetch::REMAPPED_DATA_5;
	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE5_NO, 1);
      } else if(g_hpcl_comp_config.hpcl_ext_lwm_data_type == hpcl_comp_config::REMAPPED_DATA_6) {
	sub_overlay_comp_data(mf, mem_fetch::REMAPPED_DATA_6, min_part_comp_bits, min_comp_res, true);
	min_data_type = mem_fetch::REMAPPED_DATA_6;
	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::EXT_LWM_DATA_TYPE6_NO, 1);
      }
  }



}

//added by kh(071216)
double hpcl_comp_lwm_aux::get_redund_nibble_rate_for_LWM(mem_fetch* mf)
{
  if(!mf)	return -1;

  if(mf->get_comp_algo_type() != mem_fetch::LWM_COMP)	return -1;

  enum mem_fetch::COMP_DATA_TYPE data_type = mf->get_dsc_comp_data_type();
  unsigned res = mf->get_comp_res();

  hpcl_dict<unsigned short>* _loc_dict_2B = NULL;
  hpcl_dict<unsigned int>* _loc_dict_4B = NULL;
  hpcl_dict<unsigned long long>* _loc_dict_8B = NULL;
  if(data_type == mem_fetch::ORG_DATA) {
    if(res == 2)	_loc_dict_2B = (hpcl_dict<unsigned short>*) mf->get_loc_dict(2);
    else if(res == 4)	_loc_dict_4B = (hpcl_dict<unsigned int>*) mf->get_loc_dict(4);
    else if(res == 8)	_loc_dict_8B = (hpcl_dict<unsigned long long>*) mf->get_loc_dict(8);
  } else if(data_type == mem_fetch::REMAPPED_DATA_1) {
    if(res == 2)	_loc_dict_2B = (hpcl_dict<unsigned short>*) mf->get_trans_loc_dict(2,0);
    else if(res == 4)	_loc_dict_4B = (hpcl_dict<unsigned int>*) mf->get_trans_loc_dict(4,0);
    else if(res == 8)	_loc_dict_8B = (hpcl_dict<unsigned long long>*) mf->get_trans_loc_dict(8,0);
  } else if(data_type == mem_fetch::REMAPPED_DATA_2) {
    if(res == 2)	_loc_dict_2B = (hpcl_dict<unsigned short>*) mf->get_trans_loc_dict(2,1);
    else if(res == 4)	_loc_dict_4B = (hpcl_dict<unsigned int>*) mf->get_trans_loc_dict(4,1);
    else if(res == 8)	_loc_dict_8B = (hpcl_dict<unsigned long long>*) mf->get_trans_loc_dict(8,1);
  }
  double redun_nibble_rate = 0;
  if(res == 2)		redun_nibble_rate = _loc_dict_2B->get_redund_nibble_no()/(mf->get_real_data_size()*2);
  else if(res == 4)	redun_nibble_rate = _loc_dict_4B->get_redund_nibble_no()/(mf->get_real_data_size()*2);
  else if(res == 8)	redun_nibble_rate = _loc_dict_8B->get_redund_nibble_no()/(mf->get_real_data_size()*2);

  return redun_nibble_rate;
}

double hpcl_comp_lwm_aux::get_redund_nibble_rate_for_DSM(mem_fetch* mf)
{
  if(!mf)	return -1;

  if(mf->get_comp_algo_type() != mem_fetch::DSM_COMP)	return -1;

  enum mem_fetch::COMP_DATA_TYPE data_type = mf->get_dsc_comp_data_type();
  std::vector<unsigned char>* cache_data;
  if(data_type == mem_fetch::ORG_DATA)			cache_data = &mf->get_real_data_ptr();
  else if(data_type == mem_fetch::REMAPPED_DATA_1)	cache_data = &mf->get_trans_data_ptr(0);
  else if(data_type == mem_fetch::REMAPPED_DATA_2)	cache_data = &mf->get_trans_data_ptr(1);
  else	assert(0);

  std::map<unsigned char, unsigned> redund_cnt;
  for(unsigned char i = 0; i < 16; i++) {
    redund_cnt[i] = 0;
  }

  unsigned dsm_res = mf->get_dsc_comp_res();
  unsigned ds_no = mf->get_real_data_size()/dsm_res;
  for(int i = 0; i < ds_no; i++) {
    unsigned enc_status = mf->get_comp_ds_status(i, data_type);
    if(enc_status == 0) {
      int start_index = i*dsm_res;
      int end_index = start_index+dsm_res;
      for(int j = start_index; j < end_index; j++) {
	unsigned char byte_data = cache_data->at(j);
	unsigned char high_nibble = (byte_data & 0xf0) >> 4;
	unsigned char low_nibble = (byte_data & 0x0f);
	redund_cnt[high_nibble]++;
	redund_cnt[low_nibble]++;
      }
    }
  }

  double redun_nibble_rate = 0;
  for(unsigned char i = 0; i < 16; i++) {
    if(redund_cnt[i] > 1)	redun_nibble_rate += redund_cnt[i];
  }
  redun_nibble_rate = redun_nibble_rate/(mf->get_real_data_size()*2);

  return redun_nibble_rate;
}



#endif /* HPCL_COMP_LWM_AUX_H_ */
