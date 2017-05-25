/*
 * NetworkStat.cpp
 *
 *  Created on: Sep 19, 2015
 *      Author: mumichang
 */

#include "hpcl_rec_comp_lwm_pl_proc.h"
#include "hpcl_user_define_stmts.h"
#include <iostream>
#include <cassert>
#include <cmath>

extern unsigned long long  gpu_sim_cycle;
extern unsigned long long  gpu_tot_sim_cycle;
//

#include "hpcl_comp_config.h"
extern hpcl_comp_config g_hpcl_comp_config;

#include "hpcl_dict.h"

//added by kh(062816)
#include "hpcl_comp_anal.h"
extern hpcl_comp_anal* g_hpcl_comp_anal;
///
#include "gpu-cache.h"
extern l2_cache_config g_L2_config;

//added by kh(071516)
#include "hpcl_comp.h"
extern hpcl_comp* g_hpcl_comp;




hpcl_rec_comp_lwm_pl_proc::hpcl_rec_comp_lwm_pl_proc() {
  // TODO Auto-generated constructor stub
}


hpcl_rec_comp_lwm_pl_proc::~hpcl_rec_comp_lwm_pl_proc() {
  // TODO Auto-generated destructor stub
}

void hpcl_rec_comp_lwm_pl_proc::push_input(mem_fetch* new_mf)
{
  m_input_link->m_data = new_mf;
  //m_input_link->m_data2 = prev_mf;
}

mem_fetch* hpcl_rec_comp_lwm_pl_proc::pop_output()
{
  return m_output_link->m_data;
}


void hpcl_rec_comp_lwm_pl_proc::set_rec_comp_buffer(hpcl_rec_comp_buffer* rec_comp_buffer)
{
  m_rec_comp_buffer = rec_comp_buffer;
}

void hpcl_rec_comp_lwm_pl_proc::merge_mf(mem_fetch* new_mf, mem_fetch* prev_mf)
{
  if(prev_mf->get_tpc() == new_mf->get_tpc()) {
    prev_mf->add_merged_mf_to_same_SM(new_mf);
  } else {
    bool found = false;
    for(unsigned i = 0; i < prev_mf->get_merged_mf_no(); i++) {
      mem_fetch* merged_mf = prev_mf->get_merged_mf(i);
      if(merged_mf->get_tpc() == new_mf->get_tpc()) {
	merged_mf->add_merged_mf_to_same_SM(new_mf);
	found = true;
	break;
      }
    }
    if(found == false) {
      prev_mf->add_merged_mf(new_mf);
    }
  }
}

mem_fetch* hpcl_rec_comp_lwm_pl_proc::rec_append_method(mem_fetch* new_mf, mem_fetch* prev_mf)
{
  mem_fetch* ret = NULL;
  if(prev_mf) {
    if(new_mf->get_real_data_size() > 0) {
      /*
      bool is_rec_compressible = false;
      if(g_hpcl_comp_config.hpcl_rec_comp_target == hpcl_comp_config::READ_REP_TO_SAME_SM) {
	if(new_mf->get_tpc() == prev_mf->get_tpc())	is_rec_compressible = true;
	else {
	  ret = new_mf;
	}
      } else if(g_hpcl_comp_config.hpcl_rec_comp_target == hpcl_comp_config::READ_REP_TO_ALL_SM) {
	is_rec_compressible = true;	//always
      } else {
	assert(0);
      }

      if(is_rec_compressible == true) {
      */
	unsigned prev_rec_pkt_bits = prev_mf->get_rec_comp_merged_pkt_bits();
	unsigned prev_comp_pkt_bits = prev_mf->get_comp_data_bits();
	prev_comp_pkt_bits += (prev_mf->get_ctrl_size()*8);

	unsigned new_pkt_bits = new_mf->get_comp_data_bits();
	new_pkt_bits += new_mf->get_ctrl_size()*8;

	unsigned comp_byte_size = (unsigned) ceil((new_pkt_bits+prev_rec_pkt_bits+prev_comp_pkt_bits)/8.0);
	REC_COMP_DEBUG_PRINT("\t%llu | prev_rec_pkt_bits %u, prev_comp_pkt_bits %u, new_pkt_bits %u\n", (gpu_sim_cycle+gpu_tot_sim_cycle), prev_rec_pkt_bits, prev_comp_pkt_bits, new_pkt_bits);
	REC_COMP_DEBUG_PRINT("\t%llu | comp_byte_size %u, m_max_comp_data_size %u\n", (gpu_sim_cycle+gpu_tot_sim_cycle), comp_byte_size, m_max_comp_data_size);

	if(comp_byte_size <= m_max_comp_data_size) {
	  //deleted by kh(062516)
	  //prev_mf->add_merged_mf(new_mf);
	  //added by kh(062516)
	  merge_mf(new_mf, prev_mf);
	  ///
	  prev_mf->add_rec_comp_merged_pkt_bits(new_pkt_bits);
	  new_mf->add_rec_comp_data_bits(new_mf->get_comp_data_bits());
	  REC_COMP_DEBUG_PRINT("\t%llu | new mf %u (sm % u) is merged into prev mf %u (sm % u)\n", (gpu_sim_cycle+gpu_tot_sim_cycle), new_mf->get_request_uid(), new_mf->get_tpc(), prev_mf->get_request_uid(), prev_mf->get_tpc());
	  //assert(0);
	} else {
	  ret = new_mf;
	  //m_rec_comp_buffer->push_mem_fetch(new_mf);
	  //new_mf->set_rec_comp_pkt_bits(0);
	  REC_COMP_DEBUG_PRINT("\t%llu | new mf %u is pushed into rec comp buffer due to not enough space\n", (gpu_sim_cycle+gpu_tot_sim_cycle), new_mf->get_request_uid());
	}

      //}

    }
  }

  return ret;
  /*
  //Step0: mf may not be popped in the previous iteration, clear previous merged mfs
  mf->del_merged_mfs();

  //added by kh(060416)
  bool recursive_comp_enable = g_hpcl_comp_config.is_rec_comp_avail(mf->get_comp_res());
  //

  if(recursive_comp_enable == true) {

    //Step1: choose mem_fetchs to be merged
    unsigned merged_reply_size = mf->get_ctrl_size()+mf->get_comp_data_size();
    std::vector<mem_fetch*>& all_waiting_mfs = m_hpcl_comp_buffer->get_all_waiting_mfs (mf);
    std::vector<mem_fetch*> merged_mfs;
    for(unsigned j = 0; j < all_waiting_mfs.size() && all_waiting_mfs.size() > 1; j++) {
      //deleted by kh(060316)
      //if(all_waiting_mfs[j] != mf && all_waiting_mfs[j]->get_real_data_size() > 0) {
      //if(all_waiting_mfs[j] != mf
      //	  && all_waiting_mfs[j]->get_real_data_size() > 0
      // 	  && g_hpcl_comp_config.is_rec_comp_avail(all_waiting_mfs[j]->get_comp_res()) )
      //added by kh(060516)
      if(all_waiting_mfs[j] != mf
      	  && all_waiting_mfs[j]->get_real_data_size() > 0
       	  && all_waiting_mfs[j]->get_comp_res() == mf->get_comp_res())
      {
	unsigned old_merged_reply_size = merged_reply_size;
	unsigned merged_data_size = (all_waiting_mfs[j]->get_ctrl_size()+all_waiting_mfs[j]->get_comp_data_size());
	merged_reply_size += merged_data_size;
	//std::cout << " merged_data_size " << merged_data_size << " merged_reply_size " << merged_reply_size << std::endl;
	if(merged_reply_size <= m_max_comp_data_size) {
	  merged_mfs.push_back(all_waiting_mfs[j]);
	} else {
	  break;
	}
      }
    }

    //Step2: add merged mem_fetch
    mf->add_merged_mfs (merged_mfs);
  }
  */
}

