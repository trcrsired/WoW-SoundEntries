#pragma once

namespace wdb2::definitions
{

struct SoundEntriesRec
{
::std::uint_least32_t m_ID;
::std::uint_least32_t m_soundType;
::std::uint_least32_t m_name;
::std::uint_least32_t m_FileDataID[20];
::std::uint_least32_t m_Freq[20];
::std::float32_t m_volumeFloat;
::std::uint_least32_t m_flags;
::std::float32_t m_minDistance;
::std::float32_t m_distanceCutoff;
::std::uint_least32_t m_EAXDef;
::std::uint_least32_t m_soundEntriesAdvancedID;
::std::float32_t m_volumevariationplus;
::std::float32_t m_volumevariationminus;
::std::float32_t m_pitchvariationplus;
::std::float32_t m_pitchvariationminus;
::std::float32_t m_pitchAdjust;
::std::uint_least32_t m_dialogtype;
::std::uint_least32_t m_busOverwriteID;
};

}
