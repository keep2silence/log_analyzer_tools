#ifndef __BIZ_FILTER_H__
#define __BIZ_FILTER_H__

#include <vector>
#include <list>
#include <stdint.h>
#include <string>
#include <unordered_map>

#define TODO \
do {	\
	printf ("%s:%d this function will implement in the future.\n", __FILE__, __LINE__); \
} while (0)

/// һ��table����N��chain��һ��chain��N��rule��һ��rule����N��match��
/// һ��rule��������target��ƥ���ִ�е�target����ƥ��ִ�е�target
enum biz_ret
{
	RET_OK = 1,		/// ������ȷ
	RET_ACCEPT = (1 << 2),		/// ������ɣ�����Ҫ����rule��һ������
	RET_CONTINUE = (1 << 3),	/// ��rule�����������Ҫ����rule��һ������
	/// RET_DROP   = (1 << 5),		/// 
	RET_FAILED = (1 << 31),		/// �������
};

class CFTCPField;

/// biz_match���������Ϣ
class biz_data
{
public:
	CFTCPField *field; 
	uint32_t field_id;			/// ҵ��ϵͳ��ÿ��field����һ��Ψһ�ı��
	biz_ret ret;				/// ҵ�����أ�ָ����һ����δ���
	char error_msg[64];			/// ҵ�����ش�����Ϣ
	char private_data[64];		/// δ��ҵ�������ӣ�Ԥ��һ���ֿռ䣬���ⲿ�ֿռ��
								/// �����ǰ������rule��Э��
};

/// ����match��target�����������ò��������������֧��ͨ���������ı�
/// match��target����Ϊ
/// ��������ֻ�涨����
enum biz_param_type
{
	biz_param_type_char = 1,
	biz_param_type_int64,
	biz_param_type_uint64,
	biz_param_type_double
};

class biz_param
{
public:
	std::string param_name;
	biz_param_type param_type;
	uint32_t param_index;	/// biz_param��÷ŵ�vector�У��˲���Ϊvector�±�
	uint32_t param_count;	/// �п��ܴ���param_type���͵�����
	char value[64]; 		/// ʹ���߱�֤sizeof (param_type) * param_count < 64
};

/// 
class biz_match 
{
public:
	virtual ~biz_match ()
	{}

	/// ע��ʱ����
	virtual bool match_init (std::vector<biz_param>& params) = 0;
	
	/// ����ƥ��true��ƥ��false
	virtual bool handle_input (biz_data *data) = 0;
	/// ע��ǰ����
	virtual void match_exit () = 0;

	std::string _desc;
	std::string _name;
	std::vector<biz_param> _params;
};

enum biz_target_type
{
	target_type_continue = 1,	/// �����������󲿷ֶ�������������
	target_type_accept,		 	/// �����������Ҫ��һ��������ֹ���һ�й�����
};

class biz_target
{
public:
	virtual ~biz_target ()
	{}

	virtual bool target_init (std::vector<biz_param>& params)
	{}
	
	virtual biz_ret handle_input (biz_data *data) = 0;

	virtual void target_exit ()
	{}

	biz_target_type get_target_type ()
	{
		return _type;
	}

public:
	std::string _desc;
	std::string _name;
	std::vector<biz_param> _params;
	biz_target_type _type;
};

/// һ��������N��match��match֮����and��ϵ��ֻ������matchȫ��ƥ��
/// �������ƥ��
class biz_rule 
{
public:
	biz_ret handle_input (biz_data *pdata) 
	{
		for (auto iter = _matches_list.begin ();
			iter != _matches_list.end (); ++iter) {
			biz_match *pmatch = *iter;
			bool ret = pmatch->handle_input (pdata);
			if (ret == false) { /// ��ƥ������һ��������
				return RET_CONTINUE;/// һ�������е�match��and��ϵ
			}
		}

		/// ƥ������target�������target��һ��chain����ô�����
		/// ����chain��handle_input
		_ptarget->handle_input (pdata);
		
		if (_ptarget->get_target_type () == target_type_accept) {
			return RET_ACCEPT;
		}

		/// ��iptables��ͬ����һ�������������Ĭ��������Ǽ�������
		/// iptables����ֹ����
		return RET_CONTINUE;
	}

	/// ����ִ��˳�����ע�ᣬmatch֮����and��ϵ��ֻ��ȫ��match��
	/// ƥ�����Ϊƥ��
	bool register_match (biz_match *pmatch, std::vector<biz_param>& params)
	{
		if (pmatch == NULL) {
			printf ("%s:%d pmatch == NULL\n", __FILE__, __LINE__);
			return false;
		}

		if (pmatch->match_init (params) == false) {
			printf ("%s:%d match: %s init failed\n", 
				__FILE__, __LINE__, pmatch->_name.c_str ());
			return false;
		}

		_matches_list.push_back (pmatch);

		return true;
	}

	bool register_target (biz_target *ptarget, std::vector<biz_param>& params)
    {
        if (ptarget == NULL) {
            printf ("%s:%d ptarget == NULL\n", __FILE__, __LINE__);
            return false;
        }

        if (ptarget->target_init (params) == false) {
            printf ("%s:%d target: %s init failed\n", 
				__FILE__, __LINE__, ptarget->_name.c_str ());
			return false;
		}

		_ptarget = ptarget;
        return true;
    }

public:
	uint32_t _rule_position;
	std::list<biz_match *> _matches_list;
	std::string _desc;
	biz_target *_ptarget;
};

