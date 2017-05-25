
// dded by abpd

#ifndef HPCL_COMP_SC2_PL_H_
#define HPCL_COMP_SC2_PL_H_

#include "hpcl_comp_sc2_pl_proc.h"
#include "hpcl_comp_pl_data.h"

#include <vector>
#include <unordered_map>
#include <utility>
#include <memory>
using namespace std;

#define MAX_INST 20000000
#define VFT_SIZE 1168 // 7kb
#define WORD_SIZE 4 // in bytes
#define SC_FILE_NAME "SC2_HUFFMAN_DATA_"
#define SC_FILE_NAME2 "SC2_HUFFMAN_LIST_DATA_"

// HUFFAMN TREE DS
struct Tree
{	          
	public:
	bool is_symbol;
	string symbol;
	shared_ptr<Tree> left;
	shared_ptr<Tree> right;
	int val;
	Tree(bool is,string sy)
	{
		is_symbol=is;
		symbol.assign(sy);
		val=0;
	}
};
template<class K>
class hpcl_comp_sc2_pl {
private:
  unsigned m_id;
  unsigned m_pipeline_no;
  unsigned m_cur_state;
  // used for number of cycles , will try with 1 : TODO for rest
  std::vector<hpcl_comp_sc2_pl_proc<K>*> m_comp_pl_proc;

  hpcl_comp_pl_data* m_input;
  hpcl_comp_pl_data* m_output;

  vector<mem_fetch*> m_comp_buffer;
  bool do_sam;
  
public:

  hpcl_comp_sc2_pl(unsigned pipeline_no, unsigned id,unsigned cur_state);
  ~hpcl_comp_sc2_pl() {

   cout << " Committed Instructions : " << ins_ct << " M_ID : "<<m_id<<endl;
  // if(m_cur_state==0) // sampling state only
  // write_data_file();
	/* TMP Commented -> need to change to smart ptr 
    if(m_input)	 	delete m_input;
    if(m_output)	delete m_output;
	*/

    for(int i = 0; i < m_comp_pl_proc.size(); i++) {
	delete m_comp_pl_proc[i];
    }
  }
  unordered_map<string,long long> word_table;
  unordered_map<string,string> tracker; 
  long long rep_count;
  long long t_nibles;
  long long ins_ct;
  void create(unsigned buffer_size);
  void set_input(hpcl_comp_pl_data* input_data);
  hpcl_comp_pl_data* get_input();
  mem_fetch* run(long long inst_count,long long iter_sam,long long iter_enc);
  void print(unsigned pl_index=0);
  
  mem_fetch* top_compressed_mem_fetch();
  void pop_compressed_mem_fetch();
  void write_data_file();
  void traverse(shared_ptr<Tree> root,string stack,unordered_map<string,string>& code);
  void write_file(unordered_map<string,string> code);
 static bool myfunc_inc(pair<int,pair<shared_ptr<Tree>,string> > a,pair<int,pair<shared_ptr<Tree>,string> > b);
  static bool myfunc_dec(pair<int,pair<shared_ptr<Tree>,string> > a,pair<int,pair<shared_ptr<Tree>,string> > b);
  shared_ptr<Tree> gen_hf_tree(vector<pair<int,pair<shared_ptr<Tree>,string> > >& track);
  void pick_up_size(vector<pair<int,pair<shared_ptr<Tree>,string> > >& track);
private:
  unsigned m_comp_buffer_size;
public:
  bool has_comp_buffer_space();
  // encoding func
  void form_tracker();
  vector<string> split(string s,char del);

};
template<class K>
hpcl_comp_sc2_pl<K>::hpcl_comp_sc2_pl(unsigned pipeline_no, unsigned id,unsigned cur_state) {
  m_pipeline_no = pipeline_no;
  m_input = new hpcl_comp_pl_data;
  m_output = NULL;
  m_cur_state = cur_state;
  m_id = id;
  m_comp_buffer_size = -1;
  rep_count=0;
  t_nibles=0;
  ins_ct=0;
  do_sam=false;
}
// generic function
template<class K>
vector<string> hpcl_comp_sc2_pl<K>::split(string s,char del)
{
    stringstream ss(s);
	string item;
	vector<string> res;
	while(getline(ss,item,del))
    {
          res.push_back(item);
    }
	return res;
}
//ENCODING PHASE FUNCTION
template<class K>
void hpcl_comp_sc2_pl<K>::form_tracker()
{
	stringstream convert;
	convert << m_id;
    string file_name = SC_FILE_NAME+convert.str();

	ifstream ifile;
	ifile.open(file_name);
	string input;
	while(getline(ifile,input))
    {
    	vector<string> tmp = split(input,' ');
		if(tmp.size()>=2)
	    tracker.insert(make_pair(tmp[0],tmp[1]));
    }
	ifile.close();
}