#ifdef temp
void hpcl_rec_comp_lwm_pl_proc::rec_lwm_method(mem_fetch* mf)
{
  //Step0: mf may not be popped in the previous iteration, clear previous merged mfs
  mf->del_merged_mfs();
  //extend the encoding status bits from 1-bit to 2-bit
  //unsigned word_no = mf->get_real_data_size()/g_hpcl_comp_config.hpcl_comp_word_size;
  //mf->set_inter_comp_data_bits(mf->get_comp_data_size()*8+word_no);

  //Step1: choose mem_fetchs to be merged
  //double merged_reply_bits = mf->get_ctrl_size()*8+mf->get_inter_comp_data_bits();
  double total_merged_reply_bits = mf->get_ctrl_size()*8+mf->get_comp_data_size()*8;
  //unsigned data_ptr_bits = (unsigned) ceil(log2(mf->get_real_data_size()/g_hpcl_comp_config.hpcl_comp_word_size));
  //printf("mf %u,intra_comp_size %u\n", mf->get_request_uid(), mf->get_comp_data_size());

  unsigned comp_res = 0;
  hpcl_dict<unsigned short>* loc_dict_2B = NULL;
  hpcl_dict<unsigned int>* loc_dict_4B = NULL;
  hpcl_dict<unsigned long long>* loc_dict_8B = NULL;

  if(g_hpcl_comp_config.hpcl_comp_word_size == 2)	comp_res = 2;
  else if(g_hpcl_comp_config.hpcl_comp_word_size == 4)	comp_res = 4;
  else if(g_hpcl_comp_config.hpcl_comp_word_size == 8)	comp_res = 8;
  else if(g_hpcl_comp_config.hpcl_comp_word_size == 14) comp_res = mf->get_comp_res();
  else assert(0);

  if(comp_res == 2) loc_dict_2B = (hpcl_dict<unsigned short>*) mf->get_loc_dict(comp_res);
  else if(comp_res == 4) loc_dict_4B = (hpcl_dict<unsigned int>*) mf->get_loc_dict(comp_res);
  else if(comp_res == 8) loc_dict_8B = (hpcl_dict<unsigned long long>*) mf->get_loc_dict(comp_res);
  else			 assert(0);

  //added by kh(053116)
  //for debugging
  ///if(comp_res == 2)	 loc_dict_2B->print();
  ///else if(comp_res == 4) loc_dict_4B->print();
  ///else if(comp_res == 8) loc_dict_8B->print();
  ///

  //added by kh(060216)
  bool recursive_comp_enable = g_hpcl_comp_config.is_rec_comp_avail(mf->get_comp_res());
  //let's restrict the recursive compression for 2B resolution
  //if(g_hpcl_comp_config.hpcl_comp_word_size == 14 && comp_res != 2) {
  // recursive_comp_enable = false;
  //}
  ///


  if(recursive_comp_enable == true)
  {
    unsigned data_ptr_bits = (unsigned) ceil(log2((double)mf->get_real_data_size()/comp_res));

    INTERCOMP_DEBUG_PRINT("mf %u, reply_bits %f\n", mf->get_request_uid(), total_merged_reply_bits);
    //printf("mf %u, reply_bits %f\n", mf->get_request_uid(), total_merged_reply_bits);

    //added by kh(060316)
    std::vector<hpcl_dict<unsigned short>* > loc_dict_2B_hist;
    std::vector<hpcl_dict<unsigned int>* > loc_dict_4B_hist;
    std::vector<hpcl_dict<unsigned long long>* > loc_dict_8B_hist;


    //added by kh(053116)
    hpcl_dict<unsigned short>* loc_dict_2B_sec = NULL;
    hpcl_dict<unsigned int>* loc_dict_4B_sec = NULL;
    hpcl_dict<unsigned long long>* loc_dict_8B_sec = NULL;
    ///

    //added by kh(060216)
    double merged_reply_bits = 0;
    std::vector<mem_fetch*>& all_waiting_mfs = m_hpcl_comp_buffer[i]->get_all_waiting_mfs (mf);
    std::vector<mem_fetch*> merged_mfs;
    for(unsigned j = 0; j < all_waiting_mfs.size() && all_waiting_mfs.size() > 1; j++)
    {

	if(all_waiting_mfs[j] != mf && all_waiting_mfs[j]->get_real_data_size() > 0) {
	  //added by kh(052416)
	  //When resolution mismatches, skip the recursive compression.
	  if(g_hpcl_comp_config.hpcl_inter_comp_algo == hpcl_comp_config::INTER_COMP) {
	    //added by kh(052016)
	    if(g_hpcl_comp_config.hpcl_comp_word_size == 14) {
	      if(all_waiting_mfs[j]->get_comp_res() != mf->get_comp_res()) {

		  //printf("resolution mismatch: mf's res %u, waiting mf's res %u\n",
		  //	 mf->get_comp_res(), all_waiting_mfs[j]->get_comp_res());
		  continue;
	      }
	    }
	    ///
	  }

	  //printf("\twaiting_mf %u\n", all_waiting_mfs[j]->get_request_uid());


	  //deleted by kh(053116), bug fix
	  //unsigned intra_comp_bits = all_waiting_mfs[j]->get_comp_data_size()*8;
	  //added by kh(053116)
	  unsigned intra_comp_bits = all_waiting_mfs[j]->get_comp_data_size(comp_res)*8;
	  //added by kh(060316)
	  unsigned org_intra_comp_bits = intra_comp_bits;

	  //INTERCOMP_DEBUG_PRINT("\tmf %u, merged_reply_bits %f\n", all_waiting_mfs[j]->get_request_uid(), (double)(mf->get_ctrl_size()*8+mf->get_comp_data_size(comp_res)*8));
	  ///

	  hpcl_dict<unsigned short>* _loc_dict_2B = NULL;
	  hpcl_dict<unsigned int>* _loc_dict_4B = NULL;
	  hpcl_dict<unsigned long long>* _loc_dict_8B = NULL;

	  //added by kh(052416)
	  //assert(all_waiting_mfs[j]->has_merged_mem_fetch() == false);
	  //printf("mf->get_comp_res() %u all_waiting_mfs[j]->get_comp_res() %u\n", mf->get_comp_res(), all_waiting_mfs[j]->get_comp_res());
	  //assert(all_waiting_mfs[j]->get_comp_res() == 2);


	  if(comp_res == 2)	    _loc_dict_2B = (hpcl_dict<unsigned short>* ) all_waiting_mfs[j]->get_loc_dict(comp_res);
	  else if(comp_res == 4)    _loc_dict_4B = (hpcl_dict<unsigned int>* ) all_waiting_mfs[j]->get_loc_dict(comp_res);
	  else if(comp_res == 8)    _loc_dict_8B = (hpcl_dict<unsigned long long>* ) all_waiting_mfs[j]->get_loc_dict(comp_res);
	  else			assert(0);

	  //added by kh(053116)
	  //for debugging
	  //if(comp_res == 2)		_loc_dict_2B->print();
	  //else if(comp_res == 4)	_loc_dict_4B->print();
	  //else if(comp_res == 8)    _loc_dict_8B->print();
	  ///


	  unsigned same_word_no = 0;
	  if(comp_res == 2)		loc_dict_2B->compare_dict(_loc_dict_2B, same_word_no);
	  else if(comp_res == 4)	loc_dict_4B->compare_dict(_loc_dict_4B, same_word_no);
	  else if(comp_res == 8)	loc_dict_8B->compare_dict(_loc_dict_8B, same_word_no);
	  else	assert(0);

	  INTERCOMP_DEBUG_PRINT("\tsame_word_no : %u, comp_res : %u\n", same_word_no, comp_res);
	  INTERCOMP_DEBUG_PRINT("\tintra_comp_bits1 %u,", intra_comp_bits);
	  intra_comp_bits -= (same_word_no*comp_res*8);
	  INTERCOMP_DEBUG_PRINT("\tintra_comp_bits2 %u,", intra_comp_bits);
	  intra_comp_bits += (same_word_no*data_ptr_bits);
	  INTERCOMP_DEBUG_PRINT("\tintra_comp_bits3 %u, ", intra_comp_bits);
	  //extend the encoding status bits from 1-bit to 2-bit
	  unsigned _word_no = all_waiting_mfs[j]->get_real_data_size()/comp_res;
	  intra_comp_bits += _word_no;	//extra encoding bits
	  INTERCOMP_DEBUG_PRINT("\tintra_comp_bits4 %u, ", intra_comp_bits);
	  unsigned merged_data_size = (all_waiting_mfs[j]->get_ctrl_size()*8+intra_comp_bits);
	  INTERCOMP_DEBUG_PRINT("\tintra_comp_bits5 %u\n", merged_data_size);
	  /*
	  merged_reply_bits += merged_data_size;
	  INTERCOMP_DEBUG_PRINT("\ttotal_merged_reply_bits %f\n", merged_reply_bits);
	  all_waiting_mfs[j]->set_inter_comp_data_bits(0);
	  all_waiting_mfs[j]->set_inter_comp_matching_word_no(0);
	  if(ceil(merged_reply_bits/8.0) <= 32*9) {
	    all_waiting_mfs[j]->set_inter_comp_data_bits(intra_comp_bits);
	    all_waiting_mfs[j]->set_inter_comp_matching_word_no(same_word_no);
	    merged_mfs.push_back(all_waiting_mfs[j]);
	    INTERCOMP_DEBUG_PRINT("\tsucc: mf %u, merged_data_size %f\n", all_waiting_mfs[j]->get_request_uid(), ceil(merged_reply_bits/8.0));
	  } else {
	    INTERCOMP_DEBUG_PRINT("\tfail: mf %u, merged_reply_bits %f, merged_data_size %f, 32*9 %u\n", all_waiting_mfs[j]->get_request_uid(), merged_reply_bits, ceil(merged_reply_bits/8.0), 32*9);
	    break;
	  }
	  */

	  //added by kh(053116)
	  //Second Recursive Compression
	  unsigned same_word_no_sec = 0;
	  /*
	  if(g_hpcl_comp_config.hpcl_inter_comp_window == 2) {
	    //second recursive compression
	    if(comp_res == 2 && loc_dict_2B_sec != NULL)      loc_dict_2B_sec->compare_dict(_loc_dict_2B, same_word_no_sec);
	    else if(comp_res == 4 && loc_dict_4B_sec != NULL) loc_dict_4B_sec->compare_dict(_loc_dict_4B, same_word_no_sec);
	    else if(comp_res == 8 && loc_dict_8B_sec != NULL) loc_dict_8B_sec->compare_dict(_loc_dict_8B, same_word_no_sec);

	    if(same_word_no_sec > 0) {
	      //printf("\tsame_word_no_sec : %u, comp_res : %u\n", same_word_no_sec, comp_res);
	      //INTERCOMP_DEBUG_PRINT("\tsame_word_no_sec : %u, comp_res : %u\n", same_word_no_sec, comp_res);
	      INTERCOMP_DEBUG_PRINT("\tintra_comp_bits1_sec %u,", intra_comp_bits);
	      intra_comp_bits -= (same_word_no_sec*comp_res*8);
	      INTERCOMP_DEBUG_PRINT("\tintra_comp_bits2_sec %u,", intra_comp_bits);
	      intra_comp_bits += (same_word_no_sec*data_ptr_bits);
	      INTERCOMP_DEBUG_PRINT("\tintra_comp_bits3_sec %u, ", intra_comp_bits);
	      unsigned merged_data_size_sec = (all_waiting_mfs[j]->get_ctrl_size()*8+intra_comp_bits);
	      INTERCOMP_DEBUG_PRINT("\tintra_comp_bits4_sec %u\n", merged_data_size_sec);
	      merged_data_size = merged_data_size_sec;
	    }
	    ///
	  }
	  */

	  //added by kh(060316)
	  //printf("try recursive comp\n");
	  //printf("hist size %u _loc_dict_2B %d\n", loc_dict_2B_hist.size(), _loc_dict_2B);
	  unsigned loc_dict_hist_size = 0;
	  if(comp_res == 2)		loc_dict_hist_size = loc_dict_2B_hist.size();
	  else if(comp_res == 4)	loc_dict_hist_size = loc_dict_4B_hist.size();
	  else if(comp_res == 8)	loc_dict_hist_size = loc_dict_8B_hist.size();
	  else assert(0);

	  for(unsigned k = 0; k < loc_dict_hist_size; k++) {
	    unsigned same_word_no_rec = 0;

	    //printf("before compare_dict\n");
	    if(comp_res == 2) loc_dict_2B_hist[k]->compare_dict(_loc_dict_2B, same_word_no_rec);
	    else if(comp_res == 4) loc_dict_4B_hist[k]->compare_dict(_loc_dict_4B, same_word_no_rec);
	    else if(comp_res == 8) loc_dict_8B_hist[k]->compare_dict(_loc_dict_8B, same_word_no_rec);

	    if(same_word_no_rec > 0) {
	      //printf("\tsame_word_no_sec : %u, comp_res : %u\n", same_word_no_sec, comp_res);
	      INTERCOMP_DEBUG_PRINT("\iter: %u, same_word_no_rec : %u, comp_res : %u\n", k, same_word_no_rec, comp_res);
	      INTERCOMP_DEBUG_PRINT("\tintra_comp_bits1_rec %u,", intra_comp_bits);
	      intra_comp_bits -= (same_word_no_rec*comp_res*8);
	      INTERCOMP_DEBUG_PRINT("\tintra_comp_bits2_rec %u,", intra_comp_bits);
	      intra_comp_bits += (same_word_no_rec*data_ptr_bits);
	      INTERCOMP_DEBUG_PRINT("\tintra_comp_bits3_rec %u\n", intra_comp_bits);

	      same_word_no_sec += same_word_no_rec;
	    }
	    //else {
	    //  INTERCOMP_DEBUG_PRINT("\same_word_no_rec %u\n", same_word_no_rec);
	    //}
	    //printf("after compare_dict1\n");

	    //added by kh(060416)
	    bool is_last = false;
	    if(comp_res == 2) {
	      if(k == (loc_dict_2B_hist.size()-1))	is_last = true;
	    } else if(comp_res == 4) {
	      if(k == (loc_dict_4B_hist.size()-1))	is_last = true;
	    } else if(comp_res == 8) {
	      if(k == (loc_dict_8B_hist.size()-1))	is_last = true;
	    } else assert(0);
	    ///

	    if(is_last == true) {
	      unsigned merged_data_size_sec = (all_waiting_mfs[j]->get_ctrl_size()*8+intra_comp_bits);
	      INTERCOMP_DEBUG_PRINT("\tintra_comp_bits4_rec %u\n", merged_data_size_sec);
	      merged_data_size = merged_data_size_sec;
	    }
	    //printf("after compare_dict2\n");
	  }

	  //added by kh(060316)
	  //If the recursive compression produces larger data, then return to the
	  //result of the original LWM compression
	  unsigned final_inter_comp_bits = merged_data_size;
	  unsigned final_intra_comp_bits = (org_intra_comp_bits+all_waiting_mfs[j]->get_ctrl_size()*8);
	  unsigned selected_comp_bits = 0;
	  if(final_inter_comp_bits > final_intra_comp_bits) {
	    selected_comp_bits = final_intra_comp_bits;
	    intra_comp_bits = org_intra_comp_bits;
	    INTERCOMP_DEBUG_PRINT("\tmf %u returns from final_inter_comp_bits %u to final_intra_comp_bits %u\n",all_waiting_mfs[j]->get_request_uid(), final_inter_comp_bits, final_intra_comp_bits);
	  } else {
	    selected_comp_bits = final_inter_comp_bits;
	    INTERCOMP_DEBUG_PRINT("\tmf %u sticks to final_inter_comp_bits %u, not final_intra_comp_bits %u\n",all_waiting_mfs[j]->get_request_uid(), final_inter_comp_bits, final_intra_comp_bits);
	    //printf("\tmf %u sticks to final_inter_comp_bits %u, not final_intra_comp_bits %u\n",all_waiting_mfs[j]->get_request_uid(), final_inter_comp_bits, final_intra_comp_bits);
	    //printf("\t%llu | mf %u sticks to final_inter_comp_bits %u, not final_intra_comp_bits %u\n",
	    //	   (gpu_sim_cycle+gpu_tot_sim_cycle), all_waiting_mfs[j]->get_request_uid(), intra_comp_bits, org_intra_comp_bits);
	  }
	  ///

	  merged_reply_bits += selected_comp_bits;
	  total_merged_reply_bits += selected_comp_bits;
	  INTERCOMP_DEBUG_PRINT("\ttotal_merged_reply_bits %f\n", total_merged_reply_bits);
	  all_waiting_mfs[j]->set_inter_comp_data_bits(0);
	  all_waiting_mfs[j]->set_inter_comp_matching_word_no(0);
	  if(ceil(total_merged_reply_bits/8.0) <= 32*g_NI_input_buffer_size) {
	    all_waiting_mfs[j]->set_inter_comp_data_bits(intra_comp_bits);
	    //printf("\t%llu, mf %u save comp_bits %u\n",
	    //	   (gpu_sim_cycle+gpu_tot_sim_cycle), all_waiting_mfs[j]->get_request_uid(), all_waiting_mfs[j]->get_inter_comp_data_bits());

	    if(final_inter_comp_bits < final_intra_comp_bits) {
	      //printf("inter_comp_bits: %u, intra_comp_bits: %u\n" ,intra_comp_bits, all_waiting_mfs[j]->get_comp_data_size()*8);
	      all_waiting_mfs[j]->set_inter_comp_matching_word_no(same_word_no+same_word_no_sec);
	    }
	    merged_mfs.push_back(all_waiting_mfs[j]);
	    INTERCOMP_DEBUG_PRINT("\tsucc: mf %u, merged_data_size %f\n", all_waiting_mfs[j]->get_request_uid(), ceil(total_merged_reply_bits/8.0));

	  } else {
	    INTERCOMP_DEBUG_PRINT("\tfail: mf %u, merged_reply_bits %f, merged_data_size %f, 32*g_NI_input_buffer_size %u\n", all_waiting_mfs[j]->get_request_uid(), total_merged_reply_bits, ceil(total_merged_reply_bits/8.0),32*g_NI_input_buffer_size);
	    break;
	  }

	  //for debugging
	  //added by kh(053116)
	  //if(same_word_no_sec > 0) {
	  //	  fflush(stdout);
	  //	  assert(0);
	  //}
	  ///

	  /*
	  //added by kh(053116)
	  if(g_hpcl_comp_config.hpcl_inter_comp_window == 2) {
	    //when the recursive window is set to 2 or more.
	    if(comp_res == 2 && loc_dict_2B_sec == NULL)	loc_dict_2B_sec = (hpcl_dict<unsigned short>* ) all_waiting_mfs[j]->get_loc_dict(comp_res);
	    else if(comp_res == 4 && loc_dict_4B_sec == NULL)	loc_dict_4B_sec = (hpcl_dict<unsigned int>* ) all_waiting_mfs[j]->get_loc_dict(comp_res);
	    else if(comp_res == 8 && loc_dict_8B_sec == NULL)	loc_dict_8B_sec = (hpcl_dict<unsigned long long>* ) all_waiting_mfs[j]->get_loc_dict(comp_res);
	    ///
	  }
	  */

	  //added by kh(060316)
	  bool can_save = false;
	  if(g_hpcl_comp_config.hpcl_inter_comp_window == 0)	can_save = true;
	  else {
	    if(comp_res == 2) {
	      if(loc_dict_2B_hist.size() < g_hpcl_comp_config.hpcl_inter_comp_window) can_save = true;
	      else	can_save = false;
	    } else if(comp_res == 4) {
	      if(loc_dict_4B_hist.size() < g_hpcl_comp_config.hpcl_inter_comp_window) can_save = true;
	      else	can_save = false;
	    } else if(comp_res == 8) {
	      if(loc_dict_8B_hist.size() < g_hpcl_comp_config.hpcl_inter_comp_window) can_save = true;
	      else	can_save = false;
	    } else assert(0);
	  }

	  if(can_save == true) {
	    if(comp_res == 2) 		loc_dict_2B_hist.push_back((hpcl_dict<unsigned short>*)all_waiting_mfs[j]->get_loc_dict(comp_res));
	    else if(comp_res == 4) 	loc_dict_4B_hist.push_back((hpcl_dict<unsigned int>*)all_waiting_mfs[j]->get_loc_dict(comp_res));
	    else if(comp_res == 8) 	loc_dict_8B_hist.push_back((hpcl_dict<unsigned long long>*)all_waiting_mfs[j]->get_loc_dict(comp_res));
	    else assert(0);
	  }


	}



    }

    //Step2: add merged mem_fetch
    mf->add_merged_mfs (merged_mfs);
    //added by kh(060216)
    mf->set_merged_mf_comp_size_bits(merged_reply_bits);

}
#endif

#ifdef old
void hpcl_rec_comp_lwm_pl_proc::rec_comp_test_proc(mem_fetch* new_mf)
{
  int comp_res = -1;
  int targe_sm = -1;
  if(g_hpcl_comp_config.hpcl_rec_comp_algo == hpcl_comp_config::BASIC_APPEND) {
	comp_res = -1;
  } else if(g_hpcl_comp_config.hpcl_rec_comp_algo == hpcl_comp_config::INTER_COMP) {
	comp_res = (int) new_mf->get_comp_res();
  }

  if(g_hpcl_comp_config.hpcl_rec_comp_target == hpcl_comp_config::READ_REP_TO_SAME_SM) {
	targe_sm = new_mf->get_tpc();
  } else {
	targe_sm = -1;
  }

  //added by kh(062816)
  //get prev_mf from rec_comp_buffer
  mem_fetch* prev_mf = NULL;
  hpcl_rec_comp_buffer::last_mem_fetch_status last_mf_status = m_rec_comp_buffer->get_last_mem_fetch(comp_res, targe_sm, &prev_mf);

//      if(m_par_id == 0) {
//	m_rec_comp_buffer->print();
//	if(prev_mf) {
//	  printf("mf %u is selected\n", prev_mf->get_request_uid());
//	} else {
//	  printf("no mf is selected\n");
//	}
//      }

  if(!prev_mf) {
	m_rec_comp_buffer->push_mem_fetch(new_mf);
	new_mf->set_rec_comp_merged_pkt_bits(0);
	new_mf->set_rec_comp_data_bits(0);
	//m_input_link->m_data = NULL;
	REC_COMP_DEBUG_PRINT("%llu | Stage2 | Rec_Comp %u | new mf %u is pushed into rec_comp_buffer due to no prev mf\n", (gpu_sim_cycle+gpu_tot_sim_cycle), m_par_id, new_mf->get_request_uid());
  }
  else {
	//if(new_mf && new_mf->get_real_data_size() > 0) {
	mem_fetch* unmerged_mf = NULL;
	if(g_hpcl_comp_config.hpcl_rec_comp_algo == hpcl_comp_config::BASIC_APPEND) {
	  unmerged_mf = rec_append_method(new_mf, prev_mf);
	} else if(g_hpcl_comp_config.hpcl_rec_comp_algo == hpcl_comp_config::INTER_COMP) {
	  //unmerged_mf = rec_lwm_method(new_mf, prev_mf);
	  if(targe_sm == -1)	unmerged_mf = rec_lwm_method(new_mf, prev_mf);
	  else			unmerged_mf = rec_lwm_method(new_mf, prev_mf, false);
	}

	if(unmerged_mf) {
	  last_mf_status = hpcl_rec_comp_buffer::LAST_MF_FULL;
	  m_rec_comp_buffer->push_mem_fetch(unmerged_mf);
	  unmerged_mf->set_rec_comp_merged_pkt_bits(0);
	  unmerged_mf->set_rec_comp_data_bits(0);
	}
	//} else {
	//m_rec_comp_buffer->push_mem_fetch(new_mf);
	//}
  }

  //added by kh(062816)
  if(last_mf_status == hpcl_rec_comp_buffer::NOT_FOUND) {
	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::REC_COMP_LAST_MF_NOT_FOUND, 1);
  } else if(last_mf_status == hpcl_rec_comp_buffer::LAST_MF_FULL) {
	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::REC_COMP_LAST_MF_FULL, 1);
  } else if(last_mf_status == hpcl_rec_comp_buffer::COMP_RES_MISMATCH) {
	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::REC_COMP_LAST_MF_COMP_RES_MISMATCH, 1);
  }
  ///
}
#endif

