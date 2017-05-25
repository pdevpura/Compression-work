/*
 * NetworkStat.cpp
 *
 *  Created on: Sep 19, 2015
 *      Author: mumichang
 */


#include "hpcl_comp_buffer.h"

#include <vector>
#include <cassert>
#include <algorithm>    // std::find

//added by kh(052016)
#include "hpcl_comp_config.h"
extern hpcl_comp_config g_hpcl_comp_config;
///

void hpcl_comp_buffer::create(unsigned comp_buffer_size, unsigned subbuffer_no) {
  m_comp_buffer_size = comp_buffer_size;

  //added by kh(042116)
  //std::map<unsigned, mem_fetch*> tmp;
  std::vector<mem_fetch*> tmp;
  for(int i = 0; i < subbuffer_no; i++) {
      m_comp_subbuffer.push_back(tmp);
  }
  ///

  #ifdef old
  //added by kh(042316)
  m_dict_for_inter_read_replies = new hpcl_dict<unsigned char>(1024, HPCL_LFU);
  m_dict_for_inter_read_replies->clear();
  ///
  #endif

}

void hpcl_comp_buffer::push_mem_fetch(mem_fetch* mf) {

//  if(mf)	std::cout << "hpcl_comp_buffer::push_mem_fetch mf " << mf->get_request_uid() << std::endl;
//  else		std::cout << "hpcl_comp_buffer::push_mem_fetch mf " << "NULL" << std::endl;

  m_comp_buffer.push_back(mf);

  //added by kh(042116)
  //m_comp_subbuffer[mf->get_tpc()].insert(std::pair<unsigned, mem_fetch*>(mf->get_request_uid(), mf));
  if(m_comp_subbuffer.size() > 0)	m_comp_subbuffer[mf->get_tpc()].push_back(mf);
  ///
}

void hpcl_comp_buffer::pop_mem_fetch() {
  mem_fetch* mf = m_comp_buffer[0];
  m_comp_buffer.erase(m_comp_buffer.begin());

  //added by kh(042116)
  //std::map<unsigned, mem_fetch*>::iterator it;
  //it = m_comp_subbuffer[mf->get_tpc()].find(mf->get_request_uid());
  ///m_comp_subbuffer[mf->get_tpc()].erase (it);

  if(m_comp_subbuffer.size() > 0) {
    std::vector<mem_fetch*>::iterator it = m_comp_subbuffer[mf->get_tpc()].begin();
    unsigned erase_index = 0;
    for(unsigned i = 0; i < m_comp_subbuffer[mf->get_tpc()].size(); i++) {
      if(m_comp_subbuffer[mf->get_tpc()][i] == mf) {
	erase_index = i;	break;
      }
    }
    m_comp_subbuffer[mf->get_tpc()].erase(it+erase_index);
    ///
  }
}

mem_fetch* hpcl_comp_buffer::top_mem_fetch() {
  if(m_comp_buffer.size() == 0)	return NULL;
  else				return m_comp_buffer[0];
}

bool hpcl_comp_buffer::has_comp_buffer_space() {
  if(m_comp_buffer.size() >= m_comp_buffer_size)	return false;
  else							return true;
}

void hpcl_comp_buffer::print() {

  printf("--- hpcl_comp_buffer ---\n");
  for(unsigned j = 0; j < m_comp_buffer.size(); j++) {
    printf("\tcomp_buffer %u mf %u\n", j, m_comp_buffer[j]->get_request_uid());
  }

}

