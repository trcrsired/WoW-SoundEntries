#pragma once
#include"raw.h"
#include"cvs.h"
#include<vector>
#include<string>

namespace wdb2
{

template<typename T>
struct section
{
	std::vector<T> records;
	std::string string_table;
	std::vector<std::uint32_t> ids;
	std::vector<copy_table_entry> copy_table;
	std::vector<offset_map_entry> offsets;
	struct relationship_mapping
	{
		std::uint32_t min_id,max_id;
		std::vector<relationship_entry>  entries;
	}relationship_map;
	template<typename Q>
	section(std::string_view str,Q& p,const section_header& header):records(check_section_validity<T>(str,p,header)),
		string_table(cvs<char>(str,p,header.string_table_size)),
		ids(cvs<std::uint32_t>(str,p,header.id_list_size/4)),
		copy_table(cvs<copy_table_entry>(str,p,header.copy_table_count))
	{
		using namespace std::string_literals;
		if(header.relationship_data_size)
		{
			decltype(auto) rmh(cvs<relationship_mapping_header>(str,p));
			relationship_map.min_id=rmh.min_id;
			relationship_map.max_id=rmh.max_id;
			relationship_map.entries=cvs<relationship_entry>(str,p,rmh.num_entries);
		}
	}
};

template<typename T>
auto svc(std::string& str,const section<T>& sct)
{
	section_header h{};
	h.file_offset=str.size();
	svc(str,sct.records,h.record_count);
	svc(str,sct.string_table,h.string_table_size);
	svc(str,sct.ids);
	h.id_list_size=sct.ids.size()*4;
	svc(str,sct.copy_table);
	h.copy_table_count=sct.copy_table.size();
	if(sct.relationship_map.entries.size())
	{
		h.relationship_data_size=sct.relationship_map.entries.size()*sizeof(relationship_entry)+12;
		relationship_mapping_header rmh{static_cast<std::uint32_t>(sct.relationship_map.entries.size()),sct.relationship_map.min_id,sct.relationship_map.max_id};
		svc(str,rmh);
		svc(str,sct.relationship_map.entries);
	}
	return h;
}

}