#ifdef old
void hpcl_rec_comp_lwm_pl_proc::rec_comp_algo_proc(mem_fetch* new_mf)
{
  int comp_res = new_mf->get_comp_res();
  int target_sm = new_mf->get_tpc();

  mem_fetch* prev_mf = m_rec_comp_buffer->get_mem_fetch_to_same_sm(comp_res, target_sm);
  if(prev_mf && prev_mf->get_merged_mf_no() == 0) {	//if prev_mf is not used for another reply to different SM
    mem_fetch* unmerged_mf = rec_lwm_method(new_mf, prev_mf, false, REC_COMP_INTRA_SM);
    //assert(!unmerged_mf);
    if(unmerged_mf) {
      m_rec_comp_buffer->push_mem_fetch(new_mf);
      new_mf->set_rec_comp_merged_pkt_bits(0);
      new_mf->set_rec_comp_data_bits(0);
    } else {


      #ifdef REC_ANAL_COMP_DEBUG
      REC_ANAL_COMP_DEBUG_PRINT("\tINTRA: mf %u (SM %u) is merged to mf %u (SM %u)\n", new_mf->get_request_uid(), new_mf->get_tpc(), prev_mf->get_request_uid(), prev_mf->get_tpc());

      REC_ANAL_COMP_DEBUG_PRINT("Best_Match_Same_SM: Prev mf %u Cur mf %u\n", prev_mf->get_request_uid(), new_mf->get_request_uid());

      REC_ANAL_COMP_DEBUG_PRINT("Prev_Data %u (%03u) = ", prev_mf->get_request_uid(), prev_mf->get_real_data_size());
      for(int i = prev_mf->get_data_size()-1; i >= 0 ; i--) {
	  REC_ANAL_COMP_DEBUG_PRINT("%02x ", prev_mf->get_real_data(i));
      }
      REC_ANAL_COMP_DEBUG_PRINT("\n");

      REC_ANAL_COMP_DEBUG_PRINT("Cur_Data %u (%03u) = ", new_mf->get_request_uid(), new_mf->get_real_data_size());
      for(int i = new_mf->get_data_size()-1; i >= 0 ; i--) {
	  REC_ANAL_COMP_DEBUG_PRINT("%02x ", new_mf->get_real_data(i));
      }
      REC_ANAL_COMP_DEBUG_PRINT("\n");


      REC_ANAL_COMP_DEBUG_PRINT("Prev_Trans_Data %u (%03u) = ", prev_mf->get_request_uid(), prev_mf->get_trans_data_size());
      for(int i = prev_mf->get_trans_data_size()-1; i >= 0 ; i--) {
	  REC_ANAL_COMP_DEBUG_PRINT("%02x ", prev_mf->get_trans_data(i));
      }
      REC_ANAL_COMP_DEBUG_PRINT("\n");

      REC_ANAL_COMP_DEBUG_PRINT("Cur_Trans_Data %u (%03u) = ", new_mf->get_request_uid(), new_mf->get_trans_data_size());
      for(int i = new_mf->get_trans_data_size()-1; i >= 0 ; i--) {
	  REC_ANAL_COMP_DEBUG_PRINT("%02x ", new_mf->get_trans_data(i));
      }
      REC_ANAL_COMP_DEBUG_PRINT("\n");


      #endif




    }
  } else {

    #ifdef old
    //Search another reply
    prev_mf = m_rec_comp_buffer->get_closest_mem_fetch(comp_res, target_sm, new_mf);
    if(prev_mf) {
      mem_fetch* unmerged_mf = rec_lwm_method(new_mf, prev_mf);
      assert(!unmerged_mf);
    } else {
      m_rec_comp_buffer->push_mem_fetch(new_mf);
      new_mf->set_rec_comp_merged_pkt_bits(0);
      new_mf->set_rec_comp_data_bits(0);
    }
    #endif

    prev_mf = NULL;
    std::vector<mem_fetch*> rec_comp_candi;
    m_rec_comp_buffer->get_candi_for_rec_comp_to_diff_sm(comp_res, target_sm, new_mf, rec_comp_candi);
    int min_flit_no_diff = -1;
    mem_fetch* best_match_prev_mf = NULL;
    for(int i = 0; i < rec_comp_candi.size(); i++) {
      int flit_no_diff = speculate_rec_lwm_method(new_mf, rec_comp_candi[i]);
      assert(flit_no_diff >= 0);

      if(min_flit_no_diff == -1) {
	min_flit_no_diff = flit_no_diff;
	best_match_prev_mf = rec_comp_candi[i];
      }
      else {
	if(min_flit_no_diff > flit_no_diff) {
	  min_flit_no_diff = flit_no_diff;
	  best_match_prev_mf = rec_comp_candi[i];
	}
      }
    }
    if(min_flit_no_diff == 0) {
      prev_mf = best_match_prev_mf;
    } else {


	#ifdef REC_ANAL_COMP_DEBUG
	if(best_match_prev_mf) {

	  REC_ANAL_COMP_DEBUG_PRINT("Best_Match_Diff_SM: Prev mf %u Cur mf %u\n", best_match_prev_mf->get_request_uid(), new_mf->get_request_uid());

	  REC_ANAL_COMP_DEBUG_PRINT("Prev_Data %u (%03u) = ", best_match_prev_mf->get_request_uid(), best_match_prev_mf->get_real_data_size());
	  for(int i = best_match_prev_mf->get_data_size()-1; i >= 0 ; i--) {
	      REC_ANAL_COMP_DEBUG_PRINT("%02x ", best_match_prev_mf->get_real_data(i));
	  }
	  REC_ANAL_COMP_DEBUG_PRINT("\n");

	  REC_ANAL_COMP_DEBUG_PRINT("Cur_Data %u (%03u) = ", new_mf->get_request_uid(), new_mf->get_real_data_size());
	  for(int i = new_mf->get_data_size()-1; i >= 0 ; i--) {
	      REC_ANAL_COMP_DEBUG_PRINT("%02x ", new_mf->get_real_data(i));
	  }
	  REC_ANAL_COMP_DEBUG_PRINT("\n");
	}
	#endif

    }

    if(prev_mf) {
      mem_fetch* unmerged_mf = rec_lwm_method(new_mf, prev_mf, false, REC_COMP_INTER_SM);
      //deleted by kh(063016)
      //assert(!unmerged_mf);
      if(unmerged_mf) {
	m_rec_comp_buffer->push_mem_fetch(unmerged_mf);
	unmerged_mf->set_rec_comp_merged_pkt_bits(0);
	unmerged_mf->set_rec_comp_data_bits(0);
      }
    } else {
      m_rec_comp_buffer->push_mem_fetch(new_mf);
      new_mf->set_rec_comp_merged_pkt_bits(0);
      new_mf->set_rec_comp_data_bits(0);
    }

  }
}
#endif

