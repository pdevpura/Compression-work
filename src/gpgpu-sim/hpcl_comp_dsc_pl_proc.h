/*
 * hpcl_comp_pl_proc.h
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#ifndef HPCL_COMP_DSC_PL_PROC_H_
#define HPCL_COMP_DSC_PL_PROC_H_

#include "hpcl_comp_pl_data.h"
#include "hpcl_user_define_stmts.h"
#include <cmath>

#include "hpcl_comp_config.h"
extern hpcl_comp_config g_hpcl_comp_config;

class hpcl_comp_dsc_pl_proc {
private:
  hpcl_comp_pl_data* m_input;
  hpcl_comp_pl_data* m_output;
  int m_pl_index;

public:
  hpcl_comp_dsc_pl_proc();
  ~hpcl_comp_dsc_pl_proc() {
    if(m_input)	 	delete m_input;
    if(m_output)	delete m_output;
  };
  void set_pl_index(int pl_index);
  void set_output(hpcl_comp_pl_data* output);
  hpcl_comp_pl_data* get_output();
  hpcl_comp_pl_data* get_input();
  void reset_output();

public:
  void run();

//added by kh(042216)
//public:
//static void decompose_data(std::vector<unsigned char>& cache_data, std::vector<std::vector<unsigned char> >& ds_list, unsigned res);
//  static void decompose_data(K word, std::set<unsigned char>& word_list);
//  static void decompose_data(K word, std::vector<unsigned char>& word1B_list);
//  static void print_word_list(std::vector<K>& word_list);
///

private:
  void run_ds_comp(mem_fetch* mf, unsigned res, unsigned data_type, std::vector<unsigned char>& cache_data, std::vector<unsigned>& enc_status, unsigned& comp_bits, unsigned& comp_data_bits_only);
  int m_min_data_segment_size;
private:
  void run_ds_comp_multi_res(mem_fetch* mf, unsigned max_res, unsigned data_type, std::vector<unsigned char>& cache_data,std::vector<unsigned>& enc_status, unsigned& comp_bits, unsigned& comp_data_bits_only);

private:
  unsigned encoding_bits;

//added by kh(073016)
private:
  int m_type;
public:
  void set_pl_type(int type);
  int get_pl_type();
  enum
  {
    NONE=0,
    DUMMY,
    COMP,
    GET_OUTPUT,
  };
///


};

hpcl_comp_dsc_pl_proc::hpcl_comp_dsc_pl_proc() {
  m_input = new hpcl_comp_pl_data;
  m_output = NULL;
  m_pl_index = -1;
  m_min_data_segment_size = 8;

  //deleted bny kh(030117)
  //encoding_bits = 4;
  if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::DATA_SEG_MATCHING)
	  encoding_bits = 4;
  else if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::DATA_SEG_MATCHING2)
	  encoding_bits = 1;
  else
	  assert(0);
  ///


  m_type = NONE;
}

void hpcl_comp_dsc_pl_proc::set_pl_index(int pl_index) {
  m_pl_index = pl_index;
}

void hpcl_comp_dsc_pl_proc::set_pl_type(int type) {
  m_type = type;
}

int hpcl_comp_dsc_pl_proc::get_pl_type() {
  return m_type;
}

void hpcl_comp_dsc_pl_proc::set_output(hpcl_comp_pl_data* output) {
  m_output = output;
}

hpcl_comp_pl_data* hpcl_comp_dsc_pl_proc::get_output() {
  return m_output;
}

hpcl_comp_pl_data* hpcl_comp_dsc_pl_proc::get_input() {
  return m_input;
}

void hpcl_comp_dsc_pl_proc::reset_output() {
  m_output->clean();
}

void hpcl_comp_dsc_pl_proc::run()
{
  mem_fetch* mf = m_input->get_mem_fetch();
  m_output->set_mem_fetch(mf);

  if(!mf) return;

  if(mf->get_real_data_size() == 0)	return;

  #ifdef OLD //added by kh (021817)
  unsigned DSC_MIN_DS_SIZE = g_hpcl_comp_config.hpcl_dsc_min_ds_size; //16; //2; //16;

  std::vector<std::vector<unsigned char>* > cache_data;
  std::vector<std::vector<unsigned> > enc_status;
  std::vector<unsigned> comp_data_bits;
  std::vector<unsigned> comp_data_bits_only;

  //added by kh(071316)
  unsigned data_type_no = mf->get_trans_data_no() + 1;

  cache_data.resize(data_type_no, NULL);
  enc_status.resize(data_type_no);
  comp_data_bits.resize(data_type_no, 0);
  comp_data_bits_only.resize(data_type_no, 0);

  cache_data[0] = &mf->get_real_data_ptr();
  for(int i = 1; i < data_type_no; i++) cache_data[i] = &mf->get_trans_data_ptr(i-1);

  std::vector<unsigned> dsc_res_candi;
  dsc_res_candi.push_back(DSC_MIN_DS_SIZE);
  unsigned ds_size = DSC_MIN_DS_SIZE*2;
  while(ds_size <= cache_data[0]->size()) {
    //added by kh(073016)
    if(ds_size > g_hpcl_comp_config.hpcl_dsc_max_ds_size) {
      break;
    }
    ///
    dsc_res_candi.push_back(ds_size);
    ds_size = ds_size*2;
  }
  #endif


  //Step1-1: gather all preprocessed data for data
  std::vector<std::vector<unsigned char>* > cache_data;
  std::vector<std::vector<unsigned> > enc_status;
  std::vector<unsigned> comp_data_bits;
  std::vector<unsigned> comp_data_bits_only;
  unsigned data_type_no = -1;

  if(g_hpcl_comp_config.hpcl_char_preproc_en == 1) {

	  //added by kh (021817)
	  if( mf->get_chartype() == false ) {	//Numerical Data

		  data_type_no = mf->get_trans_data_no() + 1;
		  cache_data.resize(data_type_no, NULL);
		  enc_status.resize(data_type_no);
		  comp_data_bits.resize(data_type_no, 0);
		  comp_data_bits_only.resize(data_type_no, 0);

		  cache_data[0] = &mf->get_real_data_ptr();
		  for(int i = 1; i < data_type_no; i++) cache_data[i] = &mf->get_trans_data_ptr(i-1);
		  ///

	  } else {	//Char Data

		  data_type_no = 2;
		  cache_data.resize(data_type_no, NULL);
		  enc_status.resize(data_type_no);
		  comp_data_bits.resize(data_type_no, 0);
		  comp_data_bits_only.resize(data_type_no, 0);

		  cache_data[0] = &mf->get_real_data_ptr();
		  cache_data[1] = &mf->get_txt_trans_data_ptr();

	  }
	  ///

  } else {

	  data_type_no = mf->get_trans_data_no() + 1;
	  cache_data.resize(data_type_no, NULL);
	  enc_status.resize(data_type_no);
	  comp_data_bits.resize(data_type_no, 0);
	  comp_data_bits_only.resize(data_type_no, 0);

	  cache_data[0] = &mf->get_real_data_ptr();
	  for(int i = 1; i < data_type_no; i++) cache_data[i] = &mf->get_trans_data_ptr(i-1);

  }

  //Step1-2: gather all configured ds sizes
  unsigned DSC_MIN_DS_SIZE = g_hpcl_comp_config.hpcl_dsc_min_ds_size; //16; //2; //16;
  std::vector<unsigned> dsc_res_candi;
  dsc_res_candi.push_back(DSC_MIN_DS_SIZE);
  unsigned ds_size = DSC_MIN_DS_SIZE*2;
  while(ds_size <= cache_data[0]->size()) {
	//added by kh(073016)
	if(ds_size > g_hpcl_comp_config.hpcl_dsc_max_ds_size) {
	  break;
	}
	///
	dsc_res_candi.push_back(ds_size);
	ds_size = ds_size*2;
  }
  ///

  /*
  for(int i = 0; i < dsc_res_candi.size(); i++) {
    printf("des_res_candi : %u ", dsc_res_candi[i]);
  }
  printf("\n");
  assert(0);
  */

  unsigned min_index = 0;
  unsigned min_comp_data_bits = 0;
  unsigned min_ds_size = 0;
  unsigned min_comp_data_bits_only = 0;
  unsigned candi_res_no = dsc_res_candi.size();

  //added by kh(080316)
  unsigned start_data_type_index = 0;
  if(g_hpcl_comp_config.hpcl_remap_data_use_only_en == 1) {
    start_data_type_index = 1;
  }
  ///

  //deleted by kh(080316)
  //for(unsigned m = 0; m < data_type_no; m++) {
  //added by kh(080316)
  for(unsigned m = start_data_type_index; m < data_type_no; m++) {
    //static resolution for all segments
    //#ifdef DATA_SEG_COMP_STATIC_RES
    unsigned min_rec_comp_data_bits = 0;
    unsigned min_rec_comp_data_seg = 0;
    unsigned min_rec_comp_data_bits_only = 0;
    if(g_hpcl_comp_config.hpcl_dsc_lwm_rep_opt_en == 0) {
      std::vector<std::vector<unsigned> > rec_enc_status;
      std::vector<unsigned> rec_comp_data_bits;
      std::vector<unsigned> rec_comp_data_bits_only;
      rec_enc_status.resize(candi_res_no);
      rec_comp_data_bits.resize(candi_res_no,0);
      rec_comp_data_bits_only.resize(candi_res_no,0);

      for(unsigned i = 0; i < candi_res_no; i++) {
		run_ds_comp(mf, dsc_res_candi[i], m, *cache_data[m], rec_enc_status[i], rec_comp_data_bits[i], rec_comp_data_bits_only[i]);
		DATA_SEG_COMP_DEBUG_PRINT("DSM : mf %u res %u data_type %d org_data_bits %u comp_bits %u\n",
		  mf->get_request_uid(), dsc_res_candi[i], m, mf->get_real_data_size()*8, rec_comp_data_bits[i]);
      }

      min_rec_comp_data_bits = rec_comp_data_bits[0];
      min_rec_comp_data_seg = dsc_res_candi[0];
      min_rec_comp_data_bits_only = rec_comp_data_bits_only[0];
      for(int i = 1; i < rec_comp_data_bits.size(); i++) {
		if(rec_comp_data_bits[i] < min_rec_comp_data_bits) {
		  min_rec_comp_data_bits = rec_comp_data_bits[i];
		  min_rec_comp_data_seg = dsc_res_candi[i];
		  min_rec_comp_data_bits_only = rec_comp_data_bits_only[i];
		}
      }


    } else {
    //#else

      //added by kh(072016)
      //dynamic resolution for all segments
      std::vector<unsigned> min_rec_enc_status;
      //unsigned min_rec_comp_data_bits = 0;
      //unsigned min_rec_comp_data_bits_only = 0;
      min_rec_comp_data_seg = 16;	//need to be fixed.
      run_ds_comp_multi_res(mf, 16, m, *cache_data[m], min_rec_enc_status, min_rec_comp_data_bits, min_rec_comp_data_bits_only);

    //#endif
    }

    if(min_comp_data_bits == 0 || (min_comp_data_bits > min_rec_comp_data_bits)) {
      min_comp_data_bits = min_rec_comp_data_bits;
      min_ds_size = min_rec_comp_data_seg;
      min_index = m;
      min_comp_data_bits_only = min_rec_comp_data_bits_only;
    }

  }

  DATA_SEG_COMP_DEBUG_PRINT("min_comp_data_bits %d, min_ds_size %u, data_source %u\n", min_comp_data_bits, min_ds_size, min_index);


  if(min_comp_data_bits > cache_data[0]->size()*8) { //if the compressed data is larger than original cache data

    min_comp_data_bits = cache_data[0]->size()*8+1;		//1-bit indicates compressed or not
    min_comp_data_bits_only = cache_data[0]->size()*8;

    //mf->set_comp_data_bits(cache_data[0]->size()*8+1);	//1-bit indicates compressed or not
    mf->set_comp_data_bits(min_comp_data_bits);
    mf->set_dsc_comp_res(0);
    mf->set_dsc_comp_data_type(mem_fetch::NO_DATA_TYPE);

    mf->set_comp_algo_type(mem_fetch::NO_COMP);

    #ifdef old_hybrid
    mf->set_comp_ds_status_size(cache_data[0]->size()/DSC_MIN_DS_SIZE, 0);
    mf->set_comp_ds_status_size(cache_data[0]->size()/DSC_MIN_DS_SIZE, 0, mem_fetch::ORG_DATA);
    mf->set_comp_ds_status_size(cache_data[0]->size()/DSC_MIN_DS_SIZE, 0, mem_fetch::REMAPPED_DATA_1);
    mf->set_comp_ds_status_size(cache_data[0]->size()/DSC_MIN_DS_SIZE, 0, mem_fetch::REMAPPED_DATA_2);
    mf->set_comp_ds_status_size(cache_data[0]->size()/DSC_MIN_DS_SIZE, 0, mem_fetch::REMAPPED_DATA_3);
    mf->set_comp_ds_status_size(cache_data[0]->size()/DSC_MIN_DS_SIZE, 0, mem_fetch::REMAPPED_DATA_4);
    #endif

    mf->set_dsc_compressed_bits_only(min_comp_data_bits_only);

    DATA_SEG_COMP_DEBUG_PRINT("correct min_comp_data_bits %d\n", cache_data[0]->size()*8+1);
    DATA_SEG_COMP_DEBUG_PRINT("compressed_bits_only %u\n", mf->get_dsc_compressed_bits_only());

    //std::cout << "comp_data_type = " << mf->get_dsc_comp_data_type() << std::endl;
    //assert(0);










  } else {

    mf->set_comp_data_bits(min_comp_data_bits);
    mf->set_dsc_comp_res(min_ds_size);

    mf->set_comp_algo_type(mem_fetch::DSM_COMP);

    if(g_hpcl_comp_config.hpcl_char_preproc_en == 1) {

  	  //added by kh (021817)
  	  if( mf->get_chartype() == true ) {	//Char Data

  		mf->set_dsc_comp_data_type(mem_fetch::CHAR_DATA);

  	  } else {

  	    if(min_index == 0) 		mf->set_dsc_comp_data_type(mem_fetch::ORG_DATA);
  	    else 					mf->set_dsc_comp_data_type(mf->get_remapped_data_type(min_index-1));

  	  }
  	  ///

    } else {

        if(min_index == 0) 		mf->set_dsc_comp_data_type(mem_fetch::ORG_DATA);
        else 					mf->set_dsc_comp_data_type(mf->get_remapped_data_type(min_index-1));

    }


/*
    printf("----- mf %u starts ---- \n", mf->get_request_uid());
    mf->print_data_all(min_ds_size);
    printf("DSM - Final: Select ");
    mf->print_dsc_comp_data_type(mf->get_dsc_comp_data_type());
    printf("min_comp_data_bits %u, org_data_bits %u\n", min_comp_data_bits, mf->get_real_data_size()*8);
    printf("----------------------- \n\n");
*/
    /*
    if(min_index == 0) 		mf->set_dsc_comp_data_type(mem_fetch::ORG_DATA);
    else if(min_index == 1) 	mf->set_dsc_comp_data_type(mem_fetch::REMAPPED_DATA_1);
    else if(min_index == 2) 	mf->set_dsc_comp_data_type(mem_fetch::REMAPPED_DATA_2);
    else if(min_index == 3) 	mf->set_dsc_comp_data_type(mem_fetch::REMAPPED_DATA_3);
    else if(min_index == 4) 	mf->set_dsc_comp_data_type(mem_fetch::REMAPPED_DATA_4);
    else			assert(0);
    */

    #ifdef old_hybrid
    mf->set_comp_ds_status_size(cache_data[0]->size()/DSC_MIN_DS_SIZE, 0);
    mf->set_comp_ds_status_size(cache_data[0]->size()/DSC_MIN_DS_SIZE, 0, mem_fetch::ORG_DATA);
    mf->set_comp_ds_status_size(cache_data[0]->size()/DSC_MIN_DS_SIZE, 0, mem_fetch::REMAPPED_DATA_1);
    mf->set_comp_ds_status_size(cache_data[0]->size()/DSC_MIN_DS_SIZE, 0, mem_fetch::REMAPPED_DATA_2);
    mf->set_comp_ds_status_size(cache_data[0]->size()/DSC_MIN_DS_SIZE, 0, mem_fetch::REMAPPED_DATA_3);
    mf->set_comp_ds_status_size(cache_data[0]->size()/DSC_MIN_DS_SIZE, 0, mem_fetch::REMAPPED_DATA_4);
    for(unsigned j = 0; j < min_comp_status->size(); j++) {
      unsigned offset = min_ds_size/DSC_MIN_DS_SIZE;
      for(unsigned k = 0; k < offset; k++) {
	mf->set_comp_ds_status(j*offset+k, min_comp_status->at(j));
	unsigned ds_index = j*offset+k;
	DATA_SEG_COMP_DEBUG_PRINT("\tDS%u - status %u\n", ds_index, min_comp_status->at(j));
      }
    }
    #endif

    mf->set_dsc_compressed_bits_only(min_comp_data_bits_only);
    DATA_SEG_COMP_DEBUG_PRINT("compressed_bits_only %u\n", mf->get_dsc_compressed_bits_only());

    #ifdef old_hybrid
    //Analysis to see if the best DSC is worse when the LWM is considered.
    for(int m = 0; m <= 4; m++) {
      unsigned dsc_comp_data_bits = 0;
      unsigned dsc_comp_data_bits_only = 0;
      std::vector<unsigned> dsc_comp_ds_status;
      run_ds_comp(mf, min_ds_size, m, *cache_data[m], dsc_comp_ds_status, dsc_comp_data_bits, dsc_comp_data_bits_only);

      for(unsigned j = 0; j < dsc_comp_ds_status.size(); j++) {
        unsigned offset = min_ds_size/DSC_MIN_DS_SIZE;
        for(unsigned k = 0; k < offset; k++) {
	  if(m == 0)		mf->set_comp_ds_status(j*offset+k, dsc_comp_ds_status[j], mem_fetch::ORG_DATA);
	  else if(m == 1)	mf->set_comp_ds_status(j*offset+k, dsc_comp_ds_status[j], mem_fetch::REMAPPED_DATA_1);
	  else if(m == 2)	mf->set_comp_ds_status(j*offset+k, dsc_comp_ds_status[j], mem_fetch::REMAPPED_DATA_2);
	  else if(m == 3)	mf->set_comp_ds_status(j*offset+k, dsc_comp_ds_status[j], mem_fetch::REMAPPED_DATA_3);
	  else if(m == 4)	mf->set_comp_ds_status(j*offset+k, dsc_comp_ds_status[j], mem_fetch::REMAPPED_DATA_4);
	  //unsigned ds_index = j*offset+k;
	  //DATA_SEG_COMP_DEBUG_PRINT("\tDS%u - status %u\n", ds_index, dsc_comp_ds_status[j]);
        }
      }

      if(m == 0)	mf->set_dsc_compressed_bits_only(dsc_comp_data_bits_only, mem_fetch::ORG_DATA);
      else if(m == 1)	mf->set_dsc_compressed_bits_only(dsc_comp_data_bits_only, mem_fetch::REMAPPED_DATA_1);
      else if(m == 2)	mf->set_dsc_compressed_bits_only(dsc_comp_data_bits_only, mem_fetch::REMAPPED_DATA_2);
      else if(m == 3)	mf->set_dsc_compressed_bits_only(dsc_comp_data_bits_only, mem_fetch::REMAPPED_DATA_3);
      else if(m == 4)	mf->set_dsc_compressed_bits_only(dsc_comp_data_bits_only, mem_fetch::REMAPPED_DATA_4);
    }
    ///
    #endif
  }


  //added by kh(090116)
  unsigned ed_bits = mf->get_dsc_compressed_bits_only();
  unsigned es_bits = mf->get_comp_data_bits() - mf->get_dsc_compressed_bits_only();
  mf->set_dsc_comp_ed_byte(ceil(ed_bits/8.0));
  mf->set_dsc_comp_es_byte(ceil(es_bits/8.0));
  ///

  DATA_SEG_COMP_DEBUG_PRINT("DSM (Final): mf %u res %u data_type %d org_data_bits %u comp_bits %u comp_bits_only %u\n", mf->get_request_uid(), mf->get_dsc_comp_res(), mf->get_dsc_comp_data_type(), mf->get_real_data_size()*8, mf->get_comp_data_bits(), mf->get_dsc_compressed_bits_only());
  DATA_SEG_COMP_DEBUG_PRINT("\n");

}

