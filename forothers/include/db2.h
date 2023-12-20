#pragma once
#include<cstdint>
#include"cvs.h"
#include"section.h"
#include<string_view>
#include<stdexcept>
#include<vector>

namespace wdb2
{

template<typename T>
struct db2
{
	using value_type = T;
	header hd;
	std::vector<field_structure> fields;
	std::vector<field_storage_info> field_storages;
	std::vector<section_header> section_headers;
	std::vector<section<T>> sections;
	std::vector<std::byte> pallet,common;
	explicit db2(std::string_view str)
	{
		using namespace std::string_view_literals;
		auto magic(str.substr(0,4));
		if(magic!="WDB2"sv)
			throw std::runtime_error(::fast_io::concat("Magic is not WDB. which is (",magic,")"));
		auto p(str.data()+4);
		hd=cvs<header>(str,p);
		if(hd.record_size!=sizeof(T))
			throw std::runtime_error(::fast_io::concat("record size ",sizeof(T)," is incorrect. Correct one should be ",hd.record_size," bytes"));
		if(hd.flags&1)
			throw std::runtime_error(::fast_io::concat("offset records not implemented, flags is ",hd.flags));

		section_headers=cvs<section_header>(str,p,hd.section_count);

		fields=cvs<field_structure>(str,p,hd.total_field_count);
		field_storages=cvs<field_storage_info>(str,p,hd.field_storage_info_size/sizeof(field_storage_info));
		pallet=cvs<std::byte>(str,p,hd.pallet_data_size);
		common=cvs<std::byte>(str,p,hd.common_data_size);

		for(const auto &ele : section_headers)
			sections.emplace_back(str,p,ele);
		if(str.data()+str.size()!=p)
			throw std::runtime_error("incorrect size");
	}
};

template<typename T>
inline auto serialize(db2<T> const&d)
{
	using namespace std::string_literals;
	auto s("WDB2"s);
	s.reserve(10000000);
	decltype(auto) header(svc(s,d.hd));
	if(header.flags&1)
		throw std::runtime_error(::fast_io::concat("offset records not implemented, flags is ",header.flags));
	decltype(auto) secheader(svcs<section_header>(s,d.sections.size()));
	decltype(auto) fields(svc(s,d.fields,header.total_field_count));
	decltype(auto) field_storages(svc(s,d.field_storages,header.field_storage_info_size));
	header.field_storage_info_size=header.field_storage_info_size*sizeof(field_storage_info);
	decltype(auto) pallet(svc(s,d.pallet,header.pallet_data_size));
	decltype(auto) common(svc(s,d.common,header.common_data_size));
	std::size_t record_count(0),string_table_size(0);
	for(std::size_t i(0);i!=d.sections.size();++i)
	{
		decltype(auto) ele(d.sections[i]);
		secheader.at(i)=svc(s,ele);
		record_count+=ele.records.size();
		string_table_size+=ele.string_table.size();
	}
 	header.record_count=record_count;
	header.string_table_size=string_table_size;
	return s;
}

template<typename ostrm,typename T>
decltype(auto) operator<<(ostrm &out,db2<T> const& t)
{
	for(std::size_t i(0);i!=t.field_storages.size();++i)
	{
		if(i)
		{
			out.put('\n');
			out.put('\n');
		}
		(out<<i).put('\n');
		out<<t.field_storages[i];
	}
	return (out);
}

}
