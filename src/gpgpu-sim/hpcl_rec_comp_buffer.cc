/*
 * NetworkStat.cpp
 *
 *  Created on: Sep 19, 2015
 *      Author: mumichang
 */


#include "hpcl_rec_comp_buffer.h"

#include <vector>
#include <cassert>
#include <algorithm>    // std::find

//added by kh(052016)
#include "hpcl_comp_config.h"
extern hpcl_comp_config g_hpcl_comp_config;
///

//added by kh(062516)
extern unsigned long long  gpu_sim_cycle;
extern unsigned long long  gpu_tot_sim_cycle;
///

#include "hpcl_dict.h"

void hpcl_rec_comp_buffer::create(unsigned comp_buffer_size)
{
  m_comp_buffer_size = comp_buffer_size;
  m_comp_buffer.clear();
}

void hpcl_rec_comp_buffer::push_mem_fetch(mem_fetch* mf)
{
  //printf("%llu | hpcl_rec_comp_buffer::push_mem_fetch is called\n", (gpu_sim_cycle+gpu_tot_sim_cycle));
  m_comp_buffer.push_back(mf);
}

void hpcl_rec_comp_buffer::pop_mem_fetch()
{
  mem_fetch* mf = m_comp_buffer[0];
  m_comp_buffer.erase(m_comp_buffer.begin());
}

mem_fetch* hpcl_rec_comp_buffer::top_mem_fetch()
{
  if(m_comp_buffer.empty())	return NULL;
  else				return m_comp_buffer[0];
}

bool hpcl_rec_comp_buffer::has_comp_buffer_space()
{
  //if(m_comp_buffer.size() >= m_comp_buffer_size)	return false;
  //else							return true;
  unsigned mf_no = 0;
  for(unsigned i = 0; i < m_comp_buffer.size(); i++) {
    mf_no += 1;
    mf_no += m_comp_buffer[i]->get_merged_mf_to_same_SM_no();
    mf_no += m_comp_buffer[i]->get_merged_mf_no();
  }

  if(mf_no >= m_comp_buffer_size)	return false;
  else					return true;

}

/*
void hpcl_rec_comp_buffer::print()
{

}

bool hpcl_rec_comp_buffer::has_pending_mem_fetch() {
  return (m_comp_buffer.size() > 0)? true : false;
}
*/

/*
std::vector<mem_fetch*>& hpcl_rec_comp_buffer::get_all_waiting_mfs (mem_fetch* mf)
{
  unsigned sm_id = mf->get_tpc();
  return m_comp_subbuffer[sm_id];
}


void hpcl_rec_comp_buffer::delete_mfs (std::vector<mem_fetch*>& mfs)
{
  std::vector<mem_fetch*>::iterator it;
  for(unsigned i = 0; i < mfs.size(); i++) {
//    printf("hpcl_rec_comp_buffer::delete_mfs - mf %u", mfs[i]->get_request_uid());

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

    //printf("hpcl_rec_comp_buffer::delete_mfs - mf %u, mf %u", (*it1)->get_request_uid(), (*it2)->get_request_uid());
  }
}
*/

mem_fetch* hpcl_rec_comp_buffer::get_next_mem_fetch() {
  return top_mem_fetch();
}


void hpcl_rec_comp_buffer::del_mem_fetch(mem_fetch* mf) {

  std::vector<mem_fetch*>::iterator it1 = m_comp_buffer.begin();
  int erase_index1 = -1;
  for(unsigned i = 0; i < m_comp_buffer.size(); i++) {
    if(m_comp_buffer[i] == mf) {
      erase_index1 = i;	break;
    }
  }
  assert(erase_index1 >= 0);
  m_comp_buffer.erase(it1+erase_index1);

}

hpcl_rec_comp_buffer::last_mem_fetch_status hpcl_rec_comp_buffer::get_last_mem_fetch(int comp_res, int sm, mem_fetch** last_mf) {

  enum last_mem_fetch_status status = NOT_FOUND;
  *last_mf = NULL;

  if(comp_res == -1) {
    if(sm == -1) {
      for(int i = (m_comp_buffer.size()-1); i >= 0; i--) {
        if(m_comp_buffer[i]->get_real_data_size() > 0) {
	  status = FOUND;
	  *last_mf = m_comp_buffer[i];
	  return status;
	} else {

	}
      }
    } else {
      for(int i = (m_comp_buffer.size()-1); i >= 0; i--) {
	if(m_comp_buffer[i]->get_real_data_size() > 0) {
	  if(m_comp_buffer[i]->get_tpc() == sm) {
	    status = FOUND;
	    *last_mf = m_comp_buffer[i];
	    return status;
	  }
	}
      }
    }
  } else {

    if(sm == -1) {
      for(int i = (m_comp_buffer.size()-1); i >= 0; i--) {
	if(m_comp_buffer[i]->get_real_data_size() > 0) {
	  if(m_comp_buffer[i]->get_comp_res() == comp_res) {
	    status = FOUND;
	    *last_mf = m_comp_buffer[i];
	    return status;
	  } else {
	    status = COMP_RES_MISMATCH;
	  }
	}
      }
    } else {
      for(int i = (m_comp_buffer.size()-1); i >= 0; i--) {
	if(m_comp_buffer[i]->get_real_data_size() > 0 && m_comp_buffer[i]->get_tpc() == sm) {
	  if(m_comp_buffer[i]->get_comp_res() == comp_res) {
	    status = FOUND;
	    *last_mf = m_comp_buffer[i];
	    return status;
	  } else {
	    status = COMP_RES_MISMATCH;
	  }
	}
      }
    }

  }
  return status;
}