#ifdef old
void hpcl_comp_dsc_pl_proc::run()
{
  mem_fetch* mf = m_input->get_mem_fetch();
  m_output->set_mem_fetch(mf);

  if(!mf) return;

  //If mf is not type for compression,
  if(mf->get_real_data_size() == 0)	return;

  unsigned DSC_MIN_DS_SIZE = g_hpcl_comp_config.hpcl_dsc_min_ds_size; //16; //2; //16;

  //DATA_SEG_MATCHING: choose the case where the entire size of cache is minimum
  //DATA_SEG_LWM_MATCHING: choose the case where the size of compressed data segments is minimum

  //printf("hpcl_comp_dsc_pl_proc: mf %u\n", mf->get_request_uid());

  std::vector<std::vector<unsigned char>* > cache_data;
  std::vector<std::vector<unsigned> > enc_status;
  std::vector<unsigned> comp_data_bits;
  std::vector<unsigned> comp_data_bits_only;

  //added by kh(071316)
  unsigned data_type_no = mf->get_trans_data_no() + 1;
  //printf("data_type_no: %u\n", data_type_no);

  cache_data.resize(data_type_no, NULL);
  enc_status.resize(data_type_no);
  comp_data_bits.resize(data_type_no, 0);
  comp_data_bits_only.resize(data_type_no, 0);

  cache_data[0] = &mf->get_real_data_ptr();
  for(int i = 1; i < data_type_no; i++) cache_data[i] = &mf->get_trans_data_ptr(i-1);
  //cache_data[1] = &mf->get_trans_data_ptr(0);
  //cache_data[2] = &mf->get_trans_data_ptr(1);


  unsigned min_comp_data_bits = 0;
  unsigned min_index = 0;
  unsigned min_ds_size = DSC_MIN_DS_SIZE;
  unsigned min_comp_data_bits_only = 0;
  std::vector<unsigned>* min_comp_status = NULL;
  for(int i = 0; i < cache_data.size(); i++) {
    run_ds_comp(mf, DSC_MIN_DS_SIZE, i, *cache_data[i], enc_status[i], comp_data_bits[i], comp_data_bits_only[i]);
    if(i == 0) {
      min_comp_data_bits = comp_data_bits[i];
      min_index = 0;
    } else {
      if(min_comp_data_bits > comp_data_bits[i]) {
	min_comp_data_bits = comp_data_bits[i];
	min_index = i;
      }
    }
  }
  min_comp_data_bits_only = comp_data_bits_only[min_index];
  min_comp_status = &enc_status[min_index];

  DATA_SEG_COMP_DEBUG_PRINT("mf %u min_comp_data_bits %u, min_index %u\n", mf->get_request_uid(), min_comp_data_bits, min_index);


  DATA_SEG_COMP_DEBUG_PRINT("DSM : mf %u res %u data_type %d org_data_bits %u comp_bits %u\n", mf->get_request_uid(), min_ds_size, min_index, mf->get_real_data_size()*8, min_comp_data_bits);


  std::vector<unsigned> dsc_res_candi;
  unsigned ds_size = DSC_MIN_DS_SIZE*2;
  while(ds_size <= cache_data[0]->size()) {
    dsc_res_candi.push_back(ds_size);
    ds_size = ds_size*2;
  }

  unsigned candi_res_no = dsc_res_candi.size();
  std::vector<std::vector<unsigned> > rec_enc_status;
  std::vector<unsigned> rec_comp_data_bits;
  std::vector<unsigned> rec_comp_data_bits_only;
  rec_enc_status.resize(candi_res_no);
  rec_comp_data_bits.resize(candi_res_no,0);
  rec_comp_data_bits_only.resize(candi_res_no,0);

  for(unsigned i = 0; i < candi_res_no; i++) {
    run_ds_comp(mf, dsc_res_candi[i], min_index, *cache_data[min_index], rec_enc_status[i], rec_comp_data_bits[i], rec_comp_data_bits_only[i]);
    DATA_SEG_COMP_DEBUG_PRINT("DSM : mf %u res %u data_type %d org_data_bits %u comp_bits %u\n", mf->get_request_uid(), dsc_res_candi[i], min_index, mf->get_real_data_size()*8, rec_comp_data_bits[i]);
  }

/*
  unsigned candi_res_no = cache_data[0]->size()/32;	//cache block can be 64B < 128B
  candi_res_no = candi_res_no/2 + 1;			//32B --> 32B DS, 64 or 96B --> 32B, 64B DS, 128--> 32B, 64B, 128B DS
  rec_enc_status.resize(candi_res_no);
  rec_comp_data_bits.resize(candi_res_no,0);
  rec_comp_data_bits_only.resize(candi_res_no,0);

  for(unsigned i = 0; i < candi_res_no; i++) {
    run_ds_comp(32*pow(2,i), *cache_data[min_index], rec_enc_status[i], rec_comp_data_bits[i], rec_comp_data_bits_only[i]);
  }
*/
  int min_res_candi_index = -1;
  for(int i = 0; i < rec_comp_data_bits.size(); i++) {
    if(rec_comp_data_bits[i] < min_comp_data_bits) {
      min_comp_data_bits = rec_comp_data_bits[i];
      min_res_candi_index = i;
    }
  }
  if(min_res_candi_index >= 0) {
    min_ds_size = dsc_res_candi[min_res_candi_index];
    min_comp_data_bits_only = rec_comp_data_bits_only[min_res_candi_index];
    min_comp_status = &rec_enc_status[min_res_candi_index];
  }
  ///


  DATA_SEG_COMP_DEBUG_PRINT("min_comp_data_bits %d, min_ds_size %u\n", min_comp_data_bits, min_ds_size);


  if(min_comp_data_bits > cache_data[0]->size()*8) { //if the compressed data is larger than original cache data
    mf->set_comp_data_bits(cache_data[0]->size()*8+1);	//1-bit indicates compressed or not
    mf->set_dsc_comp_res(0);
    mf->set_dsc_comp_data_type(mem_fetch::NO_DATA_TYPE);

    mf->set_comp_algo_type(mem_fetch::NO_COMP);

    #ifdef old_hybrid
    mf->set_comp_ds_status_size(cache_data[0]->size()/DSC_MIN_DS_SIZE, 0);
    mf->set_comp_ds_status_size(cache_data[0]->size()/DSC_MIN_DS_SIZE, 0, mem_fetch::ORG_DATA);
    mf->set_comp_ds_status_size(cache_data[0]->size()/DSC_MIN_DS_SIZE, 0, mem_fetch::REMAPPED_DATA_1);
    mf->set_comp_ds_status_size(cache_data[0]->size()/DSC_MIN_DS_SIZE, 0, mem_fetch::REMAPPED_DATA_2);
    mf->set_comp_ds_status_size(cache_data[0]->size()/DSC_MIN_DS_SIZE, 0, mem_fetch::REMAPPED_DATA_3);
    mf->set_comp_ds_status_size(cache_data[0]->size()/DSC_MIN_DS_SIZE, 0, mem_fetch::REMAPPED_DATA_4);
    #endif

    mf->set_dsc_compressed_bits_only(1);

    DATA_SEG_COMP_DEBUG_PRINT("correct min_comp_data_bits %d\n", cache_data[0]->size()*8+1);
    DATA_SEG_COMP_DEBUG_PRINT("compressed_bits_only %u\n", mf->get_dsc_compressed_bits_only());

    //std::cout << "comp_data_type = " << mf->get_dsc_comp_data_type() << std::endl;
    //assert(0);


  } else {

    mf->set_comp_data_bits(min_comp_data_bits);
    mf->set_dsc_comp_res(min_ds_size);

    mf->set_comp_algo_type(mem_fetch::DSM_COMP);

    if(min_index == 0) 		mf->set_dsc_comp_data_type(mem_fetch::ORG_DATA);
    else 			mf->set_dsc_comp_data_type(mf->get_remapped_data_type(min_index-1));

    /*
    if(min_index == 0) 		mf->set_dsc_comp_data_type(mem_fetch::ORG_DATA);
    else if(min_index == 1) 	mf->set_dsc_comp_data_type(mem_fetch::REMAPPED_DATA_1);
    else if(min_index == 2) 	mf->set_dsc_comp_data_type(mem_fetch::REMAPPED_DATA_2);
    else if(min_index == 3) 	mf->set_dsc_comp_data_type(mem_fetch::REMAPPED_DATA_3);
    else if(min_index == 4) 	mf->set_dsc_comp_data_type(mem_fetch::REMAPPED_DATA_4);
    else			assert(0);
    */

    #ifdef old_hybrid
    mf->set_comp_ds_status_size(cache_data[0]->size()/DSC_MIN_DS_SIZE, 0);
    mf->set_comp_ds_status_size(cache_data[0]->size()/DSC_MIN_DS_SIZE, 0, mem_fetch::ORG_DATA);
    mf->set_comp_ds_status_size(cache_data[0]->size()/DSC_MIN_DS_SIZE, 0, mem_fetch::REMAPPED_DATA_1);
    mf->set_comp_ds_status_size(cache_data[0]->size()/DSC_MIN_DS_SIZE, 0, mem_fetch::REMAPPED_DATA_2);
    mf->set_comp_ds_status_size(cache_data[0]->size()/DSC_MIN_DS_SIZE, 0, mem_fetch::REMAPPED_DATA_3);
    mf->set_comp_ds_status_size(cache_data[0]->size()/DSC_MIN_DS_SIZE, 0, mem_fetch::REMAPPED_DATA_4);
    for(unsigned j = 0; j < min_comp_status->size(); j++) {
      unsigned offset = min_ds_size/DSC_MIN_DS_SIZE;
      for(unsigned k = 0; k < offset; k++) {
	mf->set_comp_ds_status(j*offset+k, min_comp_status->at(j));
	unsigned ds_index = j*offset+k;
	DATA_SEG_COMP_DEBUG_PRINT("\tDS%u - status %u\n", ds_index, min_comp_status->at(j));
      }
    }
    #endif

    mf->set_dsc_compressed_bits_only(min_comp_data_bits_only);
    DATA_SEG_COMP_DEBUG_PRINT("compressed_bits_only %u\n", mf->get_dsc_compressed_bits_only());

    #ifdef old_hybrid
    //Analysis to see if the best DSC is worse when the LWM is considered.
    for(int m = 0; m <= 4; m++) {
      unsigned dsc_comp_data_bits = 0;
      unsigned dsc_comp_data_bits_only = 0;
      std::vector<unsigned> dsc_comp_ds_status;
      run_ds_comp(mf, min_ds_size, m, *cache_data[m], dsc_comp_ds_status, dsc_comp_data_bits, dsc_comp_data_bits_only);

      for(unsigned j = 0; j < dsc_comp_ds_status.size(); j++) {
        unsigned offset = min_ds_size/DSC_MIN_DS_SIZE;
        for(unsigned k = 0; k < offset; k++) {
	  if(m == 0)		mf->set_comp_ds_status(j*offset+k, dsc_comp_ds_status[j], mem_fetch::ORG_DATA);
	  else if(m == 1)	mf->set_comp_ds_status(j*offset+k, dsc_comp_ds_status[j], mem_fetch::REMAPPED_DATA_1);
	  else if(m == 2)	mf->set_comp_ds_status(j*offset+k, dsc_comp_ds_status[j], mem_fetch::REMAPPED_DATA_2);
	  else if(m == 3)	mf->set_comp_ds_status(j*offset+k, dsc_comp_ds_status[j], mem_fetch::REMAPPED_DATA_3);
	  else if(m == 4)	mf->set_comp_ds_status(j*offset+k, dsc_comp_ds_status[j], mem_fetch::REMAPPED_DATA_4);
	  //unsigned ds_index = j*offset+k;
	  //DATA_SEG_COMP_DEBUG_PRINT("\tDS%u - status %u\n", ds_index, dsc_comp_ds_status[j]);
        }
      }

      if(m == 0)	mf->set_dsc_compressed_bits_only(dsc_comp_data_bits_only, mem_fetch::ORG_DATA);
      else if(m == 1)	mf->set_dsc_compressed_bits_only(dsc_comp_data_bits_only, mem_fetch::REMAPPED_DATA_1);
      else if(m == 2)	mf->set_dsc_compressed_bits_only(dsc_comp_data_bits_only, mem_fetch::REMAPPED_DATA_2);
      else if(m == 3)	mf->set_dsc_compressed_bits_only(dsc_comp_data_bits_only, mem_fetch::REMAPPED_DATA_3);
      else if(m == 4)	mf->set_dsc_compressed_bits_only(dsc_comp_data_bits_only, mem_fetch::REMAPPED_DATA_4);
    }
    ///
    #endif
  }
  DATA_SEG_COMP_DEBUG_PRINT("DSM (Final): mf %u res %u data_type %d org_data_bits %u comp_bits %u\n", mf->get_request_uid(), mf->get_dsc_comp_res(), mf->get_dsc_comp_data_type(), mf->get_real_data_size()*8, mf->get_comp_data_bits());
  DATA_SEG_COMP_DEBUG_PRINT("\n");

}
#endif

