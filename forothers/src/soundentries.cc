int main()
try
{
	auto soundentriesdb{::wdb2::read_db2<::wdb2::definitions::SoundEntriesRec>("SoundEntries.db2")};
#if 0
	::wdb2::write_db2("SoundEntries_new.db2",soundentriesdb);
#endif
}
catch(::std::exception const& e)
{
	::fast_io::io::perrln(::fast_io::mnp::os_c_str(e.what()));
	return 1;
}