/// target������һ���µ���������Ҫ�̳�
class biz_chain : public biz_target
{
public:
	biz_ret handle_input (biz_data *pdata)
	{
		biz_ret ret;
		for (auto iter = rules_list.begin (); 
			iter != rules_list.end (); ++iter) {
			biz_rule *prule = *iter;
			ret = prule->handle_input (pdata);
			if (ret == RET_CONTINUE) {
				continue;
			}

			/// ����RET_CONTINUE�Ĺ���Ҫ���ظ���������
			/// �п������������ã���Ҫ���ظ�����������Ҳ��Ҫ
			/// �����־����������ᴦ�����ϵĹ���
			return ret;
		}

		return RET_CONTINUE;
	}

	/// ������Լӵ�����λ�ã�ҵ������ʹ����Ա��֤
	/// position == 0���������ײ�����
	/// position == 1���ڵ�һ����������
	/// position == rules_list.size ()��������β����
	/// ע��: ���������������ʱ�����������λ��һֱ�ڱ䣬������ǰ��һ�������п���
	/// ��ɵڶ�������Ҫ��ϸ����
	bool register_rule (biz_rule *prule, size_t position) 
	{
		if (position < 0 || position > rules_list.size ()) {
			printf ("%s:%d argument position: %zu is invalid\n", 
				__FILE__, __LINE__, position);
			return false;
		}

		if (prule == NULL) {
			printf ("%s:%d argument prule == NULL is invalid\n", 
				__FILE__, __LINE__);
			return false;
		}

		size_t current_position = 0;
		for (auto iter = rules_list.begin (); iter != rules_list.end (); ++iter) {
			if (current_position++ == position) {
				rules_list.insert (iter, prule);
				return true;
			}
		}
		/// ǰ���Ѿ�����position�����Լ��
		rules_list.push_back (prule);
		
		return false;
	}

	/// Ŀǰ������ʹ��id��ɾ��rule������ͨ��λ����ִ��ɾ��
	/// ע�⣺��һ�������position == 1������˳����
	bool unregister_rule (size_t position)
	{
		if (position < 1 || position > rules_list.size ()) {
			printf ("%s:%d argument position: %zu is invalid\n", 
				__FILE__, __LINE__, position);
			return false;
		}

		size_t current_position = 0;
		for (auto iter = rules_list.begin (); iter != rules_list.end (); ++iter) {
			if (++current_position == position) {
				rules_list.erase (iter);
				return true;
			}
		}
		
		return false;
	}

	std::string& get_chain_name ()
	{
		return _chain_name;
	}

	int32_t get_chain_id ()
	{
		return _chain_id;
	}

private:
	std::string _chain_name;
	int32_t _chain_id;	/// ÿ����ȫ��Ψһid�������ڱ��У���ͬ��֮���п�������ͬchain_id
						/// ������ֿ����������������������
	std::string _desc;
	/// һ��chain�ϵ�rule���Բ����µ�rule������ɾ��
	std::list<biz_rule *> rules_list;
};

class biz_table
{
public:
	biz_table (std::string table_name, int32_t table_id)
	: _table_id (table_id), _table_name (table_name), _chain_id (0)
	{
	}

	/// chain_id��һ�ű��е�ĳ������ʹ��ʱ��Ҫע�ⲻҪֱ����ת��������
	/// ҵ������
	void handle_input (biz_data *pdata, int32_t chain_id)
	{
		auto iter = _chain_map.find (chain_id); 
		if (iter == _chain_map.end ()) {
			printf ("%s:%d chain_id: %d not find.\n", 
				__FILE__, __LINE__, chain_id);
			abort ();
		}

		biz_chain *pchain = iter->second;
		pchain->handle_input (pdata);
	}

	bool delete_chain (std::string& chain_name)
	{
		for (auto iter = _chain_map.begin (); 
			iter != _chain_map.end (); ++iter) {
			if (iter->second->get_chain_name () != chain_name) {
				_chain_map.erase (iter);
				return true;
			}
		}

		printf ("%s:%d chain: %s doesn't exist.\n",
			 __FILE__, __LINE__, chain_name.c_str ());
		return false;
	}

	bool delete_chain (int32_t chain_id) 
	{
		auto iter = _chain_map.find (chain_id);
		if (iter == _chain_map.end ()) {
			printf ("%s:%d chain_id: %d doesn't exist.\n",
				 __FILE__, __LINE__, chain_id);
			return false;
		}

		_chain_map.erase (iter);
		return true;
	}

	/// chain_name ����Ψһ��chain�����ظ�ע��
	/// tableΪÿһ��chain����Ψһid������������תʱ����������
	/// return -1����ʾ����
	int32_t register_chain (biz_chain* pchain) 
	{
		for (auto iter = _chain_map.begin (); 
			iter != _chain_map.end (); ++iter) {
			if (iter->second->get_chain_name () == pchain->get_chain_name ()) {
				printf ( "%s has been registered.\n", 
					pchain->get_chain_name ().c_str ());
				return -1;
			}
		}

		++_chain_id;
		_chain_map.insert (std::make_pair (_chain_id, pchain));
		return _chain_id;
	}

private:
	int32_t _table_id; /// ȫ��Ψһ�ı�id
	std::string _table_name;
	int32_t _chain_id;

	/// �п��ܽ�����chainɾ��
	/// <chain_id, chain>
	std::unordered_map<int32_t, biz_chain *> _chain_map;
};

#endif
