
#include "hpcl_dyn_comp.h"
#include <cassert>




hpcl_dyn_comp::hpcl_dyn_comp ()
{
  // TODO Auto-generated constructor stub
  m_kernel_cache_state.resize(KERNEL_CAHE_STATE_TBL_SZ);
  m_active_kernel_cache_state_index = -1;
}

hpcl_dyn_comp::~hpcl_dyn_comp ()
{
  // TODO Auto-generated destructor stub
}

void hpcl_dyn_comp::init(unsigned sm_id) {
  m_sm_id = sm_id;
}


void hpcl_dyn_comp::activate(std::string kernel_name) {

  //Step1: Obtain the kernel_cache_state index
  int active_tbl_index = -1;
  for(int i = 0; i < m_kernel_cache_state.size(); i++) {
    if(m_kernel_cache_state[i].m_kernel_name.compare(kernel_name) == 0) {
      active_tbl_index = i;
      break;
    }
  }

  if(active_tbl_index < 0) {
    for(int i = 0; i < m_kernel_cache_state.size(); i++) {
      if(m_kernel_cache_state[i].m_kernel_name.empty() == true) {
	m_kernel_cache_state[i].m_kernel_name = kernel_name;
	active_tbl_index = i;
	break;
      }
    }
  }

  assert(active_tbl_index >= 0);
  m_active_kernel_cache_state_index = active_tbl_index;
  m_active_kernel_high_cache_miss_cnt = 0;
  m_active_kernel_low_cache_miss_cnt = 0;

  struct kernel_cache_state& kcs = m_kernel_cache_state[m_active_kernel_cache_state_index];
  if(kcs.m_state == INIT) {
    if((kcs.m_prev_high_cache_miss_cnt+kcs.m_prev_low_cache_miss_cnt) >= MAX_KERNEL_CNT) {
      kcs.m_state = END;
      if(kcs.m_prev_low_cache_miss_cnt > kcs.m_prev_high_cache_miss_cnt) {
	kcs.m_cache_comp_flag = CACHE_COMP_OFF;
      } else {
	kcs.m_cache_comp_flag = CACHE_COMP_ON;
      }
    }
  }

}

//void hpcl_dyn_comp::update_sample_stat(double cache_miss_rate, double comp_rate) {
void hpcl_dyn_comp::update_sample_stat(double cache_miss_rate) {

  struct kernel_cache_state& kcs = m_kernel_cache_state[m_active_kernel_cache_state_index];
  //if(cache_miss_rate < THRE_CACHE_MISS_RATE) {
  //added by kh(110716)
  //printf("g_hpcl_comp_config.hpcl_adaptive_comp_threshold %f\n", g_hpcl_comp_config.hpcl_adaptive_comp_threshold);
  //assert(0);
  if(cache_miss_rate < g_hpcl_comp_config.hpcl_adaptive_comp_threshold) {
    m_active_kernel_low_cache_miss_cnt++;
  } else {
    m_active_kernel_high_cache_miss_cnt++;
  }

  if(kcs.m_state == INIT) {
    if((m_active_kernel_low_cache_miss_cnt+m_active_kernel_high_cache_miss_cnt) >= MAX_SAMPLING_CNT_PER_STATE) {
      kcs.m_state = MON;
      m_active_kernel_low_cache_miss_cnt = 0;
      m_active_kernel_high_cache_miss_cnt = 0;
    }
  } else if(kcs.m_state == MON) {
    if((m_active_kernel_low_cache_miss_cnt+m_active_kernel_high_cache_miss_cnt) >= MAX_SAMPLING_CNT_PER_STATE) {
      kcs.m_state = END;
      //m_active_kernel_low_cache_miss_cnt = 0;
      //m_active_kernel_high_cache_miss_cnt = 0;

      if(m_active_kernel_low_cache_miss_cnt > m_active_kernel_high_cache_miss_cnt) {
	kcs.m_cache_comp_flag = CACHE_COMP_OFF;
	printf("Kernel %s | UPDATE SM %u CACHE_COMP_OFF\n", kcs.m_kernel_name.c_str(), m_sm_id);
      } else {
	kcs.m_cache_comp_flag = CACHE_COMP_ON;
	printf("Kernel %s | UPDATE SM %u CACHE_COMP_ON\n", kcs.m_kernel_name.c_str(), m_sm_id);
      }
    }
  } else {
    //DO NOTHING
  }

}

void hpcl_dyn_comp::update_last_sample_stat(double cache_miss_rate) {

  struct kernel_cache_state& kcs = m_kernel_cache_state[m_active_kernel_cache_state_index];
  if(kcs.m_state == INIT || kcs.m_state == MON) {

//deleted by kh(103016)
/*
    if(m_active_kernel_low_cache_miss_cnt > m_active_kernel_high_cache_miss_cnt) {
      kcs.m_prev_low_cache_miss_cnt++;
      //printf("Kernel %s | UPDATE SM %u low_cache_miss_cnt %u\n", kcs.m_kernel_name.c_str(), m_sm_id, kcs.m_prev_low_cache_miss_cnt);
    } else if(m_active_kernel_low_cache_miss_cnt < m_active_kernel_high_cache_miss_cnt) {
      kcs.m_prev_high_cache_miss_cnt++;
      //printf("Kernel %s | UPDATE SM %u high_cache_miss_cnt %u\n", kcs.m_kernel_name.c_str(), m_sm_id, kcs.m_prev_high_cache_miss_cnt);
    } else {
      if(cache_miss_rate < THRE_CACHE_MISS_RATE) {
	kcs.m_prev_low_cache_miss_cnt++;
	//printf("Kernel %s | UPDATE SM %u low_cache_miss_cnt %u\n", kcs.m_kernel_name.c_str(), m_sm_id, kcs.m_prev_low_cache_miss_cnt);
      } else {
	kcs.m_prev_high_cache_miss_cnt++;
	//printf("Kernel %s | UPDATE SM %u high_cache_miss_cnt %u\n", kcs.m_kernel_name.c_str(), m_sm_id, kcs.m_prev_high_cache_miss_cnt);
      }
    }
    kcs.m_state = INIT;
*/
//added by kh(103016)
//by default, turn on cache compression
    kcs.m_cache_comp_flag = CACHE_COMP_OFF;
    kcs.m_state = END;
///

  } else {
    //Do nothing
  }

}

enum hpcl_dyn_comp::dyn_comp_pred_state hpcl_dyn_comp::run() {

  struct kernel_cache_state& kcs = m_kernel_cache_state[m_active_kernel_cache_state_index];
  if(kcs.m_state == END) {
    return kcs.m_cache_comp_flag;
  } else {
    return CACHE_COMP_OFF;
  }

}
