
// added by abpd

#ifndef HPCL_COMP_SC2_PL_PROC_H_
#define HPCL_COMP_SC2_PL_PROC_H_

#include "hpcl_comp_pl_data.h"
#include "hpcl_comp_data.h"
#include <vector>
#include <cassert>
#include <iostream>
#include <cmath>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <unordered_set>
#include<memory>


using namespace std;

#define ABPD_DEBUG 0
#define WORD_LEN 8
template<class K>
class hpcl_comp_sc2_pl_proc {
private:
  hpcl_comp_pl_data* m_input;
  hpcl_comp_pl_data* m_output;

  int m_pl_index;

public:
  hpcl_comp_sc2_pl_proc();
  ~hpcl_comp_sc2_pl_proc() {
	//cout<<"Deleting shared ptr\n";
    if(m_input)	 	delete m_input;
    if(m_output)	delete m_output;
      };
  void set_pl_index(int pl_index);
  void set_output(hpcl_comp_pl_data* output);
  hpcl_comp_pl_data* get_output();
  hpcl_comp_pl_data* get_input();
  void reset_output();

private:
  int m_type;
  void decompose_data(std::vector<unsigned char>& cache_data, string& packet);
  string int_to_hex( K i );
  unsigned m_id;
 


public:
  void run(unordered_map<string,long long>& word_table,int state,long long& rep_count,long long& t_nibles,unordered_map<string,string> tracker,unsigned id);
  mem_fetch* get_mf_input();
  void set_mf_input(mem_fetch *mf);
  bool fetch_data;
  bool execute_data;
  void set_pl_type(int type);
  int get_pl_type();
  string ss_encode(string input,unordered_map<string,string> tracker,float& bits,long long& rep_count);
  void update_word_table(string packet,unordered_map<string,long long>& word_table);
  enum 
  {
	  NONE=0,
	  DUMMY,
	  COMP,
	  GET_OUTPUT,
  };

};

template <class K>
hpcl_comp_sc2_pl_proc<K>::hpcl_comp_sc2_pl_proc() {
  m_input = new hpcl_comp_pl_data;
  m_output = NULL;
  m_pl_index = -1;
  m_type=NONE;
}
template <class K>
void hpcl_comp_sc2_pl_proc<K>::set_pl_type(int type) {
  m_type = type;
}
template <class K>
int hpcl_comp_sc2_pl_proc<K>::get_pl_type() {
  return m_type;
}

template <class K>
void hpcl_comp_sc2_pl_proc<K>::set_pl_index(int pl_index) {
  m_pl_index = pl_index;
}

template <class K>
void hpcl_comp_sc2_pl_proc<K>::set_output(hpcl_comp_pl_data* output) {
  m_output = output;
}

template <class K>
hpcl_comp_pl_data* hpcl_comp_sc2_pl_proc<K>::get_output() {
  return m_output;
}

template <class K>
hpcl_comp_pl_data* hpcl_comp_sc2_pl_proc<K>::get_input() {
  return m_input;
}
template<class K>
string hpcl_comp_sc2_pl_proc<K>::int_to_hex( K i )
{
  stringstream stream;
  stream << std::setfill ('0') << std::setw(sizeof(K)*2) 
	  << std::hex << i;
  return stream.str();
}
template <class K>
void hpcl_comp_sc2_pl_proc<K>::decompose_data(vector<unsigned char>& cache_data, string& packet)
{
  for(unsigned i = 0; i < cache_data.size(); i=i+sizeof(K)) {
    K word_candi = 0;
    for(int j = sizeof(K)-1; j >= 0; j--) {
	         K tmp = cache_data[i+j];
      tmp = (tmp << (8*j));

      word_candi += tmp;
    }
    packet+=int_to_hex(word_candi);
  }
}

// added by abpd
template <class K>
mem_fetch* hpcl_comp_sc2_pl_proc<K>::get_mf_input() {
	mem_fetch* mf = m_input->get_mem_fetch();

  return mf;
}
template <class K>
void hpcl_comp_sc2_pl_proc<K>::set_mf_input(mem_fetch *mf) {

	m_input->set_mem_fetch(mf);
}

template <class K>
void hpcl_comp_sc2_pl_proc<K>::reset_output() {
  m_output->clean();
}