// write data to file for next phase
template<class K>
void hpcl_comp_sc2_pl<K>::write_file(unordered_map<string,string> code)
{
	stringstream convert;
	convert << m_id;
    string file_name = SC_FILE_NAME+convert.str();
	ofstream ofile;
    ofile.open(file_name);
	auto lt = code.begin();
	for(;lt!=code.end();lt++)
	{
	    pair<string,string> p = *lt;
		ofile<<p.first<<" "<<p.second<<endl;
    }
}
// compare functions

template<class K>
bool hpcl_comp_sc2_pl<K>::myfunc_inc(pair<int,pair<shared_ptr<Tree>,string> > a,pair<int,pair<shared_ptr<Tree>,string> > b)
{
	return a.first<b.first;
}

template<class K>
bool hpcl_comp_sc2_pl<K>::myfunc_dec(pair<int,pair<shared_ptr<Tree>,string> > a,pair<int,pair<shared_ptr<Tree>,string> > b)
{
	return a.first>b.first;
}
// main function to generate huffman tree

template<class K>
shared_ptr<Tree> hpcl_comp_sc2_pl<K>::gen_hf_tree(vector<pair<int,pair<shared_ptr<Tree>,string> > >& track)
{
    if(track.size()==0) return shared_ptr<Tree>();
    if(track.size()==1)
	{
			pair<shared_ptr<Tree>,string> p = track[0].second;
			if(!p.first) {
					shared_ptr<Tree> nd(new Tree("true",p.second));
					return nd;
			} else {
					return track[0].second.first;
			}
			
	}
	pair<int,pair<shared_ptr<Tree>,string> > a = track[0];
	pair<int,pair<shared_ptr<Tree>,string> > b = track[1];
	track.erase(track.begin());
	track.erase(track.begin());
	shared_ptr<Tree> node(new Tree(false,""));
	node->val=a.first+b.first;
	shared_ptr<Tree> one_node(a.second.first);
	shared_ptr<Tree> two_node(b.second.first);
	if(!a.second.first)
    {
        one_node.reset(new Tree(true,a.second.second));
	    one_node->val=a.first;  
	}
	if(!b.second.first)
    {
   		two_node.reset(new Tree(true,b.second.second));
   	    two_node->val=b.first;
   	}
	node->left=one_node;
	node->right=two_node;
	pair<int,pair<shared_ptr<Tree>,string> > c;
	c.first=a.first+b.first;
	c.second.first=node;
	track.push_back(c);
	sort(track.begin(),track.end(),myfunc_inc);
	return gen_hf_tree(track);      																		                                        
}
// sampling phase function
template<class K>
void hpcl_comp_sc2_pl<K>::traverse(shared_ptr<Tree> root,string stack,unordered_map<string,string>& code)
{
	if(!root) return;
	if(root->is_symbol)
	{
		code.insert(make_pair(root->symbol,stack));
		return;
	}
	traverse(root->left,stack+"0",code);
	traverse(root->right,stack+"1",code);
}

template<class K>
void hpcl_comp_sc2_pl<K>::pick_up_size(vector<pair<int,pair<shared_ptr<Tree>,string> > >& track)
{
	int max_vft_enteries  = VFT_SIZE/WORD_SIZE;
	max_vft_enteries = track.size()>max_vft_enteries ? max_vft_enteries : track.size();
	vector<pair<int,pair<shared_ptr<Tree>,string> > > tmp;
	tmp.insert(tmp.end(),track.begin(),track.begin()+max_vft_enteries);
	track.clear();
	track.insert(track.end(),tmp.begin(),tmp.end());
}