void hpcl_rec_comp_lwm_pl_proc::rec_comp_algo1_proc(mem_fetch* new_mf)
{
  int comp_res = new_mf->get_comp_res();
  int target_sm = new_mf->get_tpc();

  mem_fetch* prev_mf = m_rec_comp_buffer->get_mem_fetch_to_same_sm(target_sm);
  //prev_mf has not been used for merging another reply to different SM
  if(prev_mf) {
    mem_fetch* unmerged_mf = rec_comp_method_for_intra_sm(new_mf, prev_mf);
    //assert(!unmerged_mf);
    if(unmerged_mf) {
      m_rec_comp_buffer->push_mem_fetch(new_mf);
      new_mf->set_rec_comp_merged_pkt_bits(0);
      new_mf->set_rec_comp_data_bits(0);
    } else {

    }
  } else {
    m_rec_comp_buffer->push_mem_fetch(new_mf);
    new_mf->set_rec_comp_merged_pkt_bits(0);
    new_mf->set_rec_comp_data_bits(0);
  }
}

mem_fetch* hpcl_rec_comp_lwm_pl_proc::rec_comp_method_for_inter_sm(mem_fetch* new_mf)
{
  mem_fetch* ret = NULL;

  std::vector<mem_fetch*> diff_sm_mfs;
  int target_sm = new_mf->get_tpc();

  m_rec_comp_buffer->get_mem_fetch_to_diff_sm(target_sm, diff_sm_mfs);
  int mf_index = -1;

  m_comp_buffer->print();
  m_rec_comp_buffer->print();
  if(diff_sm_mfs.size() == 0) {
    std::cout << "no mem_fetch is found! for mf " << new_mf->get_request_uid() << std::endl;
  }

  for(int i = 0; i < diff_sm_mfs.size(); i++) {
    int is_same = g_hpcl_comp->compare_data(diff_sm_mfs[i], new_mf);
    if(is_same == 0) {
      mf_index = i;
      std::cout << "prev mem_fetch " << diff_sm_mfs[i]->get_request_uid() << " has same content as mf " << new_mf->get_request_uid() << std::endl;
      break;
    } else {
      std::cout << "prev mem_fetch " << diff_sm_mfs[i]->get_request_uid() << " has diff content as mf " << new_mf->get_request_uid() << std::endl;
    }
  }

  if(mf_index >= 0) {
    mem_fetch* prev_mf = diff_sm_mfs[mf_index];
    unsigned prev_rec_pkt_bits = prev_mf->get_rec_comp_merged_pkt_bits();
    unsigned prev_comp_pkt_bits = prev_mf->get_comp_data_bits();
    prev_comp_pkt_bits += (prev_mf->get_ctrl_size()*8);
    unsigned cur_comp_pkt_bits = 2;	//2bits for indexing data, new_mf->get_comp_data_bits();
    cur_comp_pkt_bits += ((new_mf->get_ctrl_size()*8)/2);		//address only added, packet header reduced

    unsigned total_pkt_bits = (prev_rec_pkt_bits+prev_comp_pkt_bits+cur_comp_pkt_bits);
    unsigned comp_byte_size = (unsigned) ceil(total_pkt_bits/8.0);

    if(comp_byte_size > m_max_comp_data_size) {
      ret = new_mf;
      //m_rec_comp_buffer->push_mem_fetch(new_mf);
      //new_mf->set_rec_comp_merged_pkt_bits(0);
      //new_mf->set_rec_comp_data_bits(0);
      REC_COMP_DEBUG_PRINT("\t%llu | new mf %u is pushed into rec comp buffer due to not enough space\n", (gpu_sim_cycle+gpu_tot_sim_cycle), new_mf->get_request_uid());
    } else {
      merge_mf(new_mf, prev_mf);
      //std::cout << "new_mf " << new_mf->get_request_uid() << " is merged to prev_mf " << prev_mf->get_request_uid() << std::endl;
      REC_COMP_DEBUG_PRINT("\t%llu | new mf %u is merged to prev_mf %u\n", (gpu_sim_cycle+gpu_tot_sim_cycle), new_mf->get_request_uid(), prev_mf->get_request_uid());
      prev_mf->add_rec_comp_merged_pkt_bits(2);
      new_mf->set_rec_comp_data_bits(2);
      /*
      printf("new_mf %u ", new_mf->get_request_uid());
      new_mf->print_real_data(8);
      printf("prev_mf %u ", prev_mf->get_request_uid());
      prev_mf->print_real_data(8);
      */
    }

  } else {
    ret = new_mf;

    //m_rec_comp_buffer->push_mem_fetch(new_mf);
    //new_mf->set_rec_comp_merged_pkt_bits(0);
    //new_mf->set_rec_comp_data_bits(0);
  }

  return ret;

}


