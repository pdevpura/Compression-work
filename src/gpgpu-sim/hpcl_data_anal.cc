/*
 * hpcl_comp.cc
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#include "hpcl_data_anal.h"

#include "hpcl_user_define_stmts.h"


hpcl_data_anal::hpcl_data_anal ()
{
  // TODO Auto-generated constructor stub
}

hpcl_data_anal::~hpcl_data_anal ()
{
  // TODO Auto-generated destructor stub
}

void hpcl_data_anal::create(double char_type_threshold) 
{
	m_char_type_no = new hpcl_stat;
	m_nonchar_type_no = new hpcl_stat;
	m_char_type_threshold = char_type_threshold;
	
	m_zero_val_pat_no = new hpcl_stat;
	m_narrow_val_pat_no = new hpcl_stat;
	m_repeat_val_pat_no = new hpcl_stat;
	m_similar_val_1_25_byte_pat_no = new hpcl_stat;
	m_similar_val_26_50_byte_pat_no = new hpcl_stat;
	m_similar_val_51_75_byte_pat_no = new hpcl_stat;
	m_similar_val_76_100_byte_pat_no = new hpcl_stat;
	m_no_pattern_no = new hpcl_stat;

	//added by kh(030217)
	m_char_data_dist.resize(16);
	for(int i = 0; i < m_char_data_dist.size(); i++)		m_char_data_dist[i] = new hpcl_stat;
	m_char_type_pct_dist.resize(10);
	for(int i = 0; i < m_char_type_pct_dist.size(); i++)	m_char_type_pct_dist[i] = new hpcl_stat;
	///

}

void hpcl_data_anal::display(std::ostream & os) const
{
	os << "====== hpcl_data_anal ======" << std::endl;
	os << "gpu_tot_ovr_char_pct = " << m_char_type_no->sum() / (m_char_type_no->sum()+m_nonchar_type_no->sum())*100 << std::endl;
	os << "gpu_tot_ovr_char_no = " << m_char_type_no->sum() << std::endl;
	os << "gpu_tot_ovr_nonchar_no = " << m_nonchar_type_no->sum() << std::endl;
	os << "gpu_tot_ovr_zero_val_pat_pct = " << m_zero_val_pat_no->sum()/m_nonchar_type_no->sum()*100 << std::endl;
	os << "gpu_tot_ovr_narrow_val_pat_pct = " << m_narrow_val_pat_no->sum()/m_nonchar_type_no->sum()*100 << std::endl;
	os << "gpu_tot_ovr_rep_val_pat_pct = " << m_repeat_val_pat_no->sum()/m_nonchar_type_no->sum()*100 << std::endl;
	os << "gpu_tot_ovr_sim_val_1_25_byte_pat_pct = " << m_similar_val_1_25_byte_pat_no->sum()/m_nonchar_type_no->sum()*100 << std::endl;
	os << "gpu_tot_ovr_sim_val_26_50_byte_pat_pct = " << m_similar_val_26_50_byte_pat_no->sum()/m_nonchar_type_no->sum()*100 << std::endl;
	os << "gpu_tot_ovr_sim_val_51_75_byte_pat_pct = " << m_similar_val_51_75_byte_pat_no->sum()/m_nonchar_type_no->sum()*100 << std::endl;
	os << "gpu_tot_ovr_sim_val_76_100_byte_pat_pct = " << m_similar_val_76_100_byte_pat_no->sum()/m_nonchar_type_no->sum()*100 << std::endl;
	os << "gpu_tot_ovr_no_pat_pct = " << m_no_pattern_no->sum()/m_nonchar_type_no->sum()*100 << std::endl;

	//added by kh(030217)
	os << "gpu_tot_char_val_dist = ";
	for(int i = 0; i < m_char_data_dist.size(); i++) {
	   os << m_char_data_dist[i]->avg() << " ";
	}
	os << std::endl;

	os << "gpu_tot_char_type_pct_dist = ";
	double total_case = 0;
	for(int i = 0; i < m_char_type_pct_dist.size(); i++) {
		total_case += m_char_type_pct_dist[i]->sum();
	}
	for(int i = 0; i < m_char_type_pct_dist.size(); i++) {
		os << m_char_type_pct_dist[i]->sum()/total_case*100 << " ";
	}
	os << std::endl;
	///

	os << "===============================" << std::endl;
}

int hpcl_data_anal::analyze_data_type(std::vector<unsigned char>& data, bool is_char, double char_type_pct)
{
	int ret = -1;

	//added by kh(030217)
	int index = char_type_pct/10;
	index = (index == 10) ? 9 : index;
	m_char_type_pct_dist[index]->add_sample(1);
	///

	//double char_type_pct = get_char_type_pct(data);
	//if(char_prob >= m_char_type_threshold) {
	if(is_char == true) {

		m_char_type_no->add_sample(1); 
		DATA_TYEP_CHECK_DEBUG_PRINT("--> CHAR DATA\n");
		ret = 0;

//		//added by kh(030217)
//		printf("Data = ");
//		for(int i = 0; i < data.size(); i++) {
//			printf("%02x", data[i]);
//		}
//		printf("\n");

		//measure the statistics of character data
		std::vector<double> counter(16,0);
		int all_elem_no = data.size();
		for(int i = 0; i < all_elem_no; i++) {
			unsigned char byte_data = data[i];
			byte_data = (byte_data >> 4);
			counter[byte_data]++;
		}
		for(int i = 0; i < counter.size(); i++) {
			//counter[i] = (counter[i]/all_elem_no*100);
			//g_hpcl_comp_anal->add_sample(hpcl_comp_anal::CHAR_DATA_DIST, counter[i]/all_elem_no*100, i);
			double ratio = counter[i]/all_elem_no*100;
			m_char_data_dist[i]->add_sample(ratio);
			//printf("char %d freq %f ratio %f\n", i, counter[i], ratio);
		}
		///
		//assert(0);
	} 
	else {

		ret = 1;
		m_nonchar_type_no->add_sample(1);	
		
		DATA_TYEP_CHECK_DEBUG_PRINT("--> NON-CHAR DATA, ");
		
		enum int_data_redundancy_type type = get_int_redundancy_type_pct(data);
		if(type == ZERO_VAL_PATTERNS) {
			DATA_TYEP_CHECK_DEBUG_PRINT("ZERO-VAL-PATTERN\n");
			m_zero_val_pat_no->add_sample(1);
		}
		else if(type == NARROW_VAL_PATTERNS) {
			DATA_TYEP_CHECK_DEBUG_PRINT("NARROW-VAL-PATTERN\n");
			m_narrow_val_pat_no->add_sample(1);
		}
		else if(type == REPEAT_VAL_PATTERNS) {
			DATA_TYEP_CHECK_DEBUG_PRINT("REPEAT-VAL-PATTERN\n");
			m_repeat_val_pat_no->add_sample(1);
		}
		else if(type == SIMILARITY_VAL_1_25_BYTE_PATTERNS) 		{
			DATA_TYEP_CHECK_DEBUG_PRINT("SIM-VAL-1-25-PATTERN\n");
			m_similar_val_1_25_byte_pat_no->add_sample(1);
		}
		else if(type == SIMILARITY_VAL_26_50_BYTE_PATTERNS) 	{
			DATA_TYEP_CHECK_DEBUG_PRINT("SIM-VAL-26-50-PATTERN\n");
			m_similar_val_26_50_byte_pat_no->add_sample(1);
		}
		else if(type == SIMILARITY_VAL_51_75_BYTE_PATTERNS) 	{
			DATA_TYEP_CHECK_DEBUG_PRINT("SIM-VAL-51-75-PATTERN\n");
			m_similar_val_51_75_byte_pat_no->add_sample(1);
		}
		else if(type == SIMILARITY_VAL_76_100_BYTE_PATTERNS) 	{
			DATA_TYEP_CHECK_DEBUG_PRINT("SIM-VAL-76-100-PATTERN\n");
			m_similar_val_76_100_byte_pat_no->add_sample(1);
		}
		else if(type == NO_PATTERNS) 	{
			DATA_TYEP_CHECK_DEBUG_PRINT("NO-PATTERN\n");
			m_no_pattern_no->add_sample(1);
			ret = -1;
		}
		else	assert(0);
	}
	
	return ret;
}

//double hpcl_data_anal::get_char_type_pct(std::vector<unsigned char>& data)
//{
//	int all_elem_no = data.size();
//	int char_elem_no = 0;
//	for(int i = 0; i < all_elem_no; i++) {
//		bool is_char = true;
//		if(data[i] >= 0 && data[i] < 32) {
//			if(data[i] != 9 && data[i] != 10 && data[i] != 13) {	//Not tab, line feed, carriage return
//				is_char = false;
//			}
//		} else if(data[i] >= 32 && data[i] < 128) {
//			if(data[i] == 127)	is_char = false;
//		} else {
//			is_char = false;
//		}
//
//		if(is_char == true) {
//			char_elem_no++;
//		}
//	}
//
//	return (double)char_elem_no/(double)all_elem_no*100;
//}

enum hpcl_data_anal::int_data_redundancy_type 
hpcl_data_anal::get_int_redundancy_type_pct(std::vector<unsigned char>& data)
{
	//Check similarity among bytes at the same positions across all 4B elements
	std::map<unsigned char, int> byte_checker[4];
	for(int m = 0; m < 4; m++) {
		for(int i = m; i < data.size(); i = i+4) {
			std::map<unsigned char, int>::iterator it = byte_checker[m].find(data[i]);
			if(it == byte_checker[m].end()) {
				byte_checker[m][data[i]] = 1;
			} else {
				byte_checker[m][data[i]]++;
			}
			//printf("m: %d Byte_Val: 0x%02x\n", m, data[i]);
		}
	}
	
	//Get the maximum similarity degree in each byte position
	double all_elem_no = data.size()/4;
	double most_freq_byte_pct[4] = {0,0,0,0};
	unsigned char most_freq_byte_val[4] = {0,0,0,0};
	for(int m = 0; m < 4; m++) {
		int most_freq_byte_no = -1;
		unsigned char most_freq_byte = 0;
		std::map<unsigned char, int>::iterator it = byte_checker[m].begin();
		for(; it != byte_checker[m].end(); ++it) {
			
			//std::cout << "byte " << it->first << " no " << it->second << std::endl;
			//std::cout << "byte " << it->first << " no " << it->second << std::endl;
			//printf("Byte_Val: 0x%02x, Freq: %d\n", it->first,it->second);
			
			if(most_freq_byte_no < it->second) {
				most_freq_byte_no = it->second;
				most_freq_byte = it->first;
			}
		}
		
		//std::cout << "most_freq_byte_no " << most_freq_byte_no << std::endl;
		assert(most_freq_byte_no > 0);
		most_freq_byte_pct[m] = most_freq_byte_no/all_elem_no*100;
		most_freq_byte_val[m] = most_freq_byte;
	}
	///
	
	//If all bytes with zeros are same, it is all-zeros
	bool is_all_zero_pattern = true;
	for(int m = 0; m < 4; m++) {
		if(!(most_freq_byte_pct[m] == 100 && most_freq_byte_val[m] == 0)) {
			is_all_zero_pattern = false;
			break;
		}
	}
	if(is_all_zero_pattern == true)	return ZERO_VAL_PATTERNS;
		
	//If all bytes are same, it is repeating values 
	bool is_repeating = true;
	for(int m = 0; m < 4; m++) {
		if(most_freq_byte_pct[m] != 100) {
			is_repeating = false;
			break;
		}
	}
	if(is_repeating == true)	return REPEAT_VAL_PATTERNS;

	//If all MSBs of a cache block are zeros, it has the narrow value pattern.
	bool is_narrow_val = false;
	if(most_freq_byte_pct[3] == 100 && most_freq_byte_val[3] == 0) {
		is_narrow_val = true;
	}
	if(is_narrow_val == true)	return NARROW_VAL_PATTERNS;

	//If average similarity is more than 50
	double ovr_avg_redund_pct = 0;
	for(int m = 0; m < 4; m++) {
		double redund_byte_elem_no = 0;
		std::map<unsigned char, int>::iterator it = byte_checker[m].begin();
		for(; it != byte_checker[m].end(); ++it) {
			if(it->second > 1) {
				redund_byte_elem_no += it->second;
			}
		}
		ovr_avg_redund_pct += (redund_byte_elem_no/all_elem_no*100);
	}
	ovr_avg_redund_pct /= 4;


	if(ovr_avg_redund_pct >= 1 && ovr_avg_redund_pct <= 25)					return SIMILARITY_VAL_1_25_BYTE_PATTERNS;
	else if(ovr_avg_redund_pct >= 26 && ovr_avg_redund_pct <= 50) 	return SIMILARITY_VAL_26_50_BYTE_PATTERNS;
	else if(ovr_avg_redund_pct >= 51 && ovr_avg_redund_pct <= 75)		return SIMILARITY_VAL_51_75_BYTE_PATTERNS;
	else if(ovr_avg_redund_pct >= 76 && ovr_avg_redund_pct <= 100)	return SIMILARITY_VAL_76_100_BYTE_PATTERNS;
	else 																														return NO_PATTERNS;
		
	assert(0);	
}

void hpcl_data_anal::decompose_data(std::vector<unsigned char>& data, std::vector<unsigned int>& word_list)
{
  for(unsigned i = 0; i < data.size(); i=i+sizeof(unsigned int)) {
    unsigned int word_candi = 0;
    //printf("decomposed %u\n", i);
    for(int j = sizeof(unsigned int)-1; j >= 0; j--) {
      unsigned tmp = data[i+j];
      tmp = (tmp << (8*j));
      word_candi += tmp;
      //printf("\t%x %x %x\n", cache_data[i+j], tmp, word_candi);
    }
    word_list.push_back(word_candi);
    //printf("\tword -- ");
    //hpcl_dict_elem<K>::print_word_data(word_candi);
    //printf("\n");
  }

  //printf("word_list_size %u, test %u\n",word_list.size(), cache_data.size()/sizeof(K));
  //assert(word_list.size() == data.size()/sizeof(unsigned int));
}

