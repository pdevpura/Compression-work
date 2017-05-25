
// added by abpd

#ifndef HPCL_COMP_FPH_PL_H_
#define HPCL_COMP_FPH_PL_H_

#include "hpcl_comp_fph_pl_proc.h"
#include "hpcl_comp_pl_data.h"

#include <vector>
#include <unordered_map>
#include <utility>

#include "huffman.h"
#define EXP_FILE "FPH_HUFFMAN_EXP_"
#define MH_FILE "FPH_HUFFMAN_MH_"
#define ML_FILE "FPH_HUFFMAN_ML_"
#define MAX_INST_COUNT 1000000000

#define FILE_SIZE 102 // change it inorder to adjust value
using namespace std;

template<class K>
class hpcl_comp_fph_pl {
private:
  unsigned m_id;
  unsigned m_pipeline_no;
  unsigned m_cur_state;
  // used for number of cycles , will try with 1 : TODO for rest
  std::vector<hpcl_comp_fph_pl_proc<K>*> m_comp_pl_proc;

  hpcl_comp_pl_data* m_input;
  hpcl_comp_pl_data* m_output;

  vector<mem_fetch*> m_comp_buffer;

  
public:

  hpcl_comp_fph_pl(unsigned pipeline_no, unsigned id,unsigned cur_state);
  ~hpcl_comp_fph_pl() {

	cout << " Committed Instructions "<<ins<<"\t"<<m_id<<endl;
	if(m_cur_state==0)
			write_data_file();
		  /* TMP commented , use of smart pointer 
    if(m_input)	 	delete m_input;
    if(m_output)	delete m_output;

		*/
    for(int i = 0; i < m_comp_pl_proc.size(); i++) {
	delete m_comp_pl_proc[i];
    }
  }
 unordered_map<string,long long> exp_track;
 unordered_map<string,long long> mh_track;
 unordered_map<string,long long> ml_track;
	 // convert data to huffman tree
 vector<pair<int,pair<hufman*,string> > > exp_tree;
 vector<pair<int,pair<hufman*,string> > > mh_tree;
 vector<pair<int,pair<hufman*,string> > > ml_tree;
 unordered_map<string,string> exp_code;
 unordered_map<string,string> mh_code;
 unordered_map<string,string> ml_code;

 // init trackers;
 unordered_map<string,string> exp_tracker;
 unordered_map<string,string> mh_tracker;
 unordered_map<string,string> ml_tracker;

 long long rep_count;
 long long t_nibles;
 long long ins;
  void create(unsigned buffer_size);
  void set_input(hpcl_comp_pl_data* input_data);
  hpcl_comp_pl_data* get_input();
  mem_fetch* run(int state,long long ins_count);
  void print(unsigned pl_index=0);
  
  mem_fetch* top_compressed_mem_fetch();
  void pop_compressed_mem_fetch();
  void print_extra_info(unordered_map<string,string>& code,vector<pair<int,string>>& track,string name );
  
private:
  unsigned m_comp_buffer_size;
public:
  bool has_comp_buffer_space();
  // FPH Specific functions
  void write_data_file();
  void write_file(string file_name,unordered_map<string,string> code);
  void traverse(hufman *root,string stack,unordered_map<string,string>& code);
  vector<string> split(string input,char del);
  static bool myfunc_dec(pair<int,pair<hufman*,string> > a,pair<int,pair<hufman*,string> > b);
  static bool myfunc_inc(pair<int,pair<hufman*,string> > a,pair<int,pair<hufman*,string> > b);
  hufman* construct_tree(vector<pair<int,pair<hufman*,string> > >& track);
  void fill_data(unordered_map<string,long long> src_track,vector<pair<int,pair<hufman*,string> > >& src_tree,vector<pair<int,string>>& track_exp);
  void generic_init();
  void init_tracker(string file_name,unordered_map<string,string>& src_tracker);
};
template<class K>
hpcl_comp_fph_pl<K>::hpcl_comp_fph_pl(unsigned pipeline_no, unsigned id,unsigned cur_state) {
  m_pipeline_no = pipeline_no;
  m_input = new hpcl_comp_pl_data;
  m_output = NULL;
  m_id = id;
  m_cur_state=cur_state;
  m_comp_buffer_size = -1;
  rep_count=0;
  t_nibles=0;
  ins=0;
  generic_init();
}