void hpcl_comp_buffer::print(unsigned id, unsigned comp_res) {

  std::vector<unsigned short> word_list_2B;
  std::vector<unsigned int> word_list_4B;
  std::vector<unsigned long long> word_list_8B;

//  printf("hpcl_comp_buffer[%u] = ", id);
//  for(unsigned i = 0; i < m_comp_subbuffer.size(); i++)
//  {
//      printf("%u ", (unsigned)m_comp_subbuffer[i].size());
//  }
//  printf("\n");

  for(unsigned i = 0; i < m_comp_subbuffer.size(); i++)
  {
    if(m_comp_subbuffer[i].size() > 1) {
      for(unsigned j = 0; j < m_comp_subbuffer[i].size(); j++) {

	unsigned resolution = comp_res;
	if(resolution == 0) {
	    resolution = m_comp_subbuffer[i][j]->get_comp_res();
	}

	//printf("mf %u\n", m_comp_subbuffer[i][j]->get_request_uid());
	std::vector<unsigned char>& cache_data = m_comp_subbuffer[i][j]->get_real_data_ptr();
	//printf("org_data_sm_%u_rep_%u = 0x");
	//for(unsigned i = 0; i < cache_data.size(); i++) {
	//  printf("%02x", cache_data[i]);
	//}
	//printf("\n");
	if(cache_data.size() > 0) {
	  /*
	  if(resolution == 2) {
	    hpcl_comp_lwm_pl_proc<unsigned short>::decompose_data(cache_data, word_list_2B);
	    hpcl_comp_lwm_pl_proc<unsigned short>::print_word_list(word_list_2B);
	    word_list_2B.clear();
	  } else if(resolution == 4) {
	    hpcl_comp_lwm_pl_proc<unsigned int>::decompose_data(cache_data, word_list_4B);
	    hpcl_comp_lwm_pl_proc<unsigned int>::print_word_list(word_list_4B);
	    word_list_4B.clear();
	  } else if(resolution == 8) {
	    hpcl_comp_lwm_pl_proc<unsigned long long>::decompose_data(cache_data, word_list_8B);
	    hpcl_comp_lwm_pl_proc<unsigned long long>::print_word_list(word_list_8B);
	    word_list_8B.clear();
	  }
	  */
	  //m_comp_subbuffer[i][j]->print_compword();
	  //m_comp_subbuffer[i][j]->print_uncompword();
	} else {
	  //printf("\tno data\n");
	}
      }
    }
  }

//  printf("\n");
}

bool hpcl_comp_buffer::has_pending_mem_fetch() {
  return (m_comp_buffer.size() > 0)? true : false;
}

void hpcl_comp_buffer::get_reply_rate_dist(double& single_rep_rate, double& multi_rep_rate, double& no_rep_rate, double& avg_multi_rep_no)
{
  single_rep_rate = 0;
  multi_rep_rate = 0;
  no_rep_rate = 0;

  avg_multi_rep_no = 0;
  double sm_no_for_multi_rep = 0;
  for(unsigned i = 0; i < m_comp_subbuffer.size(); i++)
  {
    //deleted by kh(042316)
    /*
    if(m_comp_subbuffer[i].size() == 0)	no_rep_rate++;
    else if(m_comp_subbuffer[i].size() == 1)	single_rep_rate++;
    else					multi_rep_rate++;
    */

    //added by kh(042316)
    //To check only the redundancy in read replies
    if(m_comp_subbuffer[i].size() == 0)	no_rep_rate++;
    else if(m_comp_subbuffer[i].size() == 1) {
      if(m_comp_subbuffer[i][0]->get_is_write())	no_rep_rate++;
      else						single_rep_rate++;
    } else {
      unsigned multi_read_rep_no = 0;
      for(unsigned j = 0; j < m_comp_subbuffer[i].size(); j++) {
	 if(!m_comp_subbuffer[i][j]->get_is_write())	multi_read_rep_no++;
      }

      if(multi_read_rep_no == 0) 	no_rep_rate++;
      else if(multi_read_rep_no == 1)	single_rep_rate++;
      else if(multi_read_rep_no > 1) {
	  multi_rep_rate++;
	  avg_multi_rep_no += multi_read_rep_no;
	  sm_no_for_multi_rep++;
      }
    }
    ///
  }

  no_rep_rate = no_rep_rate/m_comp_subbuffer.size();
  single_rep_rate = single_rep_rate/m_comp_subbuffer.size();
  multi_rep_rate = multi_rep_rate/m_comp_subbuffer.size();

  if(sm_no_for_multi_rep > 0)	avg_multi_rep_no = avg_multi_rep_no/sm_no_for_multi_rep;
}