void hpcl_rec_comp_lwm_pl_proc::rec_comp_algo2_proc(mem_fetch* new_mf)
{
  //printf("rec_comp_algo2_proc starts\n");

  mem_fetch* unmerged_mf = rec_comp_method_for_inter_sm(new_mf);
  if(unmerged_mf) {
    m_rec_comp_buffer->push_mem_fetch(new_mf);
    new_mf->set_rec_comp_merged_pkt_bits(0);
    new_mf->set_rec_comp_data_bits(0);
  }

  //printf("rec_comp_algo2_proc ends\n");
}

void hpcl_rec_comp_lwm_pl_proc::rec_comp_algo3_proc(mem_fetch* new_mf)
{
  int comp_res = new_mf->get_comp_res();
  int target_sm = new_mf->get_tpc();

  mem_fetch* prev_mf = m_rec_comp_buffer->get_mem_fetch_to_same_sm(target_sm);
  //prev_mf has not been used for merging another reply to different SM
  if(prev_mf) {
    mem_fetch* unmerged_mf = rec_comp_method_for_intra_sm(new_mf, prev_mf);
    //assert(!unmerged_mf);
    if(unmerged_mf) {
      m_rec_comp_buffer->push_mem_fetch(new_mf);
      new_mf->set_rec_comp_merged_pkt_bits(0);
      new_mf->set_rec_comp_data_bits(0);
    } else {

    }
  } else {
    rec_comp_algo2_proc(new_mf);
  }
}

void hpcl_rec_comp_lwm_pl_proc::rec_comp_algo4_proc(mem_fetch* new_mf)
{
  mem_fetch* unmerged_mf = rec_comp_method_for_inter_sm(new_mf);
  if(unmerged_mf) {
    rec_comp_algo1_proc(unmerged_mf);
  }
}




void hpcl_rec_comp_lwm_pl_proc::run()
{
  mem_fetch* new_mf = m_input_link->m_data;

  //Reset output data
  m_output_link->m_data = NULL;

  if(new_mf)
  {
    if(new_mf->get_real_data_size() > 0)		//read reply mf
    {
      /*
      if(g_hpcl_comp_config.hpcl_rec_comp_algo == hpcl_comp_config::BASIC_APPEND)
	rec_comp_test_proc(new_mf);
      else if(g_hpcl_comp_config.hpcl_rec_comp_algo == hpcl_comp_config::INTER_COMP)
	rec_comp_test_proc(new_mf);
      else if(g_hpcl_comp_config.hpcl_rec_comp_algo == hpcl_comp_config::REC_COMP_ALGO1)
	//rec_comp_algo_proc(new_mf);
      */

      if(g_hpcl_comp_config.hpcl_rec_comp_algo == hpcl_comp_config::INTER_COMP_SAME_SM_PKT)
	rec_comp_algo1_proc(new_mf);
      else if(g_hpcl_comp_config.hpcl_rec_comp_algo == hpcl_comp_config::INTER_COMP_DIFF_SM_PKT)
      	rec_comp_algo2_proc(new_mf);
      else if(g_hpcl_comp_config.hpcl_rec_comp_algo == hpcl_comp_config::INTER_COMP_SAME_DIFF_SM_PKT)
	rec_comp_algo3_proc(new_mf);
      else if(g_hpcl_comp_config.hpcl_rec_comp_algo == hpcl_comp_config::INTER_COMP_DIFF_SAME_SM_PKT)
      	rec_comp_algo4_proc(new_mf);
      else assert(0);

    } else {
      m_rec_comp_buffer->push_mem_fetch(new_mf);
    }
  }

  /*
  //Reset output data
  m_output_link->m_data = NULL;
  if(new_mf && new_mf->get_real_data_size() > 0) {
    if(g_hpcl_comp_config.hpcl_inter_comp_algo == hpcl_comp_config::BASIC_APPEND) {
      rec_append_method(new_mf, prev_mf);
    }
    //m_output_link->m_data1 = mf;
  }
  */

#ifdef old
  //added by kh(042416)
  if(g_hpcl_comp_config.hpcl_inter_rep_comp_en == 1) {
    //added by kh(042316)
    //For merging read replies
    if(mf && mf->get_real_data_size() > 0) {

	if(g_hpcl_comp_config.hpcl_inter_comp_algo == hpcl_comp_config::BASIC_APPEND) {

	  //Step0: mf may not be popped in the previous iteration, clear previous merged mfs
	  mf->del_merged_mfs();

	  //added by kh(060416)
	  bool recursive_comp_enable = g_hpcl_comp_config.is_rec_comp_avail(mf->get_comp_res());
	  //

	  if(recursive_comp_enable == true) {

	    //Step1: choose mem_fetchs to be merged
	    unsigned merged_reply_size = mf->get_ctrl_size()+mf->get_comp_data_size();
	    std::vector<mem_fetch*>& all_waiting_mfs = m_hpcl_comp_buffer[i]->get_all_waiting_mfs (mf);
	    std::vector<mem_fetch*> merged_mfs;
	    for(unsigned j = 0; j < all_waiting_mfs.size() && all_waiting_mfs.size() > 1; j++) {
	      //deleted by kh(060316)
	      //if(all_waiting_mfs[j] != mf && all_waiting_mfs[j]->get_real_data_size() > 0) {
	      //if(all_waiting_mfs[j] != mf
	      //	  && all_waiting_mfs[j]->get_real_data_size() > 0
	      // 	  && g_hpcl_comp_config.is_rec_comp_avail(all_waiting_mfs[j]->get_comp_res()) )
	      //added by kh(060516)
	      if(all_waiting_mfs[j] != mf
	      	  && all_waiting_mfs[j]->get_real_data_size() > 0
	       	  && all_waiting_mfs[j]->get_comp_res() == mf->get_comp_res())
	      {
		unsigned old_merged_reply_size = merged_reply_size;
		unsigned merged_data_size = (all_waiting_mfs[j]->get_ctrl_size()+all_waiting_mfs[j]->get_comp_data_size());
		merged_reply_size += merged_data_size;
		//std::cout << " merged_data_size " << merged_data_size << " merged_reply_size " << merged_reply_size << std::endl;
		if(merged_reply_size <= 32*g_NI_input_buffer_size) {
		  merged_mfs.push_back(all_waiting_mfs[j]);
		} else {
		  break;
		}
	      }
	    }

	    //Step2: add merged mem_fetch
	    mf->add_merged_mfs (merged_mfs);
	  }

	} else if(g_hpcl_comp_config.hpcl_inter_comp_algo == hpcl_comp_config::INTER_COMP
	    || g_hpcl_comp_config.hpcl_inter_comp_algo == hpcl_comp_config::INTER_COMP_FB)
	  {


	    }
	}
    }
#endif
}

