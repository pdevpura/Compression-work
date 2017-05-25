/*
 * hpcl_comp_anal.h
 *
 *  Created on: Mar 8, 2016
 *      Author: mumichang
 */

#ifndef GPGPU_SIM_HPCL_DYN_COMP_H_
#define GPGPU_SIM_HPCL_DYN_COMP_H_

//#include "hpcl_stat.h"
//#include <iostream>

#include <vector>
#include <string>
#define KERNEL_CAHE_STATE_TBL_SZ 32// 16
#define SAMPLING_TIME	10000
#define THRE_CACHE_MISS_RATE 0.5
#define MAX_SAMPLING_CNT_PER_STATE 5
#define MAX_KERNEL_CNT 3

#include "hpcl_comp_config.h"
#include "../option_parser.h"
extern hpcl_comp_config g_hpcl_comp_config;
#include <cassert>

class hpcl_dyn_comp
{
public:
  hpcl_dyn_comp ();
  virtual ~hpcl_dyn_comp ();

public:
  enum dyn_comp_pred_state {
    CACHE_COMP_OFF = 0,
    CACHE_COMP_ON,
  };
  void init(unsigned sm_id);

private:
  enum dyn_comp_state {
    INIT = 0,
    MON,
    END
  };

  struct kernel_cache_state {
    std::string m_kernel_name;
    //cache count in_unended kernel
    unsigned m_prev_high_cache_miss_cnt, m_prev_low_cache_miss_cnt;
    enum dyn_comp_pred_state m_cache_comp_flag;
    enum dyn_comp_state m_state;

    kernel_cache_state() {
      m_prev_high_cache_miss_cnt = 0;
      m_prev_low_cache_miss_cnt = 0;
      m_cache_comp_flag = CACHE_COMP_OFF;
      m_state = INIT;
    }

    void reset_cnt() {
      m_prev_high_cache_miss_cnt = 0;
      m_prev_low_cache_miss_cnt = 0;
    }

    void update_state(enum dyn_comp_state state) {
      m_state = state;
    }

    void print() {
      printf("\tKernel %50s, ", m_kernel_name.c_str());
      if(m_state == INIT)	printf("State INIT, ");
      else if(m_state == MON)	printf("State  MON, ");
      else if(m_state == END)	printf("State  END, ");
      printf(" HiMissCnt: %02u, LoMissCnt: %02u, ", m_prev_high_cache_miss_cnt, m_prev_low_cache_miss_cnt);
      if(m_cache_comp_flag == CACHE_COMP_ON)    printf("CacheComp  ON\n");
      else					printf("CacheComp OFF\n");
    }


  };

  std::vector<struct kernel_cache_state> m_kernel_cache_state;
  int m_active_kernel_cache_state_index;
  unsigned m_active_kernel_high_cache_miss_cnt;
  unsigned m_active_kernel_low_cache_miss_cnt;
  unsigned m_sm_id;

public:
  //deleted by kh(110816)
  void update_sample_stat(double cache_miss_rate);
  //added by kh(110816)
  //void update_sample_stat(double cache_miss_rate, double comp_rate);

  void update_last_sample_stat(double cache_miss_rate);
  void activate(std::string kernel_name);
  enum dyn_comp_pred_state run();

  std::string get_active_cache_state_kernel_name() {
    //assert(m_active_kernel_cache_state_index >= 0);
    if(m_active_kernel_cache_state_index < 0) {
      std::string tmp;
      return tmp;
    } else {
      return m_kernel_cache_state[m_active_kernel_cache_state_index].m_kernel_name;
    }
  }

  enum dyn_comp_pred_state get_cache_comp_status() {
    return run();
  }

  void print() {
    printf("--- HPCL_DYN_COMP STATUS (SM %u) ---\n", m_sm_id);
    for(int i = 0; i < m_kernel_cache_state.size(); i++) {
      if(m_kernel_cache_state[i].m_kernel_name.empty() == false)
	m_kernel_cache_state[i].print();
    }
  }

};

#endif /* GPGPU_SIM_HPCL_DYN_COMP_H_ */