#ifdef old
void hpcl_comp_buffer::anal_redund_inter_read_replies() {

  std::vector<unsigned short> word_list_2B;
  std::vector<unsigned int> word_list_4B;
  std::vector<unsigned long long> word_list_8B;
/*
  printf("hpcl_comp_buffer[%u] = ", id);
  for(unsigned i = 0; i < m_comp_subbuffer.size(); i++)
  {
      printf("%u ", (unsigned)m_comp_subbuffer[i].size());
  }
  printf("\n");
*/

  for(unsigned i = 0; i < m_comp_subbuffer.size(); i++)
  {
    //if multi-reply data for the same SM are found.
    std::vector<mem_fetch*> multi_read_replies;
    //deleted by kh(051716)
    //if(m_comp_subbuffer[i].size() > 1) {
    //added by kh(051716)
    if(m_comp_subbuffer[i].size() >= 1) {

	for(unsigned j = 0; j < m_comp_subbuffer[i].size(); j++) {
	/*
	unsigned resolution = comp_res;
	if(resolution == 0) {
	  resolution = m_comp_subbuffer[i][j]->get_comp_res();
	}
	*/

//	printf("[%u] mf %u, addr 0x%08x, ld_pc 0x%08x, sm %u, time %llu\n", i,
//	       m_comp_subbuffer[i][j]->get_request_uid(),
//	       m_comp_subbuffer[i][j]->get_addr(),
//	       m_comp_subbuffer[i][j]->get_pc(),
//	       m_comp_subbuffer[i][j]->get_tpc(),
//	       (gpu_sim_cycle+gpu_tot_sim_cycle));

	std::vector<unsigned char>& cache_data = m_comp_subbuffer[i][j]->get_real_data_ptr();

	if(cache_data.size() > 0) {	//read reply(not inst) only are considered.
	  //m_comp_subbuffer[i][j]->print_compword();
	  //m_comp_subbuffer[i][j]->print_uncompword();

	  multi_read_replies.push_back(m_comp_subbuffer[i][j]);
	} else {
	  //printf("\tno data\n");
	}
      }
    }

    unsigned all_word1B_no = 0;
    unsigned redund_word1B_no = 0;
    for(unsigned j = 0; j < multi_read_replies.size() && multi_read_replies.size() > 1; j++) {
	//printf("mf %u\n", multi_read_replies[j]->get_request_uid());
	//multi_read_replies[j]->print_uncompword();

	//redundancy is measured based on the uncompressed word in the first reply
	if(j == 0) {
	  unsigned row = multi_read_replies[j]->get_uncompword_row_no();
	  for(unsigned m = 0; m < row; m++) {
	    unsigned col = multi_read_replies[j]->get_uncompword_col_no(m);
	    for(unsigned n = 0; n < col; n++) {
	      class mem_fetch::comp_word tmp = multi_read_replies[j]->get_uncompword(m, n);
	      if(tmp.comp_status == mem_fetch::UNCOMPRESSED) {
		  m_dict_for_inter_read_replies->update_dict(tmp.word1B, 0);
	      }
	    }
	  }
	} else {
	  unsigned row = multi_read_replies[j]->get_uncompword_row_no();
	  for(unsigned m = 0; m < row; m++) {
	    unsigned col = multi_read_replies[j]->get_uncompword_col_no(m);
	    for(unsigned n = 0; n < col; n++) {
	      class mem_fetch::comp_word tmp = multi_read_replies[j]->get_uncompword(m, n);
	      if(tmp.comp_status == mem_fetch::UNCOMPRESSED) {
		  int index = m_dict_for_inter_read_replies->search_word(tmp.word1B);
		  if(index >= 0) redund_word1B_no++;

		  all_word1B_no++;
	      }
	    }
	  }
	}
    }

    if(all_word1B_no > 0) {
	//printf("all_word1B_no %u, redund_word1B_no %u, inter_read_replies_redund_rate %f ",
	//       all_word1B_no, redund_word1B_no, (double)redund_word1B_no/all_word1B_no);
	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::WORD1B_REDUND_RATE_INTER_READ_REPLIES, (double)redund_word1B_no/all_word1B_no);
    }

    m_dict_for_inter_read_replies->clear();
    multi_read_replies.clear();
  }

}
#endif