//mem_fetch* hpcl_rec_comp_lwm_pl_proc::pop_output_mf()
//{
  //return m_hpcl_intra_coal_buffer->pop_mem_fetch();
  /*
  mem_fetch* mf = m_hpcl_intra_coal_buffer->pop_mem_fetch();
  if(mf) {
    std::vector<mem_fetch*>& other_mfs = mf->get_intra_coalesced_read_reply_mf();
    m_hpcl_intra_coal_buffer->delete_mfs(other_mfs);
  }
  return mf;
  */
//}
/*
void hpcl_rec_comp_lwm_pl_proc::create(unsigned sm_no, unsigned max_intra_coal_size)
{

  m_hpcl_intra_coal_buffer = new hpcl_intra_coal_buffer;
  m_hpcl_intra_coal_buffer->create(g_hpcl_coalescing_config.hpcl_read_reply_intra_coalescing_buffer_size, sm_no);
  m_max_intra_coal_size = max_intra_coal_size;
  //printf("m_max_intra_coal_size : %u\n", m_max_intra_coal_size);
}
 *
   */

//mem_fetch* hpcl_rec_comp_lwm_pl_proc::top_output_mf()
//{
  //return m_hpcl_intra_coal_buffer->top_mem_fetch();

  /*
  mem_fetch* mf = m_hpcl_intra_coal_buffer->top_mem_fetch();
  if(mf && !mf->get_is_write()) {

    //Step1: clean previously coalesced mfs.
    mf->clean_intra_coalesced_read_reply_mf();

    //Step2: add mfs destined to the same SM
    unsigned response_size = mf->get_is_write()?mf->get_ctrl_size():mf->size();
    std::vector<mem_fetch*>& other_mfs = m_hpcl_intra_coal_buffer->get_all_waiting_mfs(mf);
    for(unsigned i = 0; i < other_mfs.size(); i++) {
      if(!other_mfs[i]->get_is_write() && other_mfs[i] != mf) {
	response_size += other_mfs[i]->get_data_size();
	response_size += 4;	//4B address only.

	if(response_size <= m_max_intra_coal_size) {
	  mf->push_intra_coalesced_read_reply_mf(other_mfs[i]);
	  INTRACOAL_INTERCOMP_DEBUG_PRINT("mf %u is coalesced into mf %u\n", other_mfs[i]->get_request_uid(), mf->get_request_uid());
	} else {
	  break;
	}
      }
    }

  }

  return mf;
  */
//}


//bool hpcl_rec_comp_lwm_pl_proc::has_buffer_space()
//{
  //return m_hpcl_intra_coal_buffer->has_comp_buffer_space();
//}

void hpcl_rec_comp_lwm_pl_proc::print()
{
  //m_hpcl_intra_coal_buffer->print();
}


void hpcl_rec_comp_lwm_pl_proc::create(unsigned pipeline_stage_index, unsigned max_comp_data_size, hpcl_rec_comp_buffer* rec_comp_buffer, unsigned par_id)
{
  m_max_comp_data_size = max_comp_data_size;
  m_pipeline_stage_index = pipeline_stage_index;
  m_input_link = new pl_proc_data_link;
  m_output_link = NULL;
  m_rec_comp_buffer = rec_comp_buffer;
  m_par_id = par_id;
  //printf("max_comp_data_size %u \n", max_comp_data_size);
  //assert(0);
}

pl_proc_data_link* hpcl_rec_comp_lwm_pl_proc::get_input_link()
{
  return m_input_link;
}

void hpcl_rec_comp_lwm_pl_proc::set_output_link(pl_proc_data_link* data_link)
{
  m_output_link = data_link;
}

unsigned hpcl_rec_comp_lwm_pl_proc::sub_rec_lwm_method(mem_fetch* new_mf, mem_fetch* prev_mf)
{
  unsigned matching_word_no = 0;
  unsigned comp_res = new_mf->get_comp_res();

  hpcl_dict<unsigned short>* new_loc_dict_2B = NULL;
  hpcl_dict<unsigned int>* new_loc_dict_4B = NULL;
  hpcl_dict<unsigned long long>* new_loc_dict_8B = NULL;

  if(comp_res == 2) 	 new_loc_dict_2B = (hpcl_dict<unsigned short>*) new_mf->get_loc_dict(comp_res);
  else if(comp_res == 4) new_loc_dict_4B = (hpcl_dict<unsigned int>*) new_mf->get_loc_dict(comp_res);
  else if(comp_res == 8) new_loc_dict_8B = (hpcl_dict<unsigned long long>*) new_mf->get_loc_dict(comp_res);
  else			 assert(0);

  hpcl_dict<unsigned short>* prev_loc_dict_2B = NULL;
  hpcl_dict<unsigned int>* prev_loc_dict_4B = NULL;
  hpcl_dict<unsigned long long>* prev_loc_dict_8B = NULL;

  if(comp_res == 2) 	 prev_loc_dict_2B = (hpcl_dict<unsigned short>*) prev_mf->get_loc_dict(comp_res);
  else if(comp_res == 4) prev_loc_dict_4B = (hpcl_dict<unsigned int>*) prev_mf->get_loc_dict(comp_res);
  else if(comp_res == 8) prev_loc_dict_8B = (hpcl_dict<unsigned long long>*) prev_mf->get_loc_dict(comp_res);
  else			 assert(0);

//  if(comp_res == 2)		new_loc_dict_2B->compare_dict(prev_loc_dict_2B, matching_word_no);
//  else if(comp_res == 4)	new_loc_dict_4B->compare_dict(prev_loc_dict_4B, matching_word_no);
//  else if(comp_res == 8)	new_loc_dict_8B->compare_dict(prev_loc_dict_8B, matching_word_no);
//  else	assert(0);

  if(comp_res == 2)		prev_loc_dict_2B->compare_dict(new_loc_dict_2B, matching_word_no);
  else if(comp_res == 4)	prev_loc_dict_4B->compare_dict(new_loc_dict_4B, matching_word_no);
  else if(comp_res == 8)	prev_loc_dict_8B->compare_dict(new_loc_dict_8B, matching_word_no);
  else	assert(0);

  #ifdef REC_ANAL_COMP_DEBUG
  REC_ANAL_COMP_DEBUG_PRINT("sub_rec_lwm_method: new_mf %u, prev_mf %u, matching_word %u\n",
	new_mf->get_request_uid(), prev_mf->get_request_uid(), matching_word_no);
  if(comp_res == 2) {
    REC_ANAL_COMP_DEBUG_PRINT("new_loc_dict_2B ...\n");
    new_loc_dict_2B->print();
    REC_ANAL_COMP_DEBUG_PRINT("prev_loc_dict_2B ...\n");
    prev_loc_dict_2B->print();
  } else if(comp_res == 4) {
    REC_ANAL_COMP_DEBUG_PRINT("new_loc_dict_4B ...\n");
    new_loc_dict_4B->print();
    REC_ANAL_COMP_DEBUG_PRINT("prev_loc_dict_4B ...\n");
    prev_loc_dict_4B->print();
  } else if(comp_res == 8) {
    REC_ANAL_COMP_DEBUG_PRINT("new_loc_dict_8B ...\n");
    new_loc_dict_8B->print();
    REC_ANAL_COMP_DEBUG_PRINT("prev_loc_dict_8B ...\n");
    prev_loc_dict_8B->print();
  }
  #endif

  //REC_COMP_DEBUG_PRINT("sub_rec_lwm_method: new_mf %u, prev_mf %u, matching_word_no %u\n", new_mf->get_request_uid(), prev_mf->get_request_uid(), matching_word_no);

  return matching_word_no;
}


