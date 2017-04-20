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

/// Ò»¸ötableÖĞÓĞN¸öchain£¬Ò»¸öchainÓĞN¸örule£¬Ò»¸öruleÖĞÓĞN¸ömatch£¬
/// Ò»¸öruleÖĞÓĞÁ½¸ötarget£¬Æ¥ÅäºóÖ´ĞĞµÄtarget£¬²»Æ¥ÅäÖ´ĞĞµÄtarget
enum biz_ret
{
	RET_OK = 1,		/// ´¦ÀíÕıÈ·
	RET_ACCEPT = (1 << 2),		/// ´¦ÀíÍê³É£¬²»ĞèÒªËæºóµÄrule½øÒ»²½´¦Àí
	RET_CONTINUE = (1 << 3),	/// ±¾rule´¦Àí½áÊø£¬ĞèÒªËæºóµÄrule½øÒ»²½´¦Àí
	/// RET_DROP   = (1 << 5),		/// 
	RET_FAILED = (1 << 31),		/// ´¦Àí´íÎó
};

class CFTCPField;

/// biz_matchÊäÈëÊä³öĞÅÏ¢
class biz_data
{
public:
	CFTCPField *field; 
	uint32_t field_id;			/// ÒµÎñÏµÍ³ÖĞÃ¿¸öfield¶¼ÓĞÒ»¸öÎ¨Ò»µÄ±àºÅ
	biz_ret ret;				/// ÒµÎñ´¦Àí·µ»Ø£¬Ö¸µ¼ÏÂÒ»²½ÈçºÎ´¦Àí
	char error_msg[64];			/// ÒµÎñ´¦Àí·µ»Ø´íÎóĞÅÏ¢
	char private_data[64];		/// Î´À´ÒµÎñ»á¸ü¸´ÔÓ£¬Ô¤ÁôÒ»²¿·Ö¿Õ¼ä£¬¶ÔÕâ²¿·Ö¿Õ¼äµÄ
								/// ½â¶ÁÓÉÇ°ºóÁ½¸öruleÀ´Ğ­ÉÌ
};

/// ¸÷ÖÖmatchºÍtarget¶¼¿ÉÒÔÓĞÅäÖÃ²ÎÊı£¬ÀíÏëÇé¿öÏÂÖ§³ÖÍ¨¹ıÅäÖÃÀ´¸Ä±ä
/// matchºÍtargetµÄĞĞÎª
/// ²ÎÊıÀàĞÍÖ»¹æ¶¨¼¸ÖÖ
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
	uint32_t param_index;	/// biz_param×îºÃ·Åµ½vectorÖĞ£¬´Ë²ÎÊıÎªvectorÏÂ±ê
	uint32_t param_count;	/// ÓĞ¿ÉÄÜ´«µİparam_typeÀàĞÍµÄÊı×é
	char value[64]; 		/// Ê¹ÓÃÕß±£Ö¤sizeof (param_type) * param_count < 64
};

/// 
class biz_match 
{
public:
	virtual ~biz_match ()
	{}

	/// ×¢²áÊ±µ÷ÓÃ
	virtual bool match_init (std::vector<biz_param>& params)
	{
		return true;
	}
	
	/// ·µ»ØÆ¥Åätrue»ò²»Æ¥Åäfalse
	virtual bool handle_input (biz_data *data) = 0;
	/// ×¢ÏúÇ°µ÷ÓÃ
	virtual void match_exit () 
	{}

	std::string _desc;
	std::string _name;
	std::vector<biz_param> _params;
};

enum biz_target_type
{
	target_type_continue = 1,	/// ¼ÌĞø´¦Àí£¬¾ø´ó²¿·Ö¶¼ÊôÓÚÕâÖÖÀàĞÍ
	target_type_accept = (1 << 1), 	/// ´´¦Àí½áÊøºó²»ĞèÒª½øÒ»²½´¦Àí£¬ÖÕÖ¹ËæºóÒ»ÇĞ¹æÔò´¦Àí
	target_type_chain = (1 << 2), 	/// ´ËtargetÊÇÒ»¸öchain
};