template<class K>
void hpcl_comp_fph_pl<K>::create(unsigned comp_buffer_size) 
{
	// create pipeline  -> 2 for fph
	for(unsigned i = 0; i < m_pipeline_no; i++) 
  	{
		m_comp_pl_proc.push_back(new hpcl_comp_fph_pl_proc<K>());
		m_comp_pl_proc[i]->set_pl_index(i);
  	}
	m_comp_pl_proc[0]->set_pl_type(hpcl_comp_fph_pl_proc<K>::GET_OUTPUT);
	for(unsigned i = 1; i < m_pipeline_no-1; i++)
	{
		m_comp_pl_proc[i]->set_pl_type(hpcl_comp_fph_pl_proc<K>::DUMMY);
	}
	m_comp_pl_proc[m_pipeline_no-1]->set_pl_type(hpcl_comp_fph_pl_proc<K>::COMP);
	for(unsigned i = 0; i < m_pipeline_no-1; i++)
	{
		m_comp_pl_proc[i]->set_output(new hpcl_comp_pl_data);
	}


	m_input = m_comp_pl_proc[m_pipeline_no-1]->get_input();
  	m_comp_pl_proc[m_pipeline_no-1]->set_output(new hpcl_comp_pl_data);
  	m_comp_buffer_size = comp_buffer_size;


}
template<class K>
void hpcl_comp_fph_pl<K>::init_tracker(string file_name,unordered_map<string,string>& src_tracker)
{
    ifstream ifile;
	ifile.open(file_name);
	string input;
	while(getline(ifile,input))
	{
	     vector<string> split_data = split(input,' ');
		 src_tracker.insert(make_pair(split_data[0],split_data[1]));
	}
}
template<class K>
void hpcl_comp_fph_pl<K>::generic_init()
{
	stringstream convert;
	convert << m_id;
	string exp_file = EXP_FILE+convert.str();
	string mh_file = MH_FILE+convert.str();
	string ml_file = ML_FILE+convert.str();

	init_tracker(exp_file,exp_tracker);
	init_tracker(mh_file,mh_tracker);
	init_tracker(ml_file,ml_tracker);

}
template<class K>
vector<string> hpcl_comp_fph_pl<K>::split(string input,char del)
{
	stringstream ss (input);
	string each;
	vector<string> res;
    while(getline(ss,each,del))
	{
		res.push_back(each);
	}
	return res;
}

// write results in file
template<class K>
void hpcl_comp_fph_pl<K>::write_file(string file_name,unordered_map<string,string> code)
{
	ofstream ofile;
    ofile.open(file_name);
    auto lt = code.begin();
    for(;lt!=code.end();lt++)
    {
		pair<string,string> p = *lt;
		ofile<<p.first<<" "<<p.second<<endl;
	}
}
// traverse to form act code
template<class K>
void hpcl_comp_fph_pl<K>::traverse(hufman *root,string stack,unordered_map<string,string>& code)
{
	if(!root) return;
    if(root->is_leaf)
	{
		code.insert(make_pair(root->symbol,stack));
		return;
    }
	traverse(root->left,stack+"0",code);
	traverse(root->right,stack+"1",code);
}
template<class K>
bool hpcl_comp_fph_pl<K>::myfunc_dec(pair<int,pair<hufman*,string> > a,pair<int,pair<hufman*,string> > b)
{
		        return a.first>b.first;
}
template<class K>
bool hpcl_comp_fph_pl<K>::myfunc_inc(pair<int,pair<hufman*,string> > a,pair<int,pair<hufman*,string> > b)
{
		        return a.first<b.first;
}
template<class K>
hufman* hpcl_comp_fph_pl<K>::construct_tree(vector<pair<int,pair<hufman*,string> > >& track)
{
    if(track.size()==0) return NULL;
	if(track.size()==1 && track[0].second.first!=NULL) return track[0].second.first;
	pair<int,pair<hufman*,string> > a = track[0];
	pair<int,pair<hufman*,string> > b = track[1];
	track.erase(track.begin());
	track.erase(track.begin());
	hufman *node = new hufman(false,"");
	node->val=a.first+b.first;
	hufman *one_node = a.second.first;
	hufman *two_node = b.second.first;
	if(!a.second.first)
    {
          one_node = new hufman(true,a.second.second);
		  one_node->val=a.first;
    }
	if(!b.second.first)
	{
	     two_node = new hufman(true,b.second.second);
	     two_node->val=b.first;
    }
	node->left=one_node;
	node->right=two_node;
	pair<int,pair<hufman*,string> > c;
	c.first=a.first+b.first;
	c.second.first=node;
	track.push_back(c);
	sort(track.begin(),track.end(),myfunc_inc);
	return construct_tree(track);
}

// void fill data
template<class K>
void hpcl_comp_fph_pl<K>::fill_data(unordered_map<string,long long> src_track,vector<pair<int,pair<hufman*,string> > >& src_tree,vector<pair<int,string>>& track)
{
    auto it = src_track.begin();
	for(;it!=src_track.end();it++)
	{
	    pair<string,long long> p = *it;
		pair<hufman*,string> sub;
		sub.first=NULL;
		sub.second.assign(p.first);
		pair<int,pair<hufman*,string> > nor;
		nor.first=p.second;
		nor.second=sub;
		src_tree.push_back(nor);
    }
	sort(src_tree.begin(),src_tree.end(),myfunc_dec);
	if(src_tree.size()>FILE_SIZE)
    {
   		src_tree.resize(FILE_SIZE);
	}
	for(auto it=src_tree.begin();it!=src_tree.end();it++){
		track.push_back(make_pair(it->first,it->second.second));
	}
    sort(src_tree.begin(),src_tree.end(),myfunc_inc);
}