mem_fetch* hpcl_rec_comp_lwm_pl_proc::rec_lwm_method(mem_fetch* new_mf, mem_fetch* prev_mf, bool max_pkt_turnon, enum rec_lwm_mode mode)
{
  mem_fetch* ret = NULL;

  //assume that prev_mf should have the same comp resolution as new_mf
  unsigned comp_res = new_mf->get_comp_res();

  unsigned prev_rec_pkt_bits = prev_mf->get_rec_comp_merged_pkt_bits();
  unsigned prev_comp_pkt_bits = prev_mf->get_comp_data_bits();
  prev_comp_pkt_bits += (prev_mf->get_ctrl_size()*8);

  unsigned exp_final_pkt_bits = prev_mf->get_rec_comp_merged_pkt_bits();
  exp_final_pkt_bits += prev_mf->get_comp_data_bits();
  exp_final_pkt_bits += (prev_mf->get_ctrl_size()*8);
  REC_COMP_DEBUG_PRINT("\t%llu | prev_mf %u has %u bits\n", (gpu_sim_cycle+gpu_tot_sim_cycle), prev_mf->get_request_uid(), exp_final_pkt_bits);
  exp_final_pkt_bits += new_mf->get_comp_data_bits();
  exp_final_pkt_bits += (new_mf->get_ctrl_size()*8);
  unsigned comp_byte_size = (unsigned) ceil(exp_final_pkt_bits/8.0);
  REC_COMP_DEBUG_PRINT("\t%llu | prev_mf %u has %u exp_final_bits %u byte\n", (gpu_sim_cycle+gpu_tot_sim_cycle), prev_mf->get_request_uid(), exp_final_pkt_bits, comp_byte_size);

  if(comp_byte_size > m_max_comp_data_size) {
    ret = new_mf;
    REC_COMP_DEBUG_PRINT("\t%llu | new mf %u is pushed into rec comp buffer due to not enough space\n", (gpu_sim_cycle+gpu_tot_sim_cycle), new_mf->get_request_uid());
    return ret;
  }

  if(max_pkt_turnon == true) {
    //added by kh(062916)
    if((prev_mf->get_merged_mf_no()+prev_mf->get_merged_mf_to_same_SM_no()+1) >= g_hpcl_comp_config.hpcl_rec_comp_max_pkt_no) {
      ret = new_mf;
      REC_COMP_DEBUG_PRINT("\t%llu | new mf %u is pushed into rec comp buffer due to max recursive comp pkt no %u\n",
	  (gpu_sim_cycle+gpu_tot_sim_cycle), new_mf->get_request_uid(), g_hpcl_comp_config.hpcl_rec_comp_max_pkt_no);
      return ret;
    }
    ///
  }
  //Recursive Compression Simulation
  //Words in new_mf is compared against words in prev_mfs.
  //If the matching word is found, then set it to found. This avoids redundant matchings across differe prev mfs

  assert(comp_res == prev_mf->get_comp_res());
  //Step1: Recursive compression against the oldest one;
  unsigned total_matching_word_no = sub_rec_lwm_method(new_mf, prev_mf);


  //Step2: Recursive compression against others
  if(mode == REC_COMP_INTRA_SM || mode == REC_COMP_ALL_SM) {
    for(unsigned i = 0; i < prev_mf->get_merged_mf_to_same_SM_no(); i++) {
      assert(comp_res == prev_mf->get_merged_mf_to_same_SM(i)->get_comp_res());
      total_matching_word_no += sub_rec_lwm_method(new_mf, prev_mf->get_merged_mf_to_same_SM(i));

      //added by kh(063016)
      //Maximum recursive window is set to 2
      if(i == 0) {
        break;
      }
      ///
    }
  }

  if(mode == REC_COMP_INTER_SM || mode == REC_COMP_ALL_SM) {
    for(unsigned i = 0; i < prev_mf->get_merged_mf_no(); i++) {
      assert(comp_res == prev_mf->get_merged_mf(i)->get_comp_res());
      total_matching_word_no += sub_rec_lwm_method(new_mf, prev_mf->get_merged_mf(i));

      //added by kh(063016)
      //Maximum recursive window is set to 2
      if(i == 0) {
        break;
      }
      ///
    }
  }

  //TO-DO-LIST
  //Encoding Status bits increase according to the size of recursive compression window
  unsigned data_ptr_bits = (unsigned) ceil(log2((double)new_mf->get_real_data_size()/comp_res));
  unsigned org_comp_data_bits = new_mf->get_comp_data_bits();
  unsigned rec_comp_data_bits = new_mf->get_comp_data_bits();

  REC_ANAL_COMP_DEBUG_PRINT("new_mf %u (SM %u Addr %llu), prev_mf %u (SM %u Addr %llu)\n",
		 new_mf->get_request_uid(), new_mf->get_tpc(), g_L2_config.block_addr(new_mf->get_addr()),
		 prev_mf->get_request_uid(), prev_mf->get_tpc(), g_L2_config.block_addr(prev_mf->get_addr()));

  REC_ANAL_COMP_DEBUG_PRINT("\tsame_word_no : %u, comp_res : %u\n", total_matching_word_no, comp_res);
  REC_ANAL_COMP_DEBUG_PRINT("\tStep1: rec_comp_data_bits %u,", rec_comp_data_bits);
  rec_comp_data_bits -= (total_matching_word_no*comp_res*8);
  REC_ANAL_COMP_DEBUG_PRINT("  Step2: rec_comp_data_bits %u,", rec_comp_data_bits);
  rec_comp_data_bits += (total_matching_word_no*data_ptr_bits);
  REC_ANAL_COMP_DEBUG_PRINT("  Step3: rec_comp_data_bits %u, ", rec_comp_data_bits);
  //extend the encoding status bits from 1-bit to 2-bit
  unsigned _word_no = new_mf->get_real_data_size()/comp_res;
  rec_comp_data_bits += _word_no;	//extra encoding bits
  REC_ANAL_COMP_DEBUG_PRINT("  Step4: rec_comp_data_bits %u, ", rec_comp_data_bits);
  REC_ANAL_COMP_DEBUG_PRINT("  Final: rec_comp_data_bits %u\n", rec_comp_data_bits);

  if(rec_comp_data_bits >= org_comp_data_bits) {
    REC_ANAL_COMP_DEBUG_PRINT("\tNo Data Reduction after Rec Comp: org_comp_data_bits %u rec_comp_data_bits %u\n", org_comp_data_bits, rec_comp_data_bits);
    rec_comp_data_bits = org_comp_data_bits;


  } else {
    REC_ANAL_COMP_DEBUG_PRINT("\tYes Data Reduction after Rec Comp: org_comp_data_bits %u rec_comp_data_bits %u\n", org_comp_data_bits, rec_comp_data_bits);
  }

  //Debugging
/*
  hpcl_dict<unsigned short>* loc_dict_2B_1 = NULL;
  hpcl_dict<unsigned int>* loc_dict_4B_1 = NULL;
  hpcl_dict<unsigned long long>* loc_dict_8B_1 = NULL;
  if(comp_res == 2)		loc_dict_2B_1 = (hpcl_dict<unsigned short>*) new_mf->get_loc_dict(2);
  else if(comp_res == 4)	loc_dict_4B_1 = (hpcl_dict<unsigned int>*) new_mf->get_loc_dict(4);
  else if(comp_res == 8)	loc_dict_8B_1 = (hpcl_dict<unsigned long long>*) new_mf->get_loc_dict(8);

  hpcl_dict<unsigned short>* loc_dict_2B_2 = NULL;
  hpcl_dict<unsigned int>* loc_dict_4B_2 = NULL;
  hpcl_dict<unsigned long long>* loc_dict_8B_2 = NULL;
  if(comp_res == 2)		loc_dict_2B_2 = (hpcl_dict<unsigned short>*) prev_mf->get_loc_dict(2);
  else if(comp_res == 4)	loc_dict_4B_2 = (hpcl_dict<unsigned int>*) prev_mf->get_loc_dict(4);
  else if(comp_res == 8)	loc_dict_8B_2 = (hpcl_dict<unsigned long long>*) prev_mf->get_loc_dict(8);

  if(loc_dict_2B_1)	loc_dict_2B_1->print();
  if(loc_dict_4B_1)	loc_dict_4B_1->print();
  if(loc_dict_8B_1)	loc_dict_8B_1->print();

  if(loc_dict_2B_2)	loc_dict_2B_2->print();
  if(loc_dict_4B_2)	loc_dict_4B_2->print();
  if(loc_dict_8B_2)	loc_dict_8B_2->print();
*/

  //Check if the total size after recursive compression is fit in the maximum size
  unsigned final_pkt_bits = (prev_rec_pkt_bits+prev_comp_pkt_bits);
  unsigned merged_pkt_bits = rec_comp_data_bits;
  merged_pkt_bits += new_mf->get_ctrl_size()*8;

  final_pkt_bits += merged_pkt_bits;
  ///

  unsigned final_comp_byte_size = (unsigned) ceil(final_pkt_bits/8.0);

  if(final_comp_byte_size > m_max_comp_data_size) {
    REC_ANAL_COMP_DEBUG_PRINT("Error!: final_comp_byte_size %u, m_max_comp_data_size %u\n", final_comp_byte_size, m_max_comp_data_size);
  }

  assert(final_comp_byte_size <= m_max_comp_data_size);

  //prev_mf->add_merged_mf(new_mf);
  merge_mf(new_mf, prev_mf);
  prev_mf->add_rec_comp_merged_pkt_bits(merged_pkt_bits);
  new_mf->add_rec_comp_data_bits(rec_comp_data_bits);

  return NULL;
}

void hpcl_rec_comp_lwm_pl_proc::clear_rec_comp_info(mem_fetch* new_mf)
{
  unsigned comp_res = new_mf->get_comp_res();
  hpcl_dict<unsigned short>* loc_dict_2B = NULL;
  hpcl_dict<unsigned int>* loc_dict_4B = NULL;
  hpcl_dict<unsigned long long>* loc_dict_8B = NULL;
  //std::vector<mem_fetch*>& merged_mfs = mf->get_merged_mfs();
  //for(unsigned j = 0; j < merged_mfs.size(); j++) {
  if(comp_res == 2) {
    loc_dict_2B = (hpcl_dict<unsigned short>*) new_mf->get_loc_dict(comp_res);
    unsigned word_no = loc_dict_2B->get_word_no();
    for(unsigned k = 0; k < word_no; k++)	loc_dict_2B->clear_found_flag(k);
  } else if(comp_res == 4) {
    loc_dict_4B = (hpcl_dict<unsigned int>*) new_mf->get_loc_dict(comp_res);
    unsigned word_no = loc_dict_4B->get_word_no();
    for(unsigned k = 0; k < word_no; k++)	loc_dict_4B->clear_found_flag(k);
  } else if(comp_res == 8) {
    loc_dict_8B = (hpcl_dict<unsigned long long>*) new_mf->get_loc_dict(comp_res);
    unsigned word_no = loc_dict_8B->get_word_no();
    for(unsigned k = 0; k < word_no; k++)	loc_dict_8B->clear_found_flag(k);
  } else assert(0);
  //}

}