std::vector<mem_fetch*>& hpcl_comp_buffer::get_all_waiting_mfs (mem_fetch* mf)
{
  unsigned sm_id = mf->get_tpc();
  return m_comp_subbuffer[sm_id];
}


void hpcl_comp_buffer::delete_mfs (std::vector<mem_fetch*>& mfs)
{
  std::vector<mem_fetch*>::iterator it;
  for(unsigned i = 0; i < mfs.size(); i++) {
//    printf("hpcl_comp_buffer::delete_mfs - mf %u", mfs[i]->get_request_uid());

    std::vector<mem_fetch*>::iterator it1 = std::find (m_comp_buffer.begin(), m_comp_buffer.end(), mfs[i]);
//    if(it1 == m_comp_buffer.end()) {
//      for(unsigned j = 0; j < m_comp_buffer.size(); j++) {
//	printf("\tcomp_buffer %u mf %u\n", j, m_comp_buffer[j]->get_request_uid());
//      }
//    }
    assert (it1 != m_comp_buffer.end());
    m_comp_buffer.erase(it1);
    unsigned sm_id = mfs[i]->get_tpc();
    std::vector<mem_fetch*>::iterator it2 = std::find (m_comp_subbuffer[sm_id].begin(), m_comp_subbuffer[sm_id].end(), mfs[i]);
    assert (it2 != m_comp_subbuffer[sm_id].end());
    m_comp_subbuffer[sm_id].erase(it2);

    //printf("hpcl_comp_buffer::delete_mfs - mf %u, mf %u", (*it1)->get_request_uid(), (*it2)->get_request_uid());
  }


  //added by kh(052416)
  //for debugging
  /*
  unsigned comp_data_no = 0;
  for(unsigned i = 0; i < m_comp_subbuffer.size(); i++) {
    comp_data_no += m_comp_subbuffer[i].size();
  }
  assert(m_comp_buffer.size() == comp_data_no);
  */
  ///

}