template<class K>
void hpcl_comp_fph_pl<K>::set_input(hpcl_comp_pl_data* input_data) {
  m_input->copy(input_data);
}

template<class K>
hpcl_comp_pl_data* hpcl_comp_fph_pl<K>::get_input() {
  return m_input;
}
template<class K>
void hpcl_comp_fph_pl<K>::write_data_file()
{
	stringstream convert;
	convert << m_id;
	string exp_file = EXP_FILE+convert.str();
	string mh_file = MH_FILE+convert.str();
	string ml_file = ML_FILE+convert.str();
	vector<pair<int,string>>track_exp;
	vector<pair<int,string>>track_mh;
	vector<pair<int,string>>track_ml;
	fill_data(exp_track,exp_tree,track_exp);
	hufman *exp_hufman = construct_tree(exp_tree);
	traverse(exp_hufman,"",exp_code);
	write_file(exp_file,exp_code);
	fill_data(mh_track,mh_tree,track_mh);
	hufman *mh_hufman = construct_tree(mh_tree);
	traverse(mh_hufman,"",mh_code);
	write_file(mh_file,mh_code);
	fill_data(ml_track,ml_tree,track_ml);
	hufman *ml_hufman = construct_tree(ml_tree);
	traverse(ml_hufman,"",ml_code);
	print_extra_info(exp_code,track_exp,"EXP");
	print_extra_info(mh_code,track_mh,"MH");
	print_extra_info(ml_code,track_ml,"ML");
	write_file(ml_file,ml_code);
}
template<class K>
void hpcl_comp_fph_pl<K>::print_extra_info(unordered_map<string,string>& code,vector<pair<int,string>>& track,string name ){
#ifdef COMP_DEBUG
	ofstream file1;
	string file_name;
	stringstream ss;
	ss << m_id;
	file_name=name+"_fph_hufftree_data_"+ ss.str()+".txt";
	file1.open(file_name);
	for(auto it=track.begin();it!=track.end();it++){
		string str= it->second;
		file1<<it->first<<"\t"<<str<<"\t"<<code[str]<<endl;
	}
#endif
	return;
}


template<class K>
mem_fetch* hpcl_comp_fph_pl<K>::run(int state,long long inst_count)
{
  /*
  ins = inst_count;
  if(inst_count > MAX_INST_COUNT)
  {
    cout<<"Reached Max number of instructions "<<MAX_INST_COUNT<<" Stopping ...>>"<<endl;
    if(state ==0)
    write_data_file();
    exit(0);
  }
  */

  mem_fetch* ret = NULL;
  //deleted by kh(083116)
  //for(int i = 0; i < m_pipeline_no; i++)
  //added by kh(083116)
  for(int i = (m_pipeline_no-1); i >= 0; i--)
  {
    if(m_comp_pl_proc[i]->get_pl_type()==hpcl_comp_fph_pl_proc<K>::COMP)
    {
      m_comp_pl_proc[i]->run(state,rep_count,t_nibles,exp_track,mh_track,ml_track,exp_tracker,mh_tracker,ml_tracker);

      hpcl_comp_pl_data *tmp = m_comp_pl_proc[i]->get_output();
      m_comp_pl_proc[i-1]->get_output()->set_mem_fetch(tmp->get_mem_fetch());
      tmp->clean();
    }
    else if(m_comp_pl_proc[i]->get_pl_type()==hpcl_comp_fph_pl_proc<K>::GET_OUTPUT)
    {
      hpcl_comp_pl_data *tmp = m_comp_pl_proc[i]->get_output();
      assert(tmp);
      mem_fetch* mf = tmp->get_mem_fetch();
      tmp->clean();

      ret = mf;
    }
    else
    {
      hpcl_comp_pl_data *tmp = m_comp_pl_proc[i]->get_output();
      assert(tmp);
      mem_fetch* mf = tmp->get_mem_fetch();
      if(mf)
      {
	m_comp_pl_proc[i-1]->get_output()->set_mem_fetch(mf);
      }
      tmp->clean();
    }

  }

  return ret;
		
}

template<class K>
void hpcl_comp_fph_pl<K>::print(unsigned pl_index) {

}

template<class K>
mem_fetch* hpcl_comp_fph_pl<K>::top_compressed_mem_fetch() {
  if(m_comp_buffer.size() == 0)	return NULL;
  else				return m_comp_buffer[0];
}

template<class K>
void hpcl_comp_fph_pl<K>::pop_compressed_mem_fetch() {
  m_comp_buffer.erase(m_comp_buffer.begin());
}

template<class K>
bool hpcl_comp_fph_pl<K>::has_comp_buffer_space() {
  if(m_comp_buffer.size() >= m_comp_buffer_size)	return false;
  else							return true;
}


#endif /* HPCL_COMP_PL_H_ */
