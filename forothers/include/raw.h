#pragma once
#include<array>
#include<exception>
#include<string>
#include"cvs.h"

namespace wdb2
{

struct wdb2_header
{
	uint32_t record_count;
	uint32_t field_count;                                         // array fields count as the size of array for WDB2
	uint32_t record_size;
	uint32_t string_table_size;                                   // string block almost always contains at least one zero-byte
	uint32_t table_hash;
	uint32_t build;
	uint32_t timestamp_last_written;                              // set to time(0); when writing in WowClientDB2_Base::Save()
	uint32_t min_id;
	uint32_t max_id;
	uint32_t locale;                                              // as seen in TextWowEnum
	uint32_t copy_table_size;                                     // always zero in WDB2 (?) - see WDB3 for information on how to parse this
};

struct wdc4_header
{
	std::uint32_t record_count;           // this is for all sections combined now
	std::uint32_t field_count;
	std::uint32_t record_size;
	std::uint32_t string_table_size;      // this is for all sections combined now
	std::uint32_t table_hash;             // hash of the table name
	std::uint32_t layout_hash;            // this is a hash field that changes only when the structure of the data changes
	std::uint32_t min_id;
	std::uint32_t max_id;
	std::uint32_t locale;                 // as seen in TextWowEnum
	std::uint16_t flags;                  // possible values are listed in Known Flag Meanings
	std::uint16_t id_index;               // this is the index of the field containing ID values; this is ignored if flags & 0x04 != 0
	std::uint32_t total_field_count;      // from WDC1 onwards, this value seems to always be the same as the 'field_count' value
	std::uint32_t bitpacked_data_offset;  // relative position in record where bitpacked data begins; not important for parsing the file
	std::uint32_t lookup_column_count;
	std::uint32_t field_storage_info_size;
	std::uint32_t common_data_size;
	std::uint32_t pallet_data_size;
	std::uint32_t section_count;          // new to WDC2, this is number of sections of data (records + copy table + id list + relationship map = a section)
};

using header = wdb2_header;

struct section_header
{
	std::uint64_t tact_key_hash;       // TactKeyLookup hash
	std::uint32_t file_offset;            // absolute position to the beginning of the section
	std::uint32_t record_count;           // 'record_count' for the section
	std::uint32_t string_table_size;      // 'string_table_size' for the section
	std::uint32_t offset_records_end;	// Offset to the spot where the records end in a file with an offset map structure;
	std::uint32_t id_list_size;           // List of ids present in the DB file
	std::uint32_t relationship_data_size;// Size of the relationship data in the section
	std::uint32_t offset_map_id_count;// Count of ids present in the offset map in the section
	std::uint32_t copy_table_count;// Count of the number of deduplication entries (you can multiply by 8 to mimic the old 'copy_table_size' field)
};

template<typename ostrm>
decltype(auto) operator<<(ostrm& o,const section_header& fs)
{
	using namespace std::string_literals;
	return o<<"tact_key_hash:"s<<fs.tact_key_hash
			<<"\nfile_offset:"s<<fs.file_offset
			<<"\nrecord_count:"s<<fs.record_count
			<<"\nstring_table_size:"s<<fs.string_table_size
			<<"\noffset_records_end:"s<<fs.offset_records_end
			<<"\nid_list_size:"s<<fs.id_list_size
			<<"\nrelationship_data_size:"s<<fs.relationship_data_size
			<<"\noffset_map_id_count:"s<<fs.offset_map_id_count
			<<"\ncopy_table_count:"s<<fs.copy_table_count;
}

template<typename T>
inline decltype(auto) check_section_validity(std::string_view str,const char * &p,const section_header& s)
{
	using namespace std::string_literals;
	if(str.data()+s.file_offset!=p)
		throw std::runtime_error(
			::fast_io::concat("offset error size: ",str.size()," pos: ",p-str.data(),
						" should be: ",s.file_offset));
	return cvs<T>(str,p,s.record_count);
}

struct field_structure
{
	std::uint16_t size;
	std::uint16_t offset;
};

template<typename ostrm>
decltype(auto) operator<<(ostrm& o,const field_structure& fs)
{
	return o<<fs.offset<<"\tsize:"<<((32-fs.size)>>3);
}

enum class field_compression
{
	// None -- the field is a 8-, 16-, 32-, or 64-bit integer in the record data
	none,
	// Bitpacked -- the field is a bitpacked integer in the record data.  It
	// is field_size_bits long and starts at field_offset_bits.
	// A bitpacked value occupies
	//   (field_size_bits + (field_offset_bits & 7) + 7) / 8
	// bytes starting at byte
	//   field_offset_bits / 8
	// in the record data.  These bytes should be read as a little-endian value,
	// then the value is shifted to the right by (field_offset_bits & 7) and
	// masked with ((1ull << field_size_bits) - 1).
	bitpacked,
	// Common data -- the field is assumed to be a default value, and exceptions
	// from that default value are stored in the corresponding section in
	// common_data as pairs of { std::uint32_t record_id; std::uint32_t value; }.
	common_data,
	// Bitpacked indexed -- the field has a bitpacked index in the record data.
	// This index is used as an index into the corresponding section in
	// pallet_data.  The pallet_data section is an array of std::uint32_t, so the index
	// should be multiplied by 4 to obtain a byte offset.
	bitpacked_indexed,
	// Bitpacked indexed array -- the field has a bitpacked index in the record
	// data.  This index is used as an index into the corresponding section in
	// pallet_data.  The pallet_data section is an array of std::uint32_t[array_count],
	//
	bitpacked_indexed_array,
	// Same as field_compression_bitpacked
	bitpacked_signed
};

template<typename ostrm>
decltype(auto) operator<<(ostrm& o,const field_compression& fs)
{
	using namespace std::string_literals;
	switch(fs)
	{
		case field_compression::none: return o<<"none"s;
		case field_compression::bitpacked: return o<<"bitpacked"s;
		case field_compression::common_data: return o<<"common data"s;
		case field_compression::bitpacked_indexed: return o<<"bitpacked_indexed"s;
		case field_compression::bitpacked_indexed_array: return o<<"bitpacked_indexed_array"s;
		case field_compression::bitpacked_signed: return o<<"bitpacked_signed"s;
		default: return o<<"unknown("s<<static_cast<unsigned>(fs)<<')';
	}
}

struct field_storage_info
{
	std::uint16_t offset_bits;
	std::uint16_t field_size;
	std::uint32_t additional_data_size;
	field_compression type;
	std::array<std::uint32_t,3> values;
};

template<typename ostrm>
decltype(auto) operator<<(ostrm& o,const field_storage_info& fs)
{
	using namespace std::string_literals;
	o<<"offset_bits\t"s<<fs.offset_bits<<
		"\nfield_size\t"s<<fs.field_size<<
		"\nadditional_data_size\t"s<<fs.additional_data_size<<
		"\ntype\t"s<<fs.type;
	switch(fs.type)
	{
	case field_compression::none:break;
	case field_compression::bitpacked:
		o<<"\n\nbitpacking_offset_bits\t"s<<fs.values.front();
		o<<"\nbitpacking_size_bits\t"s<<fs.values[1];
		o<<"\nflags\t"s<<fs.values[2];
	break;
	case field_compression::common_data:
		o<<"\n\ndefault_value\t"s<<fs.values.front();
	break;
	case field_compression::bitpacked_indexed:
		o<<"\n\nbitpacking_offset_bits\t"s<<fs.values.front();
		o<<"\nbitpacking_size_bits\t"s<<fs.values[1];
	break;
	case field_compression::bitpacked_indexed_array:
		o<<"\n\nbitpacking_offset_bits\t"s<<fs.values.front();
		o<<"\nbitpacking_size_bits\t"s<<fs.values[1];
		o<<"\narray_count\t"s<<fs.values[2];
	break;
	default:
		o<<"\n\n"s;
		for(std::size_t i(0);i!=fs.values.size();++i)
			o<<fs.values[i]<<'\t';
	}
	return (o);
}

struct copy_table_entry
{
	std::uint32_t id_of_new_row;
	std::uint32_t id_of_copied_row;
};

struct offset_map_entry
{
	std::uint32_t offset;                                          // this offset is absolute, not relative to another structure; this can (and often will) be zero, in which case you should ignore that entry and move on
	std::uint16_t length;                                          // this is the length of the record located at the specified offset
};

struct relationship_entry
{
	// This is the id of the foreign key for the record, e.g. SpellID in
	// SpellX* tables.
	std::uint32_t foreign_id;
	// This is the index of the record in record_data.  Note that this is
	// *not* the record's own ID.
	std::uint32_t record_index;
};

template<typename ostrm>
decltype(auto) operator<<(ostrm& o,const relationship_entry& fs)
{
	return o<<fs.foreign_id<<' '<<fs.record_index;
}

struct relationship_mapping_header
{
	std::uint32_t num_entries;
	std::uint32_t min_id;
	std::uint32_t max_id;
};

}