mem_fetch* hpcl_comp_buffer::get_next_mem_fetch() {

  if(g_hpcl_comp_config.hpcl_rec_comp_en == 0) {
    return top_mem_fetch();
  } else {
    return top_mem_fetch();

#ifdef old
    if(g_hpcl_comp_config.hpcl_inter_comp_schedule_policy == hpcl_comp_config::INTER_COMP_FIFO) {
      return top_mem_fetch();
    } else {

      int max_rep_index = -1;
      int max_rep_no = -1;
      int min_rep_index = -1;
      int min_rep_no = -1;

      //if(m_id == 0) {
      // std::cout << "restore m_last_sm_id " << m_last_sm_id << std::endl;
      //}

      //added by kh(052416)
      int start_id = m_last_sm_id+1;
      if(start_id == m_comp_subbuffer.size()) start_id = 0;
      ///

      int count = m_comp_subbuffer.size();
      while(count)
      //for(unsigned i = 0; i < m_comp_subbuffer.size(); i++)
      {
	unsigned i = start_id;
	//if(m_id == 0) {
	//    std::cout << "\t scan " << i << std::endl;
	//}

	if(m_comp_subbuffer[i].size() != 0) {
	  int rep_no = 0;
	  for(unsigned j = 0; j < m_comp_subbuffer[i].size(); j++) {
	    std::vector<unsigned char>& cache_data = m_comp_subbuffer[i][j]->get_real_data_ptr();
	    if(cache_data.size() > 0) {	//read reply(not inst) only are considered.
	      rep_no++;
	    }
	  }

	  //debugging
	  //std::cout << "\ti " << i <<  " rep_no " << rep_no << std::endl;

	  if((max_rep_no == -1) || (max_rep_no > 0 && (max_rep_no < rep_no))) {
	    max_rep_no = rep_no;	max_rep_index = i;
	  }

	  if((min_rep_no == -1) || (min_rep_no > 0 && (min_rep_no > rep_no))) {
	    min_rep_no = rep_no;	min_rep_index = i;
	  }
	}
	count--;
	start_id++;
	if(start_id == m_comp_subbuffer.size()) start_id = 0;
      }

      //debugging
      /*
      if(max_rep_index >= 0) std::cout << "\tmax_rep_index " << max_rep_index <<  " max_rep_no " << max_rep_no << std::endl;
      if(min_rep_index >= 0) std::cout << "\tmin_rep_index " << min_rep_index <<  " min_rep_no " << min_rep_no << std::endl;
      if(count >= 3) {
	  if(max_rep_no > 1)	assert(0);
      }
      */


      if(g_hpcl_comp_config.hpcl_inter_comp_schedule_policy == hpcl_comp_config::INTER_COMP_MAX_REP_FIRST) {
	  if(max_rep_index == -1) {
	    m_last_sm_id = m_last_sm_id+1;
	    if(m_last_sm_id == m_comp_subbuffer.size()) m_last_sm_id = 0;
	    //if(m_id == 0) {
	    //	std::cout << "save m_last_sm_id " << m_last_sm_id << std::endl;
	    //}
	    return NULL;
	  } else {
	    m_last_sm_id = max_rep_index;
	    //if(m_id == 0) {
	    //	std::cout << "save m_last_sm_id(SEL) " << m_last_sm_id << std::endl;
	    //}
	    return m_comp_subbuffer[max_rep_index][0];
	  }
      } else if(g_hpcl_comp_config.hpcl_inter_comp_schedule_policy == hpcl_comp_config::INTER_COMP_MIN_REP_FIRST) {
	  if(min_rep_index == -1) {
	    m_last_sm_id = m_last_sm_id+1;
	    if(m_last_sm_id == m_comp_subbuffer.size()) m_last_sm_id = 0;
	    return NULL;
	  } else {
	    m_last_sm_id = min_rep_index;
	    return m_comp_subbuffer[min_rep_index][0];
	  }
      } else	assert(0);

      return NULL;
    }
#endif

  }

}


void hpcl_comp_buffer::del_mem_fetch(mem_fetch* mf) {

  std::vector<mem_fetch*>::iterator it1 = m_comp_buffer.begin();
  int erase_index1 = -1;
  for(unsigned i = 0; i < m_comp_buffer.size(); i++) {
    if(m_comp_buffer[i] == mf) {
      erase_index1 = i;	break;
    }
  }
  assert(erase_index1 >= 0);
  m_comp_buffer.erase(it1+erase_index1);

//  printf("mf %u is removed from m_comp_buffer\n", mf->get_request_uid());
//  printf(" ---  comp_buffer --- \n");
//  for(int i = 0; i < m_comp_buffer.size(); i++) {
//    printf("\tmf %u\n", m_comp_buffer[i]->get_request_uid());
//  }

  if(m_comp_subbuffer.size() > 0) {
    std::vector<mem_fetch*>::iterator it2 = m_comp_subbuffer[mf->get_tpc()].begin();
    int erase_index2 = -1;
    for(unsigned i = 0; i < m_comp_subbuffer[mf->get_tpc()].size(); i++) {
      if(m_comp_subbuffer[mf->get_tpc()][i] == mf) {
	erase_index2 = i;	break;
      }
    }
    assert(erase_index2 >= 0);
    m_comp_subbuffer[mf->get_tpc()].erase(it2+erase_index2);
    ///
  }
}
