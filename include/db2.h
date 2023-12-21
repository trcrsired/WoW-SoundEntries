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
	std::vector<T> records;
	std::vector<::std::int_least32_t> indicies;
	std::vector<::std::int_least16_t> string_lengths;
	::std::string string_table;
#if 0
	std::vector<std::byte> pallet,common;
#endif
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
		if(hd.max_id != 0)
		{
			::std::size_t n{static_cast<::std::size_t>(hd.max_id - hd.min_id + 1)};
			indicies=cvs<::std::int_least32_t>(str,p,n);
			string_lengths=cvs<::std::int_least16_t>(str,p,n);
		}
		records=cvs<T>(str,p,hd.record_count);
		string_table=cvs<char>(str,p,hd.string_table_size);
	}
};
#if 0
template<typename T>
inline auto serialize(db2<T> const& d)
{
	using namespace std::string_literals;
	auto s("WDB2"s);
	s.reserve(10000000);
	decltype(auto) header(svc(s,d.hd));

	decltype(auto) records(svc(s,d.fields,header.total_field_count));
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
#endif
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
