int main()
try
{
	using namespace fast_io::io;
	auto soundentriesdb{::wdb2::read_db2<::wdb2::definitions::SoundEntriesRec>("SoundEntries.db2")};
	::fast_io::obuf_file soundentrieslua("SoundEntriesToFiledataIDs.lua");
	print(soundentrieslua,R"abc(local MusicBox = LibStub("AceAddon-3.0"):GetAddon("MusicBox")
MusicBox.SoundEntries=
{)abc");
	for(auto const & e : soundentriesdb.records)
	{
		::std::size_t count{};
		for(auto const e : e.m_FileDataID)
		{
			if(e)
			{
				++count;
			}
		}
		if(count)
		{
			print(soundentrieslua,"[",e.m_ID,"]=");
			bool countnotone{count!=1u};
			if(countnotone)
			{
				print(soundentrieslua,"{");
			}
			for(auto const e : e.m_FileDataID)
			{
				if(e)
				{
					print(soundentrieslua,e,",");
				}
			}
			if(e.map_ID==36308)	//Orgrimmar Additional because we only have old versions of db2 file. These are guesses. Vulpera
			{
				print(soundentrieslua,"2145688,2145690");
			}// Orgrimmar Night
			else if(e.map_ID==36309)
			{
				print(soundentrieslua,"2145689,2145694");
			}
			else if(e.map_ID==2532)	//Stormwind. AnduinPt1. No Pt2
			{
				print(soundentrieslua,"1417240,1417241,1417242,1417243,1417244,1417245,1417246,1417247");
			}
			if(countnotone)
			{
				print(soundentrieslua,"},");
			}
			println(soundentrieslua);
		}
	}
	print(soundentrieslua,"}\n");
}
catch(::std::exception const& e)
{
	::fast_io::io::perrln(::fast_io::mnp::os_c_str(e.what()));
	return 1;
}