class biz_target
{
public:
	biz_target () 
	: _type (target_type_continue)
	{}

	biz_target (biz_target_type target_type)
	: _type (target_type)
	{}

	virtual ~biz_target ()
	{}

	virtual bool target_init (std::vector<biz_param>& params)
	{
		return true;
	}
	
	virtual biz_ret handle_input (biz_data *data) = 0;

	virtual void target_exit ()
	{}

	biz_target_type get_target_type ()
	{
		return _type;
	}

	void set_target_type (biz_target_type type)
	{
		_type = type;
	}

public:
	std::string _desc;
	std::string _name;
	std::vector<biz_param> _params;
	biz_target_type _type;
};

/// Ò»Ìõ¹æÔòÓĞN¸ömatch£¬matchÖ®¼äÊÇand¹ØÏµ£¬Ö»ÓĞËùÓĞmatchÈ«²¿Æ¥Åä
/// ºó²ÅÄÜËãÆ¥Åä
class biz_rule 
{
public:
	biz_ret handle_input (biz_data *pdata) 
	{
		for (auto iter = _matches_list.begin ();
			iter != _matches_list.end (); ++iter) {
			biz_match *pmatch = *iter;
			bool ret = pmatch->handle_input (pdata);
			if (ret == false) { /// ²»Æ¥ÅäÓÉÏÂÒ»Ìõ¹æÔò´¦Àí
				return RET_CONTINUE;/// Ò»Ìõ¹æÔòÖĞµÄmatchÊÇand¹ØÏµ
			}
		}

		/// Æ¥Åäºó½øĞĞtarget´¦Àí£¬Èç¹ûtargetÊÇÒ»Ìõchain£¬ÄÇÃ´»áµ÷ÓÃ
		/// ÕâÌõchainµÄhandle_input
		_ptarget->handle_input (pdata);
		
		if (_ptarget->get_target_type () == target_type_accept) {
			return RET_ACCEPT;
		}

		/// ºÍiptables²»Í¬µÄÊÇÒ»Ìõ¹æÔò´¦Àí½áÊøºóÄ¬ÈÏÇé¿öÏÂÊÇ¼ÌĞø´¦Àí
		/// iptablesÊÇÖÕÖ¹´¦Àí
		return RET_CONTINUE;
	}

	/// °´ÕÕÖ´ĞĞË³Ğò½øĞĞ×¢²á£¬matchÖ®¼äÊÇand¹ØÏµ£¬Ö»ÓĞÈ«²¿match¶¼
	/// Æ¥Åä²ÅÈÏÎªÆ¥Åä
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

		if (ptarget->get_target_type () == target_type_accept) {

		}
        return true;
    }

	biz_target* get_target ()
	{
		return _ptarget;
	}

public:
	uint32_t _rule_position;
	std::list<biz_match *> _matches_list;
	std::string _desc;
	biz_target *_ptarget;
};

/// target¿ÉÒÔÊÇÒ»ÌõĞÂµÄÁ´£¬ËùÒÔÒª¼Ì³Ğ
class biz_chain : public biz_target
{
public:
	biz_chain ()
	{
		set_target_type (target_type_chain);
	}

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