void hpcl_comp_dsc_pl_proc::run_ds_comp(mem_fetch* mf, unsigned res,  unsigned data_type, std::vector<unsigned char>& cache_data,std::vector<unsigned>& enc_status, unsigned& comp_bits, unsigned& comp_data_bits_only)
{
  int cache_size = cache_data.size();
  int ds_no = cache_size/res;

  #ifdef DATA_SEG_COMP_DEBUG_PRINT
  DATA_SEG_COMP_DEBUG_PRINT("mf %u, Data_Type = %u, Res = %u ", mf->get_request_uid(), data_type, res);
  DATA_SEG_COMP_DEBUG_PRINT("Data(%03d) = ", cache_size);
  for(int i = 0; i < cache_size; i++) {
      DATA_SEG_COMP_DEBUG_PRINT("%02x", cache_data[i]);
  }
  DATA_SEG_COMP_DEBUG_PRINT("\n");
  #endif

  enc_status.resize(ds_no, 0);
  comp_bits = 0;
  comp_data_bits_only = 0;

  unsigned dsc_comp_header_bits = 0; //1-bit compressed or not, 2-bit resolution size, 8-bit encoding status
  dsc_comp_header_bits += 1; 		 //1-bit compressed or not

  //added by kh(091916)
  if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::DATA_SEG_LWM_HYBRID_PARL) {
    dsc_comp_header_bits += 3; //3-bit data remapping type
    dsc_comp_header_bits += 3; //3-bit resolution ( 2 ~ 128 )
    dsc_comp_header_bits += 3; //3-bit # of ds = 2 ^ (3bit)
  }

  //dsc_comp_header_bits += (ds_no == 1)? 1 : (unsigned) log2((double)ds_no);	// based on the resolution and cache size
  //dsc_comp_header_bits += ds_no;
  comp_bits += dsc_comp_header_bits;
  //comp_data_bits_only += dsc_comp_header_bits;

  DATA_SEG_COMP_DEBUG_PRINT("\tStep1: comp_bits = %u\n", comp_bits);
  if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::DATA_SEG_MATCHING)
  {
	  //added by kh(030117)
	  int last_encoded_index = -1;
	  for(int i = 0; i < ds_no; i++)
	  {
		bool is_all_byte_same = true;

		int index = i * res;
		unsigned char byte_data = cache_data[index];
		for(int j = 1; j < res; j++)
		{
		  //DATA_SEG_COMP_DEBUG_PRINT("\tbyte_data %u, cache_data[%d] %u, ", byte_data, index+j, cache_data[index+j]);
		  if(byte_data != cache_data[index+j]) {
			is_all_byte_same = false;
			//DATA_SEG_COMP_DEBUG_PRINT("diff!\n");
			break;
		  } else {
			//DATA_SEG_COMP_DEBUG_PRINT("same!\n");
		  }
		}

		bool is_compressed = false;
		if(is_all_byte_same == true) {
		  //DATA_SEG_COMP_DEBUG_PRINT("\tDS%d has all bytes same\n", i);
		  unsigned char high_nibble = (byte_data & 0xf0) >> 4;
		  unsigned char low_nibble = (byte_data & 0x0f);

		  //DATA_SEG_COMP_DEBUG_PRINT("\thigh_nibble %u low_nibble %u\n", high_nibble, low_nibble);
		  if(high_nibble == low_nibble) {
			 is_compressed = true;
		  }
		} else {
		  //DATA_SEG_COMP_DEBUG_PRINT("\tDS%d does not have all bytes same\n", i);
		}

		if(is_compressed == true) {
		  enc_status[i] = 1;
		  comp_bits += (encoding_bits+1);		//1: encoding status bit. 4: encoding bits


//		  //added by kh(030117)
//		  if(g_hpcl_comp_config.hpcl_dyn_encoding_status_en == 0) {
//			  comp_bits += (encoding_bits+1);		//1: encoding status bit. 4: encoding bits
//		  } else {
//			  comp_bits += (encoding_bits);			//1: encoding bits
//			  last_encoded_index = i;
//		  }
		  comp_data_bits_only += (encoding_bits);	//4: encoding bits
		  DATA_SEG_COMP_DEBUG_PRINT("\tDS%d is compressed\n", i);

		} else {
		  enc_status[i] = 0;
		  comp_bits += (res*8+1);

//		  //added by kh(030117)
//		  if(g_hpcl_comp_config.hpcl_dyn_encoding_status_en == 0) {
//			  comp_bits += (res*8+1);
//		  } else {
//			  comp_bits += (res*8);
//		  }
//		  ///
		  comp_data_bits_only += (res*8);		//res*8: uncompressed data size
		  DATA_SEG_COMP_DEBUG_PRINT("\tDS%d is not compressed\n", i);
		}
	  }

//	  if(g_hpcl_comp_config.hpcl_dyn_encoding_status_en == 1) {
//		  int all_encoding_status_bits = (last_encoded_index+1 <= 0) ? 0 : (last_encoded_index+1);
//		  comp_bits += all_encoding_status_bits;
//	  }

  } else if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::DATA_SEG_MATCHING2){

	  //added by kh(030117)
	  //Add the fixed size field that shows the actual number of encoded segments.
	  if(g_hpcl_comp_config.hpcl_dyn_encoding_status_en == 1) {
		  int max_ds_no_bit_no = ceil(log2((double)(128/res)));
		  comp_bits += max_ds_no_bit_no;
	  }
	  ///

	  //added by kh(030117)
	  int last_encoded_index = -1;
	  for(int i = 0; i < ds_no; i++)
	  {
		bool is_all_byte_same = true;
		int index = i * res;
		unsigned char byte_data = cache_data[index];
		for(int j = 1; j < res; j++)
		{
		  if(byte_data != cache_data[index+j]) {
			is_all_byte_same = false;
			break;
		  }
		}

		//if compressed
		if(is_all_byte_same == true && (byte_data == 0x00 || byte_data == 0xff)) {
		  enc_status[i] = 1;
		  //added by kh(030117)
		  if(g_hpcl_comp_config.hpcl_dyn_encoding_status_en == 0) {
			  comp_bits += (1+1);			//1: encoding status bit. 1: encoding bits
		  } else {
			  comp_bits += (1);			//1: encoding bits
			  last_encoded_index = i;
		  }

		  comp_data_bits_only += (1);	//1: encoding bits
		  DATA_SEG_COMP_DEBUG_PRINT("\tDS%d is compressed\n", i);
		} else {
		  enc_status[i] = 0;

		  //added by kh(030117)
		  if(g_hpcl_comp_config.hpcl_dyn_encoding_status_en == 0) {
			  comp_bits += (res*8+1);
		  } else {
			  comp_bits += (res*8);
		  }

		  comp_data_bits_only += (res*8);		//res*8: uncompressed data size
		  DATA_SEG_COMP_DEBUG_PRINT("\tDS%d is not compressed\n", i);
		}
	  }

	  //added by kh(030117)
	  if(g_hpcl_comp_config.hpcl_dyn_encoding_status_en == 1) {
		int all_encoding_status_bits = (last_encoded_index+1 <= 0) ? 0 : (last_encoded_index+1);
		//printf("mf %u Before dynamic_encoding_status_bits : %d\n", mf->get_request_uid(), comp_bits);
		comp_bits += all_encoding_status_bits;
		//printf("mf %u After dynamic_encoding_status_bits : %d\n", mf->get_request_uid(), comp_bits);
	  }


  } else {
	  assert(0);
  }

  DATA_SEG_COMP_DEBUG_PRINT("\tStep2: comp_bits = %u, comp_data_bits_only = %u\n", comp_bits, comp_data_bits_only);
  /*
  if(comp_data_bits_only > 0) {
    comp_data_bits_only += 1;	//1-bit compressed or not
  }
   */
  //added by kh(072016)
  //#ifdef DATA_REMAP_OP
  if(g_hpcl_comp_config.hpcl_data_remap_op_en == 1) {
    //If the data is remapped.
    if(data_type > 0) {
      comp_bits += mf->get_remap_op_bits(data_type-1);
      //comp_data_bits_only += mf->get_remap_op_bits(data_type-1);
    }
    ///
  }
  //#endif

  DATA_SEG_COMP_DEBUG_PRINT("\tStep3: comp_bits = %u, comp_data_bits_only = %u\n", comp_bits, comp_data_bits_only);
}


