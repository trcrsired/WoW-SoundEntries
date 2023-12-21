#pragma once
#include"db2.h"
#include<string>
#include<string_view>

namespace wdb2
{
template<typename T>
inline db2<T> read_db2(std::string_view filename)
{
	::fast_io::native_file_loader loader(filename);
	return db2<T>(::std::string_view(loader.data(),loader.size()));
}

template<typename T>
inline void write_db2(std::string_view str,db2<T> const &w)
{
	auto sr(serialize(w));
	::fast_io::obuf_file obf(str);
	::fast_io::io::print(sr);
}

}