// sampling phase function
template<class K>
void hpcl_comp_sc2_pl<K>::write_data_file()
{
	vector<pair<int,pair<shared_ptr<Tree>,string> > > track;
		stringstream convert;
	convert << m_id;
    string file_name2 = SC_FILE_NAME2+convert.str();
	//ofstream ofile;
    //ofile.open(file_name2);
	
	auto it = word_table.begin();
	for(;it!=word_table.end();it++)
	{
			//ofile<<it->first<<" "<<it->second<<endl;
			shared_ptr<Tree> ptr;
			track.push_back(make_pair(it->second,make_pair(ptr,it->first)));
	}

	// Pick up only that fits VFT size 7KB
	sort(track.begin(),track.end(),myfunc_dec);
	pick_up_size(track);
	#ifdef COMP_DEBUG
	vector<pair<int,string>> track_data;
	for(auto it=track.begin();it!=track.end();it++){
		track_data.push_back(make_pair(it->first,it->second.second));
	}
	#endif

	sort(track.begin(),track.end(),myfunc_inc);
	//cout << track.size() <<endl;
	shared_ptr<Tree> root = gen_hf_tree(track);
	unordered_map<string,string> code;
	string stack;
	//if(!root) cout << "NULL "<<endl;
	traverse(root,stack,code);
	#ifdef COMP_DEBUG
	ofstream file1;
	string file_name;
	stringstream ss;
	ss << m_id;
	file_name="sc2_huff_tree_data_"+ ss.str()+".txt";
	file1.open(file_name);
	for(auto it=track_data.begin();it!=track_data.end();it++){
		string str= it->second;
		file1<<it->first<<"\t"<<str<<"\t"<<code[str]<<endl;
	}
#endif

	write_file(code);
}
template<class K>
void hpcl_comp_sc2_pl<K>::create(unsigned comp_buffer_size) 
{
	// create pipeline  -> 3 for sc2
	for(unsigned i = 0; i < m_pipeline_no; i++) 
  	{
		m_comp_pl_proc.push_back(new hpcl_comp_sc2_pl_proc<K>());
		m_comp_pl_proc[i]->set_pl_index(i);
  	}
	m_comp_pl_proc[0]->set_pl_type(hpcl_comp_sc2_pl_proc<K>::GET_OUTPUT);
	for(unsigned i = 1; i < m_pipeline_no-1; i++)
	{
		m_comp_pl_proc[i]->set_pl_type(hpcl_comp_sc2_pl_proc<K>::DUMMY);
	}
	m_comp_pl_proc[m_pipeline_no-1]->set_pl_type(hpcl_comp_sc2_pl_proc<K>::COMP);
	for(unsigned i = 0; i < m_pipeline_no-1; i++)
	{
		m_comp_pl_proc[i]->set_output(new hpcl_comp_pl_data);
	}


	m_input = m_comp_pl_proc[m_pipeline_no-1]->get_input();
  	m_comp_pl_proc[m_pipeline_no-1]->set_output(new hpcl_comp_pl_data);
  	m_comp_buffer_size = comp_buffer_size;


}

template<class K>
void hpcl_comp_sc2_pl<K>::set_input(hpcl_comp_pl_data* input_data) {
  m_input->copy(input_data);
}

template<class K>
hpcl_comp_pl_data* hpcl_comp_sc2_pl<K>::get_input() {
  return m_input;
}
template<class K>
mem_fetch* hpcl_comp_sc2_pl<K>::run(long long inst_count,long long iter_sam,long long iter_enc)
{
	int comp_state;
	assert(iter_sam<iter_enc);
	ins_ct = inst_count;
	if(ins_ct < iter_sam) {
		comp_state=0;
	}else if( ins_ct >= iter_sam && ins_ct < iter_enc ) {
			if(do_sam==false) {
			cout << " Reached Max  Sam Instruction Mark, End sampling  "<<iter_sam << m_id<<endl;
			write_data_file();
			form_tracker();
			do_sam=true;
			}
			comp_state=1;
	}
	else {
			// stop code
			assert(0);
	}
	
    mem_fetch* ret = NULL;
	for(int i = 0; i < m_pipeline_no; i++) 
	{
		if(m_comp_pl_proc[i]->get_pl_type()==hpcl_comp_sc2_pl_proc<K>::COMP)
		{
			m_comp_pl_proc[i]->run(word_table,comp_state,rep_count,t_nibles,tracker,m_id);
			
		 	hpcl_comp_pl_data *tmp = m_comp_pl_proc[i]->get_output();
			if(m_pipeline_no==1)
			{
				assert(tmp);
				mem_fetch* mf = tmp->get_mem_fetch();
				ret = mf;
			}
			else
			m_comp_pl_proc[i-1]->get_output()->set_mem_fetch(tmp->get_mem_fetch());
			tmp->clean();
			
		}
		else if(m_comp_pl_proc[i]->get_pl_type()==hpcl_comp_sc2_pl_proc<K>::GET_OUTPUT)
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
void hpcl_comp_sc2_pl<K>::print(unsigned pl_index) {

}

template<class K>
mem_fetch* hpcl_comp_sc2_pl<K>::top_compressed_mem_fetch() {
  if(m_comp_buffer.size() == 0)	return NULL;
  else				return m_comp_buffer[0];
}

template<class K>
void hpcl_comp_sc2_pl<K>::pop_compressed_mem_fetch() {
  m_comp_buffer.erase(m_comp_buffer.begin());
}

template<class K>
bool hpcl_comp_sc2_pl<K>::has_comp_buffer_space() {
  if(m_comp_buffer.size() >= m_comp_buffer_size)	return false;
  else							return true;
}


#endif /* HPCL_COMP_PL_H_ */