void hpcl_comp_dsc_pl_proc::run_ds_comp_multi_res(mem_fetch* mf, unsigned max_res, unsigned data_type, std::vector<unsigned char>& cache_data,std::vector<unsigned>& enc_status, unsigned& comp_bits, unsigned& comp_data_bits_only)
{
  int cache_size = cache_data.size();
  int ds_no = cache_size/max_res;

  #ifdef DATA_SEG_COMP_DEBUG_PRINT
  DATA_SEG_COMP_DEBUG_PRINT("mf %u, Data_Type = %u, Max_Res = %u ", mf->get_request_uid(), data_type, max_res);
  DATA_SEG_COMP_DEBUG_PRINT("Data(%03d) = ", cache_size);
  for(int i = 0; i < cache_size; i++) {
      DATA_SEG_COMP_DEBUG_PRINT("%02x", cache_data[i]);
  }
  DATA_SEG_COMP_DEBUG_PRINT("\n");
  #endif

  enc_status.resize(ds_no, 0);
  comp_bits = 0;
  comp_data_bits_only = 0;

  unsigned dsc_comp_header_bits = 0; //1-bit compressed or not, 2-bit resolution size, 8-bit encoding status

  if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::DATA_SEG_LWM_HYBRID_PARL) {
    dsc_comp_header_bits += 1; //1-bit compressed or not
    dsc_comp_header_bits += 3; //3-bit data remapping type
    //dsc_comp_header_bits += 3; //3-bit resolution ( 2 ~ 128 )
    dsc_comp_header_bits += 8; //static ds status bits
  }
  comp_bits += dsc_comp_header_bits;
  comp_data_bits_only += dsc_comp_header_bits;

  DATA_SEG_COMP_DEBUG_PRINT("\tStep1: comp_bits = %u\n", comp_bits);
  int MIN_RES = 1;
  for(int i = 0; i < ds_no; i++)
  {
    int min_res = -1;
    int min_encoding_data = -1;
    //Search the highest resolution for the given segment
    for(int res = MIN_RES; res <= max_res; res=res*2)
    {
      int index = i * max_res;
      bool is_compressed = false;
      int encoding_data = 0;	//resolution bits
      unsigned csn_seg_no = 0;
      for(int seg = 0; seg < max_res/res; seg++)
      {
	bool is_all_byte_same = true;
	unsigned char byte_data = cache_data[index];
	//DATA_SEG_COMP_DEBUG_PRINT("\tbyte_data %u, cache_data[%d] %u, ", byte_data, index+j, cache_data[index+j]);

	for(int j = 1; j < res; j++)
	{
	  //DATA_SEG_COMP_DEBUG_PRINT("\tres %d, byte_data %u, cache_data[%d] %u, ", res, byte_data, index+j, cache_data[index+j]);
	  if(byte_data != cache_data[index+j]) {
	    is_all_byte_same = false;
	    //DATA_SEG_COMP_DEBUG_PRINT("diff!\n");
	    break;
	  } else {
	    //DATA_SEG_COMP_DEBUG_PRINT("same!\n");
	  }
	}

	if(is_all_byte_same == true) {
	  //DATA_SEG_COMP_DEBUG_PRINT("\t%d bytes in DS%d has all bytes same\n", res, i);
	  unsigned char high_nibble = (byte_data & 0xf0) >> 4;
	  unsigned char low_nibble = (byte_data & 0x0f);

	  //DATA_SEG_COMP_DEBUG_PRINT("\thigh_nibble %u low_nibble %u\n", high_nibble, low_nibble);
	  if(high_nibble == low_nibble) {
	    //is_compressed = true;
	    csn_seg_no++;
	  }
	}

	index += res;
      }

      if(csn_seg_no == 0 || (encoding_data > max_res*8))	encoding_data = max_res*8;
      else {
	encoding_data += 2;
	encoding_data += (max_res/res);
	encoding_data += csn_seg_no*encoding_bits;
	encoding_data += (max_res/res-csn_seg_no)*res*8;
      }

      //DATA_SEG_COMP_DEBUG_PRINT("\tDS%d : encoding_data %u, csn_seg_no %u res %d\n", i, encoding_data, csn_seg_no, res);


      if((min_encoding_data == -1) || (min_encoding_data > encoding_data)) {
	min_encoding_data = encoding_data;
	min_res = res;
      }
      //DATA_SEG_COMP_DEBUG_PRINT("\n");
    }
    //DATA_SEG_COMP_DEBUG_PRINT("\n");

    if(min_res < 0) {		//if segment is not compressibile.
      enc_status[i] = 0;
      comp_bits += min_encoding_data;
      //DATA_SEG_COMP_DEBUG_PRINT("\tDS%d is uncompressed\n", i);
    } else {
      enc_status[i] = 1;
      comp_bits += min_encoding_data;
      //DATA_SEG_COMP_DEBUG_PRINT("\tDS%d is compressed, encoding_data %u, res %d\n\n", i, min_encoding_data, min_res);
    }
 /*
    if(ds_res < 0) {		//if segment is not compressibile.
      enc_status[i] = 0;
      comp_bits += (max_res*8);
      DATA_SEG_COMP_DEBUG_PRINT("\tDS%d is uncompressed\n", i);
    } else {
      enc_status[i] = 1;
      //resolution bits(2) + encoding status bits + encoding bits
      unsigned encoding_data = 2 + (max_res/ds_res) + (max_res/ds_res)*4;
      comp_bits += encoding_data;
      DATA_SEG_COMP_DEBUG_PRINT("\tDS%d is compressed, encoding_data %u, res %d\n", i, encoding_data, ds_res);
    }
*/

  }

  DATA_SEG_COMP_DEBUG_PRINT("\tStep2: comp_bits = %u, comp_data_bits_only = %u\n", comp_bits, comp_data_bits_only);
  //#ifdef DATA_REMAP_OP
  if(g_hpcl_comp_config.hpcl_data_remap_op_en == 1) {
    //If the data is remapped.
    if(data_type > 0) {
      comp_bits += mf->get_remap_op_bits(data_type-1);
      comp_data_bits_only += mf->get_remap_op_bits(data_type-1);
    }
    ///
  }
  //#endif
  DATA_SEG_COMP_DEBUG_PRINT("\tStep3: comp_bits = %u, comp_data_bits_only = %u\n", comp_bits, comp_data_bits_only);
}







#endif /* HPCL_COMP_PL_PROC_H_ */