void hpcl_rec_comp_buffer::print()
{
  printf("---- hpcl_rec_comp_buffer (Time: %llu) ----\n", (gpu_sim_cycle+gpu_tot_sim_cycle));
  for(int i = 0; i < m_comp_buffer.size(); i++) {
    unsigned is_read = m_comp_buffer[i]->get_is_write()? 0 : 1;
    printf("\t%d | mf %u | sm %u | READ %u | Data Size %u\n", i, m_comp_buffer[i]->get_request_uid(), m_comp_buffer[i]->get_tpc(), is_read, m_comp_buffer[i]->get_real_data_size());
  }
  printf("\n");
}

mem_fetch* hpcl_rec_comp_buffer::get_mem_fetch_to_same_sm(int sm)
{
  for(int i = (m_comp_buffer.size()-1); i >= 0; i--) {
    if(m_comp_buffer[i]->get_real_data_size() > 0
    && m_comp_buffer[i]->get_tpc() == sm
    && m_comp_buffer[i]->get_merged_mf_no() == 0) {
      return m_comp_buffer[i];
    }
  }
  return NULL;
}

void hpcl_rec_comp_buffer::get_mem_fetch_to_diff_sm(int sm, std::vector<mem_fetch*>& diff_sm_mfs)
{
  diff_sm_mfs.clear();
  for(int i = (m_comp_buffer.size()-1); i >= 0; i--) {
    if(m_comp_buffer[i]->get_real_data_size() > 0
    && m_comp_buffer[i]->get_tpc() != sm
    && m_comp_buffer[i]->get_merged_mf_to_same_SM_no() == 0) {
      diff_sm_mfs.push_back(m_comp_buffer[i]);
    }
  }


}













mem_fetch* hpcl_rec_comp_buffer::get_mem_fetch_to_same_sm(int comp_res, int sm)
{
  for(int i = (m_comp_buffer.size()-1); i >= 0; i--) {
    if(m_comp_buffer[i]->get_real_data_size() > 0 && m_comp_buffer[i]->get_tpc() == sm) {
      if(m_comp_buffer[i]->get_comp_res() == comp_res) {
	return m_comp_buffer[i];
      }
    }
  }

  return NULL;
}

int hpcl_rec_comp_buffer::compute_dist(mem_fetch* new_mf, mem_fetch* prev_mf)
{
  int ret = -1;
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


  if(comp_res == 2) {
    unsigned short word1_1 = new_loc_dict_2B->get_word(0);
    unsigned short word1_2 = prev_loc_dict_2B->get_word(0);
    if(word1_1 == word1_2)	ret = 0;
    else			ret = 1;
  } else if(comp_res == 4) {
    unsigned int word1_1 = new_loc_dict_4B->get_word(0);
    unsigned int word1_2 = prev_loc_dict_4B->get_word(0);
    if(word1_1 == word1_2)	ret = 0;
    else			ret = 1;
  } else if(comp_res == 8) {
    unsigned long long word1_1 = new_loc_dict_8B->get_word(0);
    unsigned long long word1_2 = prev_loc_dict_8B->get_word(0);
    if(word1_1 == word1_2)	ret = 0;
    else			ret = 1;
  }

  return ret;
}

//this is used when replies to different SMs are merged.
mem_fetch* hpcl_rec_comp_buffer::get_closest_mem_fetch(int comp_res, int sm, mem_fetch* new_mf)
{
  mem_fetch* ret = NULL;
  int dist = -1;

  for(int i = (m_comp_buffer.size()-1); i >= 0; i--) {
    if(m_comp_buffer[i]->get_real_data_size() > 0 && m_comp_buffer[i]->get_tpc() != sm) {
      if(m_comp_buffer[i]->get_comp_res() == comp_res) {
	//return m_comp_buffer[i];
	if((m_comp_buffer[i]->get_merged_mf_to_same_SM_no() == 0)
	&& ((m_comp_buffer[i]->get_merged_mf_no()+1) < g_hpcl_comp_config.hpcl_rec_comp_max_pkt_no) ) {

	   int _dist = compute_dist(new_mf, m_comp_buffer[i]);
	   if(dist == -1) {
	     dist = _dist;
	     ret = m_comp_buffer[i];
	   } else {
	     if(_dist < dist) {
	       dist = _dist;
	       ret = m_comp_buffer[i];
	     }
	   }

	}
      }
    }
  }

  //if the first word does not match, don't attempt the recursive compression
  if(dist != 0) {
    ret = NULL;
  }

  return ret;
}

void hpcl_rec_comp_buffer::get_candi_for_rec_comp_to_diff_sm(int comp_res, int sm, mem_fetch* new_mf, vector<mem_fetch*>& candi)
{
  mem_fetch* ret = NULL;
  int dist = -1;

  //for(int i = (m_comp_buffer.size()-1); i >= 0; i--)
  for(int i = 0; i < m_comp_buffer.size(); i++)
  {
    if(m_comp_buffer[i]->get_real_data_size() == 0)		continue;
    if(m_comp_buffer[i]->get_tpc() == sm)			continue;
    if(m_comp_buffer[i]->get_comp_res() != comp_res)		continue;
    if(m_comp_buffer[i]->get_merged_mf_to_same_SM_no() > 0)	continue;
    //deleted by kh(063016)
    //if(m_comp_buffer[i]->get_merged_mf_no() > 0)		continue;

    candi.push_back(m_comp_buffer[i]);
  }
}