			/// ²»ÊÇRET_CONTINUEµÄ¹æÔò¶¼Òª·µ»Ø¸ø¸¸Á´´¦Àí
			/// ÓĞ¿ÉÄÜÔÚ×ÓÁ´µ÷ÓÃ£¬ĞèÒª·µ»Ø¸ø¸¸Á´£¬¸¸Á´Ò²ĞèÒª
			/// Õâ¸ö±êÖ¾£¬·ñÔò¼ÌĞø»á´¦Àí¸¸Á´ÉÏµÄ¹æÔò
			return ret;
		}

		return RET_CONTINUE;
	}

	/// ¹æÔò¿ÉÒÔ¼Óµ½ÈÎÒâÎ»ÖÃ£¬ÒµÎñº¬ÒåÓĞÊ¹ÓÃÈËÔ±±£Ö¤
	/// position == 0£¬ÔÚÁ´±íÊ×²¿¼ÓÈë
	/// position == 1£¬ÔÚµÚÒ»¸ö¹æÔòºó¼ÓÈë
	/// position == rules_list.size ()£¬ÔÚÁ´±íÎ²¼ÓÈë
	/// ×¢Òâ: Á¬Ğø¼ÓÈë¶àÌõ¹æÔòÊ±£¬¹æÔòÁ´±íµÄÎ»ÖÃÒ»Ö±ÔÚ±ä£¬±ÈÈçÒÔÇ°µÚÒ»Ìõ¹æÔò£¬ÓĞ¿ÉÄÜ
	/// ±ä³ÉµÚ¶şÌõ£¬ĞèÒª×ĞÏ¸²Ù×÷
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
		/// Ç°ÃæÒÑ¾­×öÁËpositionºÏÀíĞÔ¼ì²é
		rules_list.push_back (prule);
		
		/// Èç¹û´ËÁ´ÖĞÓĞÒ»¸öruleµÄtargetÊÇacceptÀàĞÍµÄ£¬Õû¸öÁ´Ò²ÊÇacceptÀàĞÍµÄ
		if (prule->get_target ()->get_target_type () == target_type_accept) {
			set_target_type (target_type_accept);
		}

		return false;
	}

	/// Ä¿Ç°»¹²»ÏëÊ¹ÓÃidÀ´É¾³ırule£¬¶øÊÇÍ¨¹ıÎ»ÖÃÀ´Ö´ĞĞÉ¾³ı
	/// ×¢Òâ£ºµÚÒ»Ìõ¹æÔòµÄposition == 1£¬ÆäËûË³Ğò±àºÅ
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
	int32_t _chain_id;	/// Ã¿ÌõÁ´È«¾ÖÎ¨Ò»id£¬¾ÖÏŞÓÚ±íÖĞ£¬²»Í¬±íÖ®¼äÓĞ¿ÉÄÜÓĞÏàÍ¬chain_id
						/// ²»»á³öÏÖ¿ç±íÌøµ½ÆäËû±íµÄÁ´ÕâÖÖÇé¿ö
	std::string _desc;
	/// Ò»¸öchainÉÏµÄrule¿ÉÒÔ²åÈëĞÂµÄrule£¬¿ÉÒÔÉ¾³ı
	std::list<biz_rule *> rules_list;
};

class biz_table
{
public:
	biz_table (std::string table_name, int32_t table_id)
	: _table_id (table_id), _table_name (table_name), _chain_id (0)
	{
	}

	/// chain_idÊÇÒ»ÕÅ±íÖĞµÄÄ³ÌõÁ´£¬Ê¹ÓÃÊ±ĞèÒª×¢Òâ²»ÒªÖ±½ÓÌø×ªµ½×ÓÁ´£¬
	/// ÒµÎñ»á´íÎó
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

	/// chain_name ±ØĞëÎ¨Ò»£¬chain²»ÄÜÖØ¸´×¢²á
	/// tableÎªÃ¿Ò»¸öchain·ÖÅäÎ¨Ò»id£¬µ±ÔÚÁ´¼äÌø×ªÊ±²»ÎşÉüĞÔÄÜ
	/// return -1£¬±íÊ¾³ö´í
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
	int32_t _table_id; /// È«¾ÖÎ¨Ò»µÄ±íid
	std::string _table_name;
	int32_t _chain_id;

	/// ÓĞ¿ÉÄÜ½«ÕûÌõchainÉ¾³ı
	/// <chain_id, chain>
	std::unordered_map<int32_t, biz_chain *> _chain_map;
};

#endif