int hpcl_rec_comp_lwm_pl_proc::speculate_rec_lwm_method(mem_fetch* new_mf, mem_fetch* prev_mf)
{
  int ret = -1;

  //Step1: clear all found tag in a local dictionary
  clear_rec_comp_info(new_mf);

  //assume that prev_mf should have the same comp resolution as new_mf
  unsigned comp_res = new_mf->get_comp_res();
  unsigned prev_rec_pkt_bits = prev_mf->get_rec_comp_merged_pkt_bits();
  unsigned prev_comp_pkt_bits = prev_mf->get_comp_data_bits();
  prev_comp_pkt_bits += (prev_mf->get_ctrl_size()*8);

  //Recursive Compression Simulation
  //Words in new_mf is compared against words in prev_mfs.
  //If the matching word is found, then set it to found. This avoids redundant matchings across differe prev mfs
  assert(comp_res == prev_mf->get_comp_res());
  //Step1: Recursive compression against the oldest one;
  unsigned total_matching_word_no = sub_rec_lwm_method(new_mf, prev_mf);
  //added by kh(063016)
  //Step2: Recursive compression against others
  for(unsigned i = 0; i < prev_mf->get_merged_mf_no(); i++) {
    assert(comp_res == prev_mf->get_merged_mf(i)->get_comp_res());
    total_matching_word_no += sub_rec_lwm_method(new_mf, prev_mf->get_merged_mf(i));

    //added by kh(063016)
    //Maximum recursive window is set to 2
    if(i == 0) {
      break;
    }
    ///
  }
//  for(unsigned i = 0; i < prev_mf->get_merged_mf_to_same_SM_no(); i++) {
//      assert(comp_res == prev_mf->get_merged_mf_to_same_SM(i)->get_comp_res());
//      total_matching_word_no += sub_rec_lwm_method(new_mf, prev_mf->get_merged_mf_to_same_SM(i));
//  }

  //TO-DO-LIST
  //Encoding Status bits increase according to the size of recursive compression window
  unsigned data_ptr_bits = (unsigned) ceil(log2((double)new_mf->get_real_data_size()/comp_res));
  unsigned org_comp_data_bits = new_mf->get_comp_data_bits();
  unsigned rec_comp_data_bits = new_mf->get_comp_data_bits();

  REC_ANAL_COMP_DEBUG_PRINT("speculate : new_mf %u (SM %u Addr %llu), prev_mf %u (SM %u Addr %llu)\n",
		 new_mf->get_request_uid(), new_mf->get_tpc(), g_L2_config.block_addr(new_mf->get_addr()),
		 prev_mf->get_request_uid(), prev_mf->get_tpc(), g_L2_config.block_addr(prev_mf->get_addr()));

  REC_ANAL_COMP_DEBUG_PRINT("\tsame_word_no : %u, comp_res : %u\n", total_matching_word_no, comp_res);
  REC_ANAL_COMP_DEBUG_PRINT("\tStep1: rec_comp_data_bits %u,", rec_comp_data_bits);
  rec_comp_data_bits -= (total_matching_word_no*comp_res*8);
  REC_ANAL_COMP_DEBUG_PRINT("\tStep2: rec_comp_data_bits %u,", rec_comp_data_bits);
  rec_comp_data_bits += (total_matching_word_no*data_ptr_bits);
  REC_ANAL_COMP_DEBUG_PRINT("\tStep3: rec_comp_data_bits %u, ", rec_comp_data_bits);
  //extend the encoding status bits from 1-bit to 2-bit
  unsigned _word_no = new_mf->get_real_data_size()/comp_res;
  rec_comp_data_bits += _word_no;	//extra encoding bits
  REC_ANAL_COMP_DEBUG_PRINT("  Step4: rec_comp_data_bits %u, ", rec_comp_data_bits);
  REC_ANAL_COMP_DEBUG_PRINT("\tFinal: rec_comp_data_bits %u\n", rec_comp_data_bits);

  if(rec_comp_data_bits >= org_comp_data_bits) {
    REC_ANAL_COMP_DEBUG_PRINT("\tNo Data Reduction after Rec Comp: org_comp_data_bits %u rec_comp_data_bits %u\n", org_comp_data_bits, rec_comp_data_bits);
  } else {
    REC_ANAL_COMP_DEBUG_PRINT("\tYes Data Reduction after Rec Comp: org_comp_data_bits %u rec_comp_data_bits %u\n", org_comp_data_bits, rec_comp_data_bits);
  }

  unsigned prev_pkt_bits = prev_mf->get_comp_data_bits();
  prev_pkt_bits += (prev_mf->get_ctrl_size()*8);
  unsigned new_pkt_bits = new_mf->get_comp_data_bits();
  new_pkt_bits += (new_mf->get_ctrl_size()*8);

  unsigned org_flit_no = (unsigned) ceil(prev_pkt_bits/8.0/32.0) + (unsigned) ceil(new_pkt_bits/8.0/32.0);

  REC_ANAL_COMP_DEBUG_PRINT("prev_pkt_bits %u new_pkt_bits %u\n", prev_pkt_bits, new_pkt_bits);


  unsigned merged_pkt_bits = prev_comp_pkt_bits+rec_comp_data_bits+64;
  unsigned new_flit_no = (unsigned) ceil(merged_pkt_bits/8.0/32.0);
  new_flit_no = new_flit_no*2;

  REC_ANAL_COMP_DEBUG_PRINT("\torg_flit_no %u new_flit_no %u\n", org_flit_no,new_flit_no);

  if(org_flit_no == new_flit_no) {

      REC_ANAL_COMP_DEBUG_PRINT("\tINTER: mf %u (SM %u) can be merged to mf %u (SM %u)\n", new_mf->get_request_uid(), new_mf->get_tpc(), prev_mf->get_request_uid(), prev_mf->get_tpc());

      #ifdef REC_ANAL_COMP_DEBUG
      hpcl_dict<unsigned short>* loc_dict_2B_1 = NULL;
      hpcl_dict<unsigned int>* loc_dict_4B_1 = NULL;
      hpcl_dict<unsigned long long>* loc_dict_8B_1 = NULL;
      if(comp_res == 2)		loc_dict_2B_1 = (hpcl_dict<unsigned short>*) new_mf->get_loc_dict(2);
      else if(comp_res == 4)	loc_dict_4B_1 = (hpcl_dict<unsigned int>*) new_mf->get_loc_dict(4);
      else if(comp_res == 8)	loc_dict_8B_1 = (hpcl_dict<unsigned long long>*) new_mf->get_loc_dict(8);

      hpcl_dict<unsigned short>* loc_dict_2B_2 = NULL;
      hpcl_dict<unsigned int>* loc_dict_4B_2 = NULL;
      hpcl_dict<unsigned long long>* loc_dict_8B_2 = NULL;
      if(comp_res == 2)		loc_dict_2B_2 = (hpcl_dict<unsigned short>*) prev_mf->get_loc_dict(2);
      else if(comp_res == 4)	loc_dict_4B_2 = (hpcl_dict<unsigned int>*) prev_mf->get_loc_dict(4);
      else if(comp_res == 8)	loc_dict_8B_2 = (hpcl_dict<unsigned long long>*) prev_mf->get_loc_dict(8);

      if(loc_dict_2B_1)	loc_dict_2B_1->print();
      if(loc_dict_4B_1)	loc_dict_4B_1->print();
      if(loc_dict_8B_1)	loc_dict_8B_1->print();

      if(loc_dict_2B_2)	loc_dict_2B_2->print();
      if(loc_dict_4B_2)	loc_dict_4B_2->print();
      if(loc_dict_8B_2)	loc_dict_8B_2->print();
      #endif

  }

  ret = new_flit_no-org_flit_no;


  //Step1: clear all found tag in a local dictionary
  clear_rec_comp_info(new_mf);

  return ret;
}


mem_fetch* hpcl_rec_comp_lwm_pl_proc::rec_comp_method_for_intra_sm(mem_fetch* new_mf, mem_fetch* prev_mf)
{
  mem_fetch* ret = NULL;

  //assume that prev_mf should have the same comp resolution as new_mf
  unsigned comp_res = new_mf->get_comp_res();

  unsigned prev_rec_pkt_bits = prev_mf->get_rec_comp_merged_pkt_bits();
  unsigned prev_comp_pkt_bits = prev_mf->get_comp_data_bits();
  prev_comp_pkt_bits += (prev_mf->get_ctrl_size()*8);
  unsigned cur_comp_pkt_bits = new_mf->get_comp_data_bits();
  cur_comp_pkt_bits += ((new_mf->get_ctrl_size()*8)/2);		//address only added, packet header reduced

  //if two mfs have the same contents, is_same is zero
  int is_same = g_hpcl_comp->compare_data(prev_mf, new_mf);
  if(is_same < 0) {
    for(unsigned j = 0; j < prev_mf->get_merged_mf_to_same_SM_no(); j++) {
      mem_fetch* merged_mf = prev_mf->get_merged_mf_to_same_SM(j);
      is_same = g_hpcl_comp->compare_data(merged_mf, new_mf);
      if(is_same == 0) break;
    }
  }
  if(is_same == 0) {
    cur_comp_pkt_bits = ((new_mf->get_ctrl_size()*8)/2);

    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::REC_COMP_INTRA_SM_SAME_DATA_MERGE_NO, 1);
    printf("new_mf %u has the same contents\n", new_mf->get_request_uid());
  }

  unsigned total_pkt_bits = (prev_rec_pkt_bits+prev_comp_pkt_bits+cur_comp_pkt_bits);
  unsigned comp_byte_size = (unsigned) ceil(total_pkt_bits/8.0);

  if(comp_byte_size > m_max_comp_data_size) {
    ret = new_mf;
    REC_COMP_DEBUG_PRINT("\t%llu | new mf %u is pushed into rec comp buffer due to not enough space\n", (gpu_sim_cycle+gpu_tot_sim_cycle), new_mf->get_request_uid());
    return ret;
  } else {
    merge_mf(new_mf, prev_mf);
    //std::cout << "new_mf " << new_mf->get_request_uid() << " is merged to prev_mf " << prev_mf->get_request_uid() << std::endl;
    REC_COMP_DEBUG_PRINT("\t%llu | new mf %u is merged to prev_mf %u\n", (gpu_sim_cycle+gpu_tot_sim_cycle), new_mf->get_request_uid(), prev_mf->get_request_uid());
    prev_mf->add_rec_comp_merged_pkt_bits(cur_comp_pkt_bits);

    if(is_same == 0)	new_mf->set_rec_comp_data_bits(1);
    else 		new_mf->set_rec_comp_data_bits(new_mf->get_comp_data_bits());
  }

  return ret;
}


void hpcl_rec_comp_lwm_pl_proc::set_comp_buffer(hpcl_comp_buffer* comp_buffer)
{
  m_comp_buffer = comp_buffer;
}
//