template<class K>
void hpcl_comp_sc2_pl_proc<K>::update_word_table(string packet,unordered_map<string,long long>& word_table)
{
		for(int i=0;i<packet.size();i+=WORD_LEN)
		{			
				string each_word = packet.substr(i,WORD_LEN);
				#ifdef COMP_DEBUG
				cout<< "DEBUG::m_id: "<<m_id<<" WORD CT: "<<each_word<<endl;
				#endif
				if(word_table.find(each_word)==word_table.end())
						word_table.insert(make_pair(each_word,1));
				else
						word_table[each_word]++;
		}
}
// encode function
template<class K>
string hpcl_comp_sc2_pl_proc<K>::ss_encode(string input,unordered_map<string,string> tracker,float& bits,long long& rep_count)
{
	// handled for 4 byte word resolution
	// 0 with HF , 1 without HF
	string output;
    for(int i=0;i<input.size();i+=8)
	{
       string ss = input.substr(i,8);
	   if(tracker.find(ss)!=tracker.end()){
	   #ifdef COMP_DEBUG
			string out_word;
			out_word= tracker[ss];
			int word_sz = tracker[ss].size();
			cout<<i/8<<" INPUT WORD: "<<ss<<" COMP WORD: "<<out_word<<" size: "<<word_sz <<endl;
		#endif
			//0 if compressed and 1 if not compressed
	  		output+= ("0" + tracker[ss]);
	  		//output+= (tracker[ss]);
	        bits+=(1+tracker[ss].size());
	        //bits+=(tracker[ss].size());
	   }
	   else
	   {
			   //PD commented
	   		output+=("1" + ss);
	   		//output+=( ss);
	   		bits+=33;
	   		//bits+=32;
				#ifdef COMP_DEBUG
			string out_word;
			out_word= ("1"+ss);
			int word_sz = 33;
			cout<< i/8<<" INPUT WORD: "<<ss<<" COMP WORD: "<<out_word<<" size: "<<word_sz <<endl;
			#endif

			// Uncompressed state
			unordered_map<char,int> rep_track;
			/*for(int i=0;i<ss.size();i++)
			{
					char c = ss[i];
					if(rep_track.find(c)==rep_track.end()) rep_track.insert(make_pair(c,1));
					else rep_track[c]++;
			}
			auto it = rep_track.begin();
			for(;it!=rep_track.end();it++)
			{
						pair<char,int> p = *it;
						if(p.second>=2) rep_count+=p.second;
			}*/
	   }
    }
	return output;
}

// added by abpd
template<class K>
void hpcl_comp_sc2_pl_proc<K>::run(unordered_map<string,long long>& word_table,int state,long long& rep_count,long long& t_nibles,unordered_map<string,string> tracker,unsigned id)
{
	m_id=id;
	mem_fetch* mf = m_input->get_mem_fetch();
  	m_output->set_mem_fetch(mf);

  	if(!mf) return;

  	//If mf is not type for compression,
  	if(mf->get_real_data_size() == 0)	return;

  	std::vector<unsigned char>& cache_data = mf->get_real_data_ptr();
	std::string packet;
	decompose_data(cache_data,packet);

	transform(packet.begin(),packet.end(),packet.begin(),::toupper);
	mf->compare_data.assign(packet);
	float bits =packet.size()*4;

	t_nibles+=packet.size();
	// CODE HERE
	if(state == 0) 
	{
			// SAMPLING STATE
			update_word_table(packet,word_table);
	}
	else
	{
			bits=0.0;
			// ENCODING STATE
			string output = ss_encode(packet,tracker,bits,rep_count);
			string file_name ="rep_count_track_sc2.txt";
			ofstream ofile;
			ofile.open(file_name);
			double per = (1.0*rep_count)/t_nibles;
			ofile<<"Rep Count : "<<rep_count<<"\t"<<"Total Nibles : "<<t_nibles<<"\t"<<"Per : "<<per<<endl;
			ofile.close();
	}
	
	double bytes_count = bits/8.0;
	#ifdef COMP_DEBUG
	cout<<"----FINAL COMP BYTES:"<<bytes_count<<"---"<<endl;
#endif

	unsigned comp_byte_size = ceil((double)bytes_count);
	mf->set_comp_data_size(comp_byte_size);
	//mf->set_comp_data_size(mf->get_real_data_size());
	mf->init_size();

	//added by kh(071616)
	mf->set_comp_data_bits(comp_byte_size*8);


}

#endif /* HPCL_COMP_PL_PROC_H_ */
