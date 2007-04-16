/***************************************************************************
                          stemming.h  -  description
                             -------------------
    begin                : Sat Apr 26 2003
    copyright            : (C) 2003 by Blake Madden
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the BSD License.                                *
 *                                                                         *
 ***************************************************************************/

#ifndef __STEM_H__
#define __STEM_H__

#include "semantic/stem/meta.h"
#include "semantic/stem/string_util.h"
#include "semantic/stem/utilities.h"

namespace stemming
	{
    //constant for letters
    const xchar UPPER_A = 0x41;
    const xchar LOWER_A = 0x61;
    const xchar UPPER_B = 0x42;
    const xchar LOWER_B = 0x62;
    const xchar UPPER_C = 0x43;
    const xchar LOWER_C = 0x63;
    const xchar UPPER_D = 0x44;
    const xchar LOWER_D = 0x64;
    const xchar UPPER_E = 0x45;
    const xchar LOWER_E = 0x65;
    const xchar UPPER_F = 0x46;
    const xchar LOWER_F = 0x66;
    const xchar UPPER_G = 0x47;
    const xchar LOWER_G = 0x67;
    const xchar UPPER_H = 0x48;
    const xchar LOWER_H = 0x68;
    const xchar UPPER_I = 0x49;
    const xchar LOWER_I = 0x69;
    const xchar UPPER_J = 0x4A;
    const xchar LOWER_J = 0x6A;
    const xchar UPPER_K = 0x4B;
    const xchar LOWER_K = 0x6B;
    const xchar UPPER_L = 0x4C;
    const xchar LOWER_L = 0x6C;
    const xchar UPPER_M = 0x4D;
    const xchar LOWER_M = 0x6D;
    const xchar UPPER_N = 0x4E;
    const xchar LOWER_N = 0x6E;
    const xchar UPPER_O = 0x4F;
    const xchar LOWER_O = 0x6F;
    const xchar UPPER_P = 0x50;
    const xchar LOWER_P = 0x70;
    const xchar UPPER_Q = 0x51;
    const xchar LOWER_Q = 0x71;
    const xchar UPPER_R = 0x52;
    const xchar LOWER_R = 0x72;
    const xchar UPPER_S = 0x53;
    const xchar LOWER_S = 0x73;
    const xchar UPPER_T = 0x54;
    const xchar LOWER_T = 0x74;
    const xchar UPPER_U = 0x55;
    const xchar LOWER_U = 0x75;
    const xchar UPPER_V = 0x56;
    const xchar LOWER_V = 0x76;
    const xchar UPPER_W = 0x57;
    const xchar LOWER_W = 0x77;
    const xchar UPPER_X = 0x58;
    const xchar LOWER_X = 0x78;
    const xchar UPPER_Y = 0x59;
    const xchar LOWER_Y = 0x79;
    const xchar UPPER_Z = 0x5A;
    const xchar LOWER_Z = 0x7A;

    const xchar UPPER_A_ACUTE = 0xC1;
    const xchar LOWER_A_ACUTE = 0xE1;
    const xchar UPPER_E_ACUTE = 0xC9;
    const xchar LOWER_E_ACUTE = 0xE9;
    const xchar UPPER_I_ACUTE = 0xCD;
    const xchar LOWER_I_ACUTE = 0xED;
    const xchar UPPER_O_ACUTE = 0xD3;
    const xchar LOWER_O_ACUTE = 0xF3;
    const xchar UPPER_A_CIRCUMFLEX = 0xC2;
    const xchar LOWER_A_CIRCUMFLEX = 0xE2;
    const xchar UPPER_E_CIRCUMFLEX = 0xCA;
    const xchar LOWER_E_CIRCUMFLEX = 0xEA;
    const xchar UPPER_O_STROKE = 0xD8;
    const xchar LOWER_O_STROKE = 0xF8;
    const xchar UPPER_C_CEDILLA = 0xC7;
    const xchar LOWER_C_CEDILLA = 0xE7;
    const xchar LOWER_A_UMLAUTS = 0xC4;
    const xchar UPPER_A_UMLAUTS = 0xE4;
    const xchar UPPER_O_UMLAUTS = 0xD6;
    const xchar LOWER_O_UMLAUTS = 0xF6;
    const xchar TILDE = 0x7E;
    const xchar UPPER_A_GRAVE = 0xC0;
    const xchar LOWER_A_GRAVE = 0xE0;
    const xchar UPPER_E_GRAVE = 0xC8;
    const xchar LOWER_E_GRAVE = 0xE8;
    const xchar UPPER_I_GRAVE = 0xCC;
    const xchar LOWER_I_GRAVE = 0xEC;
    const xchar UPPER_O_GRAVE = 0xD2;
    const xchar LOWER_O_GRAVE = 0xF2;
	//these characters should not appear in an indexed word
	const xchar UPPER_Y_HASH = 7;//bell
	const xchar LOWER_Y_HASH = 9;//tab
	const xchar UPPER_I_HASH = 10;//line feed
	const xchar LOWER_I_HASH = 11;//vertical tab
	const xchar UPPER_U_HASH = 12;//form feed (new page)
	const xchar LOWER_U_HASH = 13;//carriage return

    //language constants
//    static const xchar* FRENCH_VOWELS = _XTEXT("aeiouy‚‡ÎÈÍËÔÓÙ˚˘AEIOUY¬¿À… »œŒ‘€Ÿ");
//   static const xchar* GERMAN_VOWELS = _XTEXT("aeiou¸y‰ˆAƒEIO÷U‹Y");
//    static const xchar* FINNISH_VOWELS = _XTEXT("aeiouy‰ˆAEIOUYƒ÷");
//    static const xchar* DUTCH_VOWELS = _XTEXT("aeiouyËAEIOUY»");
//    static const xchar* DUTCH_KDT = _XTEXT("kdtKDT");
//    static const xchar* NORWEGIAN_VOWELS = _XTEXT("aeio¯uyÂÊ≈A∆EIOÿUY");
//    static const xchar* PORTUGUESE_VOWELS = _XTEXT("aeiou·ÈÌÛ˙‚ÍÙAEIOU¡…Õ”⁄¬ ‘");


	template<typename Tchar_type = xchar>
	class stem
		{
	public:
		stem() : m_r1(0), m_r2(0), m_rv(0) {}
	protected:
		//R1, R2, RV functions
		void find_r1(const std::basic_string<Tchar_type>& text,
					const Tchar_type* vowel_list)
			{
			//see where the R1 section begin
			//R1 is the region after the first consonant after the first vowel
			size_t start = text.find_first_of(vowel_list, 0);
			if (start == std::basic_string<Tchar_type>::npos)
				{
				//we need at least need a vowel somewhere in the word
				m_r1 = text.length();
				return;
				}

			m_r1 = text.find_first_not_of(vowel_list,++start);
			if (m_r1 == std::basic_string<Tchar_type>::npos)
				{
				m_r1 = text.length();
				}
			else
				{
				++m_r1;
				}
			}

		void find_r2(const std::basic_string<Tchar_type>& text,
					const Tchar_type* vowel_list)
			{
			size_t start = 0;
			//look for R2--not required for all criteria.
			//R2 is the region after the first consonant after the first vowel after R1
			if (m_r1 != text.length() )
				{
				start = text.find_first_of(vowel_list, m_r1);
				}
			else
				{
				start = std::basic_string<Tchar_type>::npos;
				}
			if (start != std::basic_string<Tchar_type>::npos &&
				static_cast<int>(start) != static_cast<int>(text.length())-1)
				{
				m_r2 = text.find_first_not_of(vowel_list,++start);
				if (m_r2 == std::basic_string<Tchar_type>::npos)
					{
					m_r2 = text.length();
					}
				else
					{
					++m_r2;
					}
				}
			else
				{
				m_r2 = text.length();
				}
			}

		void find_spanish_rv(const std::basic_string<Tchar_type>& text,
					const Tchar_type* vowel_list)
			{
			//see where the RV section begin
			if (text.length() < 4)
				{
				m_rv = text.length();
				return;
				}
			//if second letter is a consonant
			if (!string_util::is_one_of(text[1], vowel_list) )
				{
				size_t start = text.find_first_of(vowel_list, 2);
				if (start == std::basic_string<Tchar_type>::npos)
					{
					//can't find next vowel
					m_rv = text.length();
					return;
					}
				else
					{
					m_rv = start+1;
					}
				}
			//if first two letters are vowels
			else if (string_util::is_one_of(text[0], vowel_list) &&
					string_util::is_one_of(text[1], vowel_list))
				{
				size_t start = text.find_first_not_of(vowel_list, 2);
				if (start == std::basic_string<Tchar_type>::npos)
					{
					//can't find next consonant
					m_rv = text.length();
					return;
					}
				else
					{
					m_rv = start+1;
					}
				}
			//consonant/vowel at beginning
			else if (!string_util::is_one_of(text[0], vowel_list) &&
					string_util::is_one_of(text[1], vowel_list))
				{
				m_rv = 3;
				}
			else
				{
				m_rv = text.length();
				}	
			}

		void find_french_rv(const std::basic_string<Tchar_type>& text,
					const Tchar_type* vowel_list)
			{
			//see where the RV section begin
			if (text.length() < 3)
				{
				m_rv = text.length();
				return;
				}
			//if first two letters are vowels
			if (string_util::is_one_of(text[0], vowel_list) &&
				string_util::is_one_of(text[1], vowel_list))
				{
				m_rv = 3;
				}
			else
				{
				size_t start = text.find_first_not_of(vowel_list, 0);
				if (start == std::basic_string<Tchar_type>::npos)
					{
					//can't find first consonant
					m_rv = text.length();
					return;
					}
				start = text.find_first_of(vowel_list, start);
				if (start == std::basic_string<Tchar_type>::npos)
					{
					//can't find first vowel
					m_rv = text.length();
					return;
					}
				m_rv = start+1;
				}
			}

		void find_russian_rv(const std::basic_string<Tchar_type>& text,
					const Tchar_type* vowel_list)
			{
			size_t start = text.find_first_of(vowel_list);
			if (start == std::basic_string<Tchar_type>::npos)
				{
				//can't find first vowel
				m_rv = text.length();
				return;
				}
			else
				{
				m_rv = start+1;
				}
			}

		inline void update_r_sections(const std::basic_string<Tchar_type>& text)
			{
			if (m_r2 > text.length() )
				{
				m_r2 = text.length();
				}
			if (m_r1 > text.length() )
				{
				m_r1 = text.length();
				}
			if (m_rv > text.length() )
				{
				m_rv = text.length();
				}
			}
		//---------------------------------------------
		void trim_western_punctuation(std::basic_string<Tchar_type>& text)
			{
            if (text.length() >= 3 &&
                string_util::strnicmp(text.c_str()+(text.length()-3), _XTEXT("'s'"), 3) == 0)
				{
				text.erase(text.length()-3);
				}
			else if (text.length() >= 2 &&
                string_util::strnicmp(text.c_str()+(text.length()-2), _XTEXT("'s"), 2) == 0)
				{
				text.erase(text.length()-2);
				}
			else if (text[text.length()-1] == _XTEXT('\''))
				{
				text.erase(text.end()-1);
				}
			while (text.length() )
				{
				if (!(text[text.length()-1] >= 48 && text[text.length()-1] <= 57) &&
					!(text[text.length()-1] >= 65 && text[text.length()-1] <= 90) &&
					!(text[text.length()-1] >= 97 && text[text.length()-1] <= 122) &&
					!(text[text.length()-1] >= assign_character(Tchar_type, 192) && text[text.length()-1] <= assign_character(Tchar_type, 246)) &&
					!(text[text.length()-1] >= assign_character(Tchar_type, 248) && text[text.length()-1] <= assign_character(Tchar_type, 255)) )
					{
					text.erase(text.end()-1);
					}
				else
					{
					break;
					}
				}
			while (text.length() )
				{
				if (!(text[0] >= 48 && text[0] <= 57) &&
					!(text[0] >= 65 && text[0] <= 90) &&
					!(text[0] >= 97 && text[0] <= 122) &&
					!(text[0] >= assign_character(Tchar_type, 192) && text[0] <= assign_character(Tchar_type, 246)) &&
					!(text[0] >= assign_character(Tchar_type, 248) && text[0] <= assign_character(Tchar_type, 255)) )
					{
					text.erase(0, 1);
					}
				else
					{
					break;
					}
				}
			}

		//suffix removal determinant functions
		///is_suffix for one character
		inline bool is_suffix(const std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U)
			{
			if (text.length() < 1)
				{ return false; }
			return is_either(text[text.length()-1], suffix1L, suffix1U);
			}
		///is_suffix for two character
		inline bool is_suffix(const std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U)
			{
			if (text.length() < 2)
				{
				return false;
				}
			return is_either(text[text.length()-2], suffix1L, suffix1U) &&
					is_either(text[text.length()-1], suffix2L, suffix2U);
			}

		///is_suffix for three character
		inline bool is_suffix(const std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U)
			{
			if (text.length() < 3)
				{
				return false;
				}
			return is_either(text[text.length()-3], suffix1L, suffix1U) &&
					is_either(text[text.length()-2], suffix2L, suffix2U) &&
					is_either(text[text.length()-1], suffix3L, suffix3U);
			}
		///is_suffix for four character
		inline bool is_suffix(const std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U)
			{
			if (text.length() < 4)
				{
				return false;
				}
			return is_either(text[text.length()-4], suffix1L, suffix1U) &&
					is_either(text[text.length()-3], suffix2L, suffix2U) &&
					is_either(text[text.length()-2], suffix3L, suffix3U) &&
					is_either(text[text.length()-1], suffix4L, suffix4U);
			}
		///is_suffix for five character
		inline bool is_suffix(const std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U,
					Tchar_type suffix5L, Tchar_type suffix5U)
			{
			if (text.length() < 5)
				{
				return false;
				}
			return is_either(text[text.length()-5], suffix1L, suffix1U) &&
					is_either(text[text.length()-4], suffix2L, suffix2U) &&
					is_either(text[text.length()-3], suffix3L, suffix3U) &&
					is_either(text[text.length()-2], suffix4L, suffix4U) &&
					is_either(text[text.length()-1], suffix5L, suffix5U);
			}
		///is_suffix for six character
		inline bool is_suffix(const std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U,
					Tchar_type suffix5L, Tchar_type suffix5U,
					Tchar_type suffix6L, Tchar_type suffix6U)
			{
			if (text.length() < 6)
				{
				return false;
				}
			return is_either(text[text.length()-6], suffix1L, suffix1U) &&
					is_either(text[text.length()-5], suffix2L, suffix2U) &&
					is_either(text[text.length()-4], suffix3L, suffix3U) &&
					is_either(text[text.length()-3], suffix4L, suffix4U) &&
					is_either(text[text.length()-2], suffix5L, suffix5U) &&
					is_either(text[text.length()-1], suffix6L, suffix6U);
			}
		///is_suffix for seven character
		inline bool is_suffix(const std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U,
					Tchar_type suffix5L, Tchar_type suffix5U,
					Tchar_type suffix6L, Tchar_type suffix6U,
					Tchar_type suffix7L, Tchar_type suffix7U)
			{
			if (text.length() < 7)
				{
				return false;
				}
			return is_either(text[text.length()-7], suffix1L, suffix1U) &&
					is_either(text[text.length()-6], suffix2L, suffix2U) &&
					is_either(text[text.length()-5], suffix3L, suffix3U) &&
					is_either(text[text.length()-4], suffix4L, suffix4U) &&
					is_either(text[text.length()-3], suffix5L, suffix5U) &&
					is_either(text[text.length()-2], suffix6L, suffix6U) &&
					is_either(text[text.length()-1], suffix7L, suffix7U);
			}
		///is_suffix for eight character
		inline bool is_suffix(const std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U,
					Tchar_type suffix5L, Tchar_type suffix5U,
					Tchar_type suffix6L, Tchar_type suffix6U,
					Tchar_type suffix7L, Tchar_type suffix7U,
					Tchar_type suffix8L, Tchar_type suffix8U)
			{
			if (text.length() < 8)
				{
				return false;
				}
			return is_either(text[text.length()-8], suffix1L, suffix1U) &&
					is_either(text[text.length()-7], suffix2L, suffix2U) &&
					is_either(text[text.length()-6], suffix3L, suffix3U) &&
					is_either(text[text.length()-5], suffix4L, suffix4U) &&
					is_either(text[text.length()-4], suffix5L, suffix5U) &&
					is_either(text[text.length()-3], suffix6L, suffix6U) &&
					is_either(text[text.length()-2], suffix7L, suffix7U) &&
					is_either(text[text.length()-1], suffix8L, suffix8U);
			}
		///is_suffix for nine characters
		inline bool is_suffix(const std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U,
					Tchar_type suffix5L, Tchar_type suffix5U,
					Tchar_type suffix6L, Tchar_type suffix6U,
					Tchar_type suffix7L, Tchar_type suffix7U,
					Tchar_type suffix8L, Tchar_type suffix8U,
					Tchar_type suffix9L, Tchar_type suffix9U)
			{
			if (text.length() < 9)
				{
				return false;
				}
			return is_either(text[text.length()-9], suffix1L, suffix1U) &&
					is_either(text[text.length()-8], suffix2L, suffix2U) &&
					is_either(text[text.length()-7], suffix3L, suffix3U) &&
					is_either(text[text.length()-6], suffix4L, suffix4U) &&
					is_either(text[text.length()-5], suffix5L, suffix5U) &&
					is_either(text[text.length()-4], suffix6L, suffix6U) &&
					is_either(text[text.length()-3], suffix7L, suffix7U) &&
					is_either(text[text.length()-2], suffix8L, suffix8U) &&
					is_either(text[text.length()-1], suffix9L, suffix9U);
			}

		///RV suffix functions
		//-------------------------------------------------
		///RV suffix comparison for one character
		inline bool is_suffix_in_rv(const std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U)
			{
			if (text.length() < 1)
				{
				return false;
				}
			return (is_either(text[text.length()-1], suffix1L, suffix1U) &&
					(m_rv <= text.length()-1) );
			}
		///RV suffix comparison for two characters
		inline bool is_suffix_in_rv(const std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U)
			{
			if (text.length() < 2)
				{
				return false;
				}
			return ((is_either(text[text.length()-2], suffix1L, suffix1U) &&
					is_either(text[text.length()-1], suffix2L, suffix2U) ) &&
					(m_rv <= text.length()-2) );
			}
		///RV suffix comparison for three characters
		inline bool is_suffix_in_rv(const std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U)
			{
			if (text.length() < 3)
				{
				return false;
				}
			return ((is_either(text[text.length()-3], suffix1L, suffix1U) &&
					is_either(text[text.length()-2], suffix2L, suffix2U) &&
					is_either(text[text.length()-1], suffix3L, suffix3U) ) &&
					(m_rv <= text.length()-3) );
			}
		///RV suffix comparison for four characters
		inline bool is_suffix_in_rv(const std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U)
			{
			if (text.length() < 4)
				{
				return false;
				}
			return ((is_either(text[text.length()-4], suffix1L, suffix1U) &&
					is_either(text[text.length()-3], suffix2L, suffix2U) &&
					is_either(text[text.length()-2], suffix3L, suffix3U) &&
					is_either(text[text.length()-1], suffix4L, suffix4U) ) &&
					(m_rv <= text.length()-4) );
			}
		///RV suffix comparison for five characters
		inline bool is_suffix_in_rv(const std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U,
					Tchar_type suffix5L, Tchar_type suffix5U)
			{
			if (text.length() < 5)
				{
				return false;
				}
			return ((is_either(text[text.length()-5], suffix1L, suffix1U) &&
					is_either(text[text.length()-4], suffix2L, suffix2U) &&
					is_either(text[text.length()-3], suffix3L, suffix3U) &&
					is_either(text[text.length()-2], suffix4L, suffix4U) &&
					is_either(text[text.length()-1], suffix5L, suffix5U) ) &&
					(m_rv <= text.length()-5) );
			}
		///RV suffix comparison for six characters
		inline bool is_suffix_in_rv(const std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U,
					Tchar_type suffix5L, Tchar_type suffix5U,
					Tchar_type suffix6L, Tchar_type suffix6U)
			{
			if (text.length() < 6)
				{
				return false;
				}
			return ((is_either(text[text.length()-6], suffix1L, suffix1U) &&
					is_either(text[text.length()-5], suffix2L, suffix2U) &&
					is_either(text[text.length()-4], suffix3L, suffix3U) &&
					is_either(text[text.length()-3], suffix4L, suffix4U) &&
					is_either(text[text.length()-2], suffix5L, suffix5U) &&
					is_either(text[text.length()-1], suffix6L, suffix6U) ) &&
					(m_rv <= text.length()-6) );
			}
		///RV suffix comparison for seven characters
		inline bool is_suffix_in_rv(const std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U,
					Tchar_type suffix5L, Tchar_type suffix5U,
					Tchar_type suffix6L, Tchar_type suffix6U,
					Tchar_type suffix7L, Tchar_type suffix7U)
			{
			if (text.length() < 7)
				{
				return false;
				}
			return ((is_either(text[text.length()-7], suffix1L, suffix1U) &&
					is_either(text[text.length()-6], suffix2L, suffix2U) &&
					is_either(text[text.length()-5], suffix3L, suffix3U) &&
					is_either(text[text.length()-4], suffix4L, suffix4U) &&
					is_either(text[text.length()-3], suffix5L, suffix5U) &&
					is_either(text[text.length()-2], suffix6L, suffix6U) &&
					is_either(text[text.length()-1], suffix7L, suffix7U) ) &&
					(m_rv <= text.length()-7) );
			}
		///RV suffix comparison for eight characters
		inline bool is_suffix_in_rv(const std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U,
					Tchar_type suffix5L, Tchar_type suffix5U,
					Tchar_type suffix6L, Tchar_type suffix6U,
					Tchar_type suffix7L, Tchar_type suffix7U,
					Tchar_type suffix8L, Tchar_type suffix8U)
			{
			if (text.length() < 8)
				{
				return false;
				}
			return ((is_either(text[text.length()-8], suffix1L, suffix1U) &&
					is_either(text[text.length()-7], suffix2L, suffix2U) &&
					is_either(text[text.length()-6], suffix3L, suffix3U) &&
					is_either(text[text.length()-5], suffix4L, suffix4U) &&
					is_either(text[text.length()-4], suffix5L, suffix5U) &&
					is_either(text[text.length()-3], suffix6L, suffix6U) &&
					is_either(text[text.length()-2], suffix7L, suffix7U) &&
					is_either(text[text.length()-1], suffix8L, suffix8U) ) &&
					(m_rv <= text.length()-8) );
			}
		//-------------------------------------------------
		inline bool is_suffix_in_rv(const std::basic_string<Tchar_type>& text,
					const Tchar_type* suffix, size_t suffix_length)
			{
			return (suffix_length > text.length() ) ?
				false :
				(string_util::strnicmp(text.c_str()+(text.length()-suffix_length), suffix, suffix_length) == 0 &&
				(m_rv <= text.length()-suffix_length) );
			}

		///R1 suffix functions
		//-------------------------------------------------
		///R1 suffix comparison for one character
		inline bool is_suffix_in_r1(const std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U)
			{
			if (text.length() < 1)
				{
				return false;
				}
			return (is_either(text[text.length()-1], suffix1L, suffix1U) &&
					(m_r1 <= text.length()-1) );
			}
		///R1 suffix comparison for two characters
		inline bool is_suffix_in_r1(const std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U)
			{
			if (text.length() < 2)
				{
				return false;
				}
			return ((is_either(text[text.length()-2], suffix1L, suffix1U) &&
					is_either(text[text.length()-1], suffix2L, suffix2U) ) &&
					(m_r1 <= text.length()-2) );
			}
		///R1 suffix comparison for three characters
		inline bool is_suffix_in_r1(const std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U)
			{
			if (text.length() < 3)
				{
				return false;
				}
			return ((is_either(text[text.length()-3], suffix1L, suffix1U) &&
					is_either(text[text.length()-2], suffix2L, suffix2U) &&
					is_either(text[text.length()-1], suffix3L, suffix3U) ) &&
					(m_r1 <= text.length()-3) );
			}
		///R1 suffix comparison for four characters
		inline bool is_suffix_in_r1(const std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U)
			{
			if (text.length() < 4)
				{
				return false;
				}
			return ((is_either(text[text.length()-4], suffix1L, suffix1U) &&
					is_either(text[text.length()-3], suffix2L, suffix2U) &&
					is_either(text[text.length()-2], suffix3L, suffix3U) &&
					is_either(text[text.length()-1], suffix4L, suffix4U) ) &&
					(m_r1 <= text.length()-4) );
			}
		///R1 suffix comparison for five characters
		inline bool is_suffix_in_r1(const std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U,
					Tchar_type suffix5L, Tchar_type suffix5U)
			{
			if (text.length() < 5)
				{
				return false;
				}
			return ((is_either(text[text.length()-5], suffix1L, suffix1U) &&
					is_either(text[text.length()-4], suffix2L, suffix2U) &&
					is_either(text[text.length()-3], suffix3L, suffix3U) &&
					is_either(text[text.length()-2], suffix4L, suffix4U) &&
					is_either(text[text.length()-1], suffix5L, suffix5U) ) &&
					(m_r1 <= text.length()-5) );
			}
		///R1 suffix comparison for six characters
		inline bool is_suffix_in_r1(const std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U,
					Tchar_type suffix5L, Tchar_type suffix5U,
					Tchar_type suffix6L, Tchar_type suffix6U)
			{
			if (text.length() < 6)
				{
				return false;
				}
			return ((is_either(text[text.length()-6], suffix1L, suffix1U) &&
					is_either(text[text.length()-5], suffix2L, suffix2U) &&
					is_either(text[text.length()-4], suffix3L, suffix3U) &&
					is_either(text[text.length()-3], suffix4L, suffix4U) &&
					is_either(text[text.length()-2], suffix5L, suffix5U) &&
					is_either(text[text.length()-1], suffix6L, suffix6U) ) &&
					(m_r1 <= text.length()-6) );
			}

		inline bool is_suffix_in_r1(const std::basic_string<Tchar_type>& text,
					const Tchar_type* suffix, size_t suffix_length)
			{
			if (suffix_length > text.length() )
				{
				return false;
				}
			return (string_util::strnicmp(text.c_str()+(text.length()-suffix_length), suffix, suffix_length) == 0 &&
					(m_r1 <= text.length()-suffix_length) );
			}

		///R2 suffix functions
		//-------------------------------------------------
		///R2 suffix comparison for one character
		inline bool is_suffix_in_r2(const std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U)
			{
			if (text.length() < 1)
				{
				return false;
				}
			return (is_either(text[text.length()-1], suffix1L, suffix1U) &&
					(m_r2 <= text.length()-1) );
			}
		///R2 suffix comparison for two characters
		inline bool is_suffix_in_r2(const std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U)
			{
			if (text.length() < 2)
				{
				return false;
				}
			return ((is_either(text[text.length()-2], suffix1L, suffix1U) &&
					is_either(text[text.length()-1], suffix2L, suffix2U) ) &&
					(m_r2 <= text.length()-2) );
			}
		///R2 suffix comparison for three characters
		inline bool is_suffix_in_r2(const std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U)
			{
			if (text.length() < 3)
				{
				return false;
				}
			return ((is_either(text[text.length()-3], suffix1L, suffix1U) &&
					is_either(text[text.length()-2], suffix2L, suffix2U) &&
					is_either(text[text.length()-1], suffix3L, suffix3U) ) &&
					(m_r2 <= text.length()-3) );
			}
		///R2 suffix comparison for four characters
		inline bool is_suffix_in_r2(const std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U)
			{
			if (text.length() < 4)
				{
				return false;
				}
			return ((is_either(text[text.length()-4], suffix1L, suffix1U) &&
					is_either(text[text.length()-3], suffix2L, suffix2U) &&
					is_either(text[text.length()-2], suffix3L, suffix3U) &&
					is_either(text[text.length()-1], suffix4L, suffix4U) ) &&
					(m_r2 <= text.length()-4) );
			}
		///R2 suffix comparison for five characters
		inline bool is_suffix_in_r2(const std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U,
					Tchar_type suffix5L, Tchar_type suffix5U)
			{
			if (text.length() < 5)
				{
				return false;
				}
			return ((is_either(text[text.length()-5], suffix1L, suffix1U) &&
					is_either(text[text.length()-4], suffix2L, suffix2U) &&
					is_either(text[text.length()-3], suffix3L, suffix3U) &&
					is_either(text[text.length()-2], suffix4L, suffix4U) &&
					is_either(text[text.length()-1], suffix5L, suffix5U) ) &&
					(m_r2 <= text.length()-5) );
			}
		///R2 suffix comparison for six characters
		inline bool is_suffix_in_r2(std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U,
					Tchar_type suffix5L, Tchar_type suffix5U,
					Tchar_type suffix6L, Tchar_type suffix6U)
			{
			if (text.length() < 6)
				{
				return false;
				}
			return ((is_either(text[text.length()-6], suffix1L, suffix1U) &&
					is_either(text[text.length()-5], suffix2L, suffix2U) &&
					is_either(text[text.length()-4], suffix3L, suffix3U) &&
					is_either(text[text.length()-3], suffix4L, suffix4U) &&
					is_either(text[text.length()-2], suffix5L, suffix5U) &&
					is_either(text[text.length()-1], suffix6L, suffix6U) ) &&
					(m_r2 <= text.length()-6) );
			}
		///R2 suffix comparison for seven characters
		inline bool is_suffix_in_r2(const std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U,
					Tchar_type suffix5L, Tchar_type suffix5U,
					Tchar_type suffix6L, Tchar_type suffix6U,
					Tchar_type suffix7L, Tchar_type suffix7U)
			{
			if (text.length() < 7)
				{
				return false;
				}
			return ((is_either(text[text.length()-7], suffix1L, suffix1U) &&
					is_either(text[text.length()-6], suffix2L, suffix2U) &&
					is_either(text[text.length()-5], suffix3L, suffix3U) &&
					is_either(text[text.length()-4], suffix4L, suffix4U) &&
					is_either(text[text.length()-3], suffix5L, suffix5U) &&
					is_either(text[text.length()-2], suffix6L, suffix6U) &&
					is_either(text[text.length()-1], suffix7L, suffix7U) ) &&
					(m_r2 <= text.length()-7) );
			}
		//-------------------------------------------------
		inline bool is_suffix_in_r2(const std::basic_string<Tchar_type>& text,
					const Tchar_type* suffix, size_t suffix_length)
			{
			assert(string_util::strlen(suffix) == suffix_length);
			if (suffix_length > text.length() )
				{
				return false;
				}
			return (string_util::strnicmp(text.c_str()+(text.length()-suffix_length), suffix, suffix_length) == 0 &&
					(m_r2 <= text.length()-suffix_length) );
			}

		//suffix removal functions
		//R1 deletion for one character suffix
		inline bool delete_if_is_in_r1(std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					bool success_on_find = true)
			{
			assert(suffix1L == string_util::tolower_western(suffix1U) );
			if (text.length() < 1)
				{
				return false;
				}
			if (is_either(text[text.length()-1], suffix1L, suffix1U))
				{
				if (m_r1 <= text.length()-1)
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					return true;
					}
				return success_on_find;
				}
			else
				{
				return false;
				}
			}
		//R1 deletion for two character suffix
		inline bool delete_if_is_in_r1(std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					bool success_on_find = true)
			{
			if (text.length() < 2)
				{
				return false;
				}
			if (is_either(text[text.length()-2], suffix1L, suffix1U) &&
				is_either(text[text.length()-1], suffix2L, suffix2U))
				{
				if (m_r1 <= text.length()-2)
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					return true;
					}
				return success_on_find;
				}
			else
				{
				return false;
				}
			}
		//R1 deletion for three character suffix
		inline bool delete_if_is_in_r1(std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					bool success_on_find = true)
			{
			if (text.length() < 3)
				{
				return false;
				}
			if (is_either(text[text.length()-3], suffix1L, suffix1U) &&
				is_either(text[text.length()-2], suffix2L, suffix2U) &&
				is_either(text[text.length()-1], suffix3L, suffix3U) )
				{
				if (m_r1 <= text.length()-3)
					{
					text.erase(text.end()-3, text.end() );
					update_r_sections(text);
					return true;
					}
				return success_on_find;
				}
			else
				{
				return false;
				}
			}
		//R1 deletion for four character suffix
		inline bool delete_if_is_in_r1(std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U,
					bool success_on_find = true)
			{
			if (text.length() < 4)
				{
				return false;
				}
			if (is_either(text[text.length()-4], suffix1L, suffix1U) &&
				is_either(text[text.length()-3], suffix2L, suffix2U) &&
				is_either(text[text.length()-2], suffix3L, suffix3U) &&
				is_either(text[text.length()-1], suffix4L, suffix4U) )
				{
				if (m_r1 <= text.length()-4)
					{
					text.erase(text.end()-4, text.end() );
					update_r_sections(text);
					return true;
					}
				return success_on_find;
				}
			else
				{
				return false;
				}
			}
		//R1 deletion for five character suffix
		inline bool delete_if_is_in_r1(std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U,
					Tchar_type suffix5L, Tchar_type suffix5U,
					bool success_on_find = true)
			{
			if (text.length() < 5)
				{
				return false;
				}
			if (is_either(text[text.length()-5], suffix1L, suffix1U) &&
				is_either(text[text.length()-4], suffix2L, suffix2U) &&
				is_either(text[text.length()-3], suffix3L, suffix3U) &&
				is_either(text[text.length()-2], suffix4L, suffix4U) &&
				is_either(text[text.length()-1], suffix5L, suffix5U) )
				{
				if (m_r1 <= text.length()-5)
					{
					text.erase(text.end()-5, text.end() );
					update_r_sections(text);
					return true;
					}
				return success_on_find;
				}
			else
				{
				return false;
				}
			}
		//R1 deletion for six character suffix
		inline bool delete_if_is_in_r1(std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U,
					Tchar_type suffix5L, Tchar_type suffix5U,
					Tchar_type suffix6L, Tchar_type suffix6U,
					bool success_on_find = true)
			{
			if (text.length() < 6)
				{
				return false;
				}
			if (is_either(text[text.length()-6], suffix1L, suffix1U) &&
				is_either(text[text.length()-5], suffix2L, suffix2U) &&
				is_either(text[text.length()-4], suffix3L, suffix3U) &&
				is_either(text[text.length()-3], suffix4L, suffix4U) &&
				is_either(text[text.length()-2], suffix5L, suffix5U) &&
				is_either(text[text.length()-1], suffix6L, suffix6U) )
				{
				if (m_r1 <= text.length()-6)
					{
					text.erase(text.end()-6, text.end() );
					update_r_sections(text);
					return true;
					}
				return success_on_find;
				}
			else
				{
				return false;
				}
			}
		//R1 deletion for seven character suffix
		inline bool delete_if_is_in_r1(std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U,
					Tchar_type suffix5L, Tchar_type suffix5U,
					Tchar_type suffix6L, Tchar_type suffix6U,
					Tchar_type suffix7L, Tchar_type suffix7U,
					bool success_on_find = true)
			{
			if (text.length() < 7)
				{
				return false;
				}
			if (is_either(text[text.length()-7], suffix1L, suffix1U) &&
				is_either(text[text.length()-6], suffix2L, suffix2U) &&
				is_either(text[text.length()-5], suffix3L, suffix3U) &&
				is_either(text[text.length()-4], suffix4L, suffix4U) &&
				is_either(text[text.length()-3], suffix5L, suffix5U) &&
				is_either(text[text.length()-2], suffix6L, suffix6U) &&
				is_either(text[text.length()-1], suffix7L, suffix7U) )
				{
				if (m_r1 <= text.length()-7)
					{
					text.erase(text.end()-7, text.end() );
					update_r_sections(text);
					return true;
					}
				return success_on_find;
				}
			else
				{
				return false;
				}
			}

		inline bool delete_if_is_in_r1(std::basic_string<Tchar_type>& text,
					const Tchar_type* suffix, size_t suffix_length,
					bool success_on_find = true)
			{
			if (suffix_length > text.length() )
				{
				return false;
				}
			if (string_util::strnicmp(text.c_str()+(text.length()-suffix_length), suffix, suffix_length) == 0)
				{
				if (m_r1 <= text.length()-suffix_length)
					{
					text.erase(text.end()-suffix_length, text.end() );
					update_r_sections(text);
					return true;
					}
				return success_on_find;
				}
			else
				{
				return false;
				}
			}

		//R2 deletion functions
		//R2 deletion for one character suffix
		inline bool delete_if_is_in_r2(std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					bool success_on_find = true)
			{
			if (text.length() < 1)
				{
				return false;
				}
			if (is_either(text[text.length()-1], suffix1L, suffix1U))
				{
				if (m_r2 <= text.length()-1)
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					return true;
					}
				return success_on_find;
				}
			else
				{
				return false;
				}
			}
		//R2 deletion for two character suffix
		inline bool delete_if_is_in_r2(std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					bool success_on_find = true)
			{
			if (text.length() < 2)
				{
				return false;
				}
			if (is_either(text[text.length()-2], suffix1L, suffix1U) &&
				is_either(text[text.length()-1], suffix2L, suffix2U))
				{
				if (m_r2 <= text.length()-2)
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					return true;
					}
				return success_on_find;
				}
			else
				{
				return false;
				}
			}
		//R2 deletion for three character suffix
		inline bool delete_if_is_in_r2(std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					bool success_on_find = true)
			{
			if (text.length() < 3)
				{
				return false;
				}
			if (is_either(text[text.length()-3], suffix1L, suffix1U) &&
				is_either(text[text.length()-2], suffix2L, suffix2U) &&
				is_either(text[text.length()-1], suffix3L, suffix3U) )
				{
				if (m_r2 <= text.length()-3)
					{
					text.erase(text.end()-3, text.end() );
					update_r_sections(text);
					return true;
					}
				return success_on_find;
				}
			else
				{
				return false;
				}
			}
		//R2 deletion for four character suffix
		inline bool delete_if_is_in_r2(std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U,
					bool success_on_find = true)
			{
			if (text.length() < 4)
				{
				return false;
				}
			if (is_either(text[text.length()-4], suffix1L, suffix1U) &&
				is_either(text[text.length()-3], suffix2L, suffix2U) &&
				is_either(text[text.length()-2], suffix3L, suffix3U) &&
				is_either(text[text.length()-1], suffix4L, suffix4U) )
				{
				if (m_r2 <= text.length()-4)
					{
					text.erase(text.end()-4, text.end() );
					update_r_sections(text);
					return true;
					}
				return success_on_find;
				}
			else
				{
				return false;
				}
			}
		///R2 deletion for five character suffix
		inline bool delete_if_is_in_r2(std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U,
					Tchar_type suffix5L, Tchar_type suffix5U,
					bool success_on_find = true)
			{
			if (text.length() < 5)
				{
				return false;
				}
			if (is_either(text[text.length()-5], suffix1L, suffix1U) &&
				is_either(text[text.length()-4], suffix2L, suffix2U) &&
				is_either(text[text.length()-3], suffix3L, suffix3U) &&
				is_either(text[text.length()-2], suffix4L, suffix4U) &&
				is_either(text[text.length()-1], suffix5L, suffix5U) )
				{
				if (m_r2 <= text.length()-5)
					{
					text.erase(text.end()-5, text.end() );
					update_r_sections(text);
					return true;
					}
				return success_on_find;
				}
			else
				{
				return false;
				}
			}
		///R2 deletion for six character suffix
		inline bool delete_if_is_in_r2(std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U,
					Tchar_type suffix5L, Tchar_type suffix5U,
					Tchar_type suffix6L, Tchar_type suffix6U,
					bool success_on_find = true)
			{
			if (text.length() < 6)
				{
				return false;
				}
			if (is_either(text[text.length()-6], suffix1L, suffix1U) &&
				is_either(text[text.length()-5], suffix2L, suffix2U) &&
				is_either(text[text.length()-4], suffix3L, suffix3U) &&
				is_either(text[text.length()-3], suffix4L, suffix4U) &&
				is_either(text[text.length()-2], suffix5L, suffix5U) &&
				is_either(text[text.length()-1], suffix6L, suffix6U) )
				{
				if (m_r2 <= text.length()-6)
					{
					text.erase(text.end()-6, text.end() );
					update_r_sections(text);
					return true;
					}
				return success_on_find;
				}
			else
				{
				return false;
				}
			}
		///R2 deletion for seven character suffix
		inline bool delete_if_is_in_r2(std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U,
					Tchar_type suffix5L, Tchar_type suffix5U,
					Tchar_type suffix6L, Tchar_type suffix6U,
					Tchar_type suffix7L, Tchar_type suffix7U,
					bool success_on_find = true)
			{
			if (text.length() < 7)
				{
				return false;
				}
			if (is_either(text[text.length()-7], suffix1L, suffix1U) &&
				is_either(text[text.length()-6], suffix2L, suffix2U) &&
				is_either(text[text.length()-5], suffix3L, suffix3U) &&
				is_either(text[text.length()-4], suffix4L, suffix4U) &&
				is_either(text[text.length()-3], suffix5L, suffix5U) &&
				is_either(text[text.length()-2], suffix6L, suffix6U) &&
				is_either(text[text.length()-1], suffix7L, suffix7U) )
				{
				if (m_r2 <= text.length()-7)
					{
					text.erase(text.end()-7, text.end() );
					update_r_sections(text);
					return true;
					}
				return success_on_find;
				}
			else
				{
				return false;
				}
			}
		///R2 deletion for eight character suffix
		inline bool delete_if_is_in_r2(std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U,
					Tchar_type suffix5L, Tchar_type suffix5U,
					Tchar_type suffix6L, Tchar_type suffix6U,
					Tchar_type suffix7L, Tchar_type suffix7U,
					Tchar_type suffix8L, Tchar_type suffix8U,
					bool success_on_find = true)
			{
			if (text.length() < 8)
				{
				return false;
				}
			if (is_either(text[text.length()-8], suffix1L, suffix1U) &&
				is_either(text[text.length()-7], suffix2L, suffix2U) &&
				is_either(text[text.length()-6], suffix3L, suffix3U) &&
				is_either(text[text.length()-5], suffix4L, suffix4U) &&
				is_either(text[text.length()-4], suffix5L, suffix5U) &&
				is_either(text[text.length()-3], suffix6L, suffix6U) &&
				is_either(text[text.length()-2], suffix7L, suffix7U) &&
				is_either(text[text.length()-1], suffix8L, suffix8U) )
				{
				if (m_r2 <= text.length()-8)
					{
					text.erase(text.end()-8, text.end() );
					update_r_sections(text);
					return true;
					}
				return success_on_find;
				}
			else
				{
				return false;
				}
			}
		
		inline bool delete_if_is_in_r2(std::basic_string<Tchar_type>& text,
					const Tchar_type* suffix, size_t suffix_length,
					bool success_on_find = true)
			{
			if (suffix_length > text.length() )
				{
				return false;
				}
			if (string_util::strnicmp(text.c_str()+(text.length()-suffix_length), suffix, suffix_length) == 0)
				{
				if (m_r2 <= text.length()-suffix_length)
					{
					text.erase(text.end()-suffix_length, text.end() );
					update_r_sections(text);
					return true;
					}
				return success_on_find;
				}
			else
				{
				return false;
				}
			}

		//RV deletion functions
		//RV deletion for one character suffix
		inline bool delete_if_is_in_rv(std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					bool success_on_find = true)
			{
			if (text.length() < 1)
				{
				return false;
				}
			if (is_either(text[text.length()-1], suffix1L, suffix1U))
				{
				if (m_rv <= text.length()-1)
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					return true;
					}
				return success_on_find;
				}
			else
				{
				return false;
				}
			}
		//RV deletion for two character suffix
		inline bool delete_if_is_in_rv(std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					bool success_on_find = true)
			{
			if (text.length() < 2)
				{
				return false;
				}
			if (is_either(text[text.length()-2], suffix1L, suffix1U) &&
				is_either(text[text.length()-1], suffix2L, suffix2U))
				{
				if (m_rv <= text.length()-2)
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					return true;
					}
				return success_on_find;
				}
			else
				{
				return false;
				}
			}
		//RV deletion for three character suffix
		inline bool delete_if_is_in_rv(std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					bool success_on_find = true)
			{
			if (text.length() < 3)
				{
				return false;
				}
			if (is_either(text[text.length()-3], suffix1L, suffix1U) &&
				is_either(text[text.length()-2], suffix2L, suffix2U) &&
				is_either(text[text.length()-1], suffix3L, suffix3U) )
				{
				if (m_rv <= text.length()-3)
					{
					text.erase(text.end()-3, text.end() );
					update_r_sections(text);
					return true;
					}
				return success_on_find;
				}
			else
				{
				return false;
				}
			}
		//RV deletion for four character suffix
		inline bool delete_if_is_in_rv(std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U,
					bool success_on_find = true)
			{
			if (text.length() < 4)
				{
				return false;
				}
			if (is_either(text[text.length()-4], suffix1L, suffix1U) &&
				is_either(text[text.length()-3], suffix2L, suffix2U) &&
				is_either(text[text.length()-2], suffix3L, suffix3U) &&
				is_either(text[text.length()-1], suffix4L, suffix4U) )
				{
				if (m_rv <= text.length()-4)
					{
					text.erase(text.end()-4, text.end() );
					update_r_sections(text);
					return true;
					}
				return success_on_find;
				}
			else
				{
				return false;
				}
			}
		//RV deletion for five character suffix
		inline bool delete_if_is_in_rv(std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U,
					Tchar_type suffix5L, Tchar_type suffix5U,
					bool success_on_find = true)
			{
			if (text.length() < 5)
				{
				return false;
				}
			if (is_either(text[text.length()-5], suffix1L, suffix1U) &&
				is_either(text[text.length()-4], suffix2L, suffix2U) &&
				is_either(text[text.length()-3], suffix3L, suffix3U) &&
				is_either(text[text.length()-2], suffix4L, suffix4U) &&
				is_either(text[text.length()-1], suffix5L, suffix5U) )
				{
				if (m_rv <= text.length()-5)
					{
					text.erase(text.end()-5, text.end() );
					update_r_sections(text);
					return true;
					}
				return success_on_find;
				}
			else
				{
				return false;
				}
			}
		//RV deletion for six character suffix
		inline bool delete_if_is_in_rv(std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U,
					Tchar_type suffix5L, Tchar_type suffix5U,
					Tchar_type suffix6L, Tchar_type suffix6U,
					bool success_on_find = true)
			{
			if (text.length() < 6)
				{
				return false;
				}
			if (is_either(text[text.length()-6], suffix1L, suffix1U) &&
				is_either(text[text.length()-5], suffix2L, suffix2U) &&
				is_either(text[text.length()-4], suffix3L, suffix3U) &&
				is_either(text[text.length()-3], suffix4L, suffix4U) &&
				is_either(text[text.length()-2], suffix5L, suffix5U) &&
				is_either(text[text.length()-1], suffix6L, suffix6U) )
				{
				if (m_rv <= text.length()-6)
					{
					text.erase(text.end()-6, text.end() );
					update_r_sections(text);
					return true;
					}
				return success_on_find;
				}
			else
				{
				return false;
				}
			}
		//RV deletion for seven character suffix
		inline bool delete_if_is_in_rv(std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U,
					Tchar_type suffix5L, Tchar_type suffix5U,
					Tchar_type suffix6L, Tchar_type suffix6U,
					Tchar_type suffix7L, Tchar_type suffix7U,
					bool success_on_find = true)
			{
			if (text.length() < 7)
				{
				return false;
				}
			if (is_either(text[text.length()-7], suffix1L, suffix1U) &&
				is_either(text[text.length()-6], suffix2L, suffix2U) &&
				is_either(text[text.length()-5], suffix3L, suffix3U) &&
				is_either(text[text.length()-4], suffix4L, suffix4U) &&
				is_either(text[text.length()-3], suffix5L, suffix5U) &&
				is_either(text[text.length()-2], suffix6L, suffix6U) &&
				is_either(text[text.length()-1], suffix7L, suffix7U) )
				{
				if (m_rv <= text.length()-7)
					{
					text.erase(text.end()-7, text.end() );
					update_r_sections(text);
					return true;
					}
				return success_on_find;
				}
			else
				{
				return false;
				}
			}
		//RV deletion for eight character suffix
		inline bool delete_if_is_in_rv(std::basic_string<Tchar_type>& text,
					Tchar_type suffix1L, Tchar_type suffix1U,
					Tchar_type suffix2L, Tchar_type suffix2U,
					Tchar_type suffix3L, Tchar_type suffix3U,
					Tchar_type suffix4L, Tchar_type suffix4U,
					Tchar_type suffix5L, Tchar_type suffix5U,
					Tchar_type suffix6L, Tchar_type suffix6U,
					Tchar_type suffix7L, Tchar_type suffix7U,
					Tchar_type suffix8L, Tchar_type suffix8U,
					bool success_on_find = true)
			{
			if (text.length() < 8)
				{
				return false;
				}
			if (is_either(text[text.length()-8], suffix1L, suffix1U) &&
				is_either(text[text.length()-7], suffix2L, suffix2U) &&
				is_either(text[text.length()-6], suffix3L, suffix3U) &&
				is_either(text[text.length()-5], suffix4L, suffix4U) &&
				is_either(text[text.length()-4], suffix5L, suffix5U) &&
				is_either(text[text.length()-3], suffix6L, suffix6U) &&
				is_either(text[text.length()-2], suffix7L, suffix7U) &&
				is_either(text[text.length()-1], suffix8L, suffix8U) )
				{
				if (m_rv <= text.length()-8)
					{
					text.erase(text.end()-8, text.end() );
					update_r_sections(text);
					return true;
					}
				return success_on_find;
				}
			else
				{
				return false;
				}
			}

		inline bool delete_if_is_in_rv(std::basic_string<Tchar_type>& text,
					const Tchar_type* suffix, size_t suffix_length,
					bool success_on_find = true)
			{
			if (suffix_length > text.length() )
				{
				return false;
				}
			if (string_util::strnicmp(text.c_str()+(text.length()-suffix_length), suffix, suffix_length) == 0)
				{
				if (m_rv <= text.length()-suffix_length)
					{
					text.erase(text.end()-suffix_length, text.end() );
					update_r_sections(text);
					return true;
					}
				return success_on_find;
				}
			else
				{
				return false;
				}
			}

		//string support functions
		void remove_german_umlauts(std::basic_string<Tchar_type>& text)
			{
			for (size_t i = 0; i < text.length(); ++i)
				{
				if (text[i] == assign_character(Tchar_type, 0xC4))
					{
					text[i] = UPPER_A;
					}
				else if (text[i] == assign_character(Tchar_type, 0xD6))
					{
					text[i] = UPPER_O;
					}
				else if (text[i] == assign_character(Tchar_type, 0xDC))
					{
					text[i] = UPPER_U;
					}
				else if (text[i] == assign_character(Tchar_type, 0xE4) )
					{
					text[i] = LOWER_A;
					}
				else if (text[i] == assign_character(Tchar_type, 0xF6))
					{
					text[i] = LOWER_O;
					}
				else if (text[i] == assign_character(Tchar_type, 0xFC))
					{
					text[i] = LOWER_U;
					}
				}
			}
		void italian_acutes_to_graves(std::basic_string<Tchar_type>& text)
			{
			for (size_t i = 0; i < text.length(); ++i)
				{
				if (text[i] == assign_character(Tchar_type, 0xC1))//A acute
					{
					text[i] = assign_character(Tchar_type, 0xC0);
					}
				else if (text[i] == assign_character(Tchar_type, 0xC9))//E acute
					{
					text[i] = assign_character(Tchar_type, 0xC8);
					}
				else if (text[i] == assign_character(Tchar_type, 0xCD))//I acute
					{
					text[i] = assign_character(Tchar_type, 0xCC);
					}
				else if (text[i] == assign_character(Tchar_type, 0xD3))//O acute
					{
					text[i] = assign_character(Tchar_type, 0xD2);
					}
				else if (text[i] == assign_character(Tchar_type, 0xDA))//U acute
					{
					text[i] = assign_character(Tchar_type, 0xD9);
					}
				else if (text[i] == assign_character(Tchar_type, 0xE1))//a acute
					{
					text[i] = assign_character(Tchar_type, 0xE0);
					}
				else if (text[i] == assign_character(Tchar_type, 0xE9))//e acute
					{
					text[i] = assign_character(Tchar_type, 0xE8);
					}
				else if (text[i] == assign_character(Tchar_type, 0xED))//i acute
					{
					text[i] =  assign_character(Tchar_type, 0xEC);
					}
				else if (text[i] == assign_character(Tchar_type, 0xF3))//o acute
					{
					text[i] = assign_character(Tchar_type, 0xF2);
					}
				else if (text[i] == assign_character(Tchar_type, 0xFA))//u acute
					{
					text[i] = assign_character(Tchar_type, 0xF9);
					}
				}
			}
		///Put initial y, y after a vowel, and i between vowels into hashed character.
		//----------------------------------------------------------
		void hash_dutch_yi(std::basic_string<Tchar_type>& text,
					const Tchar_type* vowel_string)
			{		
			//need at least 2 letters for hashing
			if (text.length() < 2)
				{ return; }

			if (text[0] == LOWER_Y)
				{
				text[0] = LOWER_Y_HASH;
				}
			else if (text[0] == UPPER_Y)
				{
				text[0] = UPPER_Y_HASH;
				}
			bool in_vowel_block = string_util::is_one_of(text[0], vowel_string);

			size_t i = 1;
			for (i = 1; i < text.length()-1; ++i)
				{
				if (in_vowel_block &&
					text[i] == LOWER_I &&
					string_util::is_one_of(text[i+1], vowel_string) )
					{
					text[i] = LOWER_I_HASH;
					in_vowel_block = false;
					}
				else if (in_vowel_block &&
					text[i] == UPPER_I &&
					string_util::is_one_of(text[i+1], vowel_string) )
					{
					text[i] = UPPER_I_HASH;
					in_vowel_block = false;
					}
				else if (in_vowel_block &&
					text[i] == LOWER_Y)
					{
					text[i] = LOWER_Y_HASH;
					in_vowel_block = false;
					}
				else if (in_vowel_block &&
					text[i] == UPPER_Y)
					{
					text[i] = UPPER_Y_HASH;
					in_vowel_block = false;
					}
				else if (string_util::is_one_of(text[i], vowel_string) )
					{
					in_vowel_block = true;
					}
				else
					{
					in_vowel_block = false;
					}
				}
			//check the last letter
			if (in_vowel_block &&
				text[i] == LOWER_Y)
				{
				text[i] = LOWER_Y_HASH;
				in_vowel_block = false;
				}
			else if (in_vowel_block &&
				text[i] == UPPER_Y)
				{
				text[i] = UPPER_Y_HASH;
				in_vowel_block = false;
				}
			}

		//----------------------------------------------------------
		inline void unhash_dutch_yi(std::basic_string<Tchar_type>& text)
			{
			string_util::replace_all(text, LOWER_Y_HASH, LOWER_Y);
			string_util::replace_all(text, UPPER_Y_HASH, UPPER_Y);
			string_util::replace_all(text, LOWER_I_HASH, LOWER_I);
			string_util::replace_all(text, UPPER_I_HASH, UPPER_I);
			}

		//----------------------------------------------------------
		void hash_german_yu(std::basic_string<Tchar_type>& text,
					const Tchar_type* vowel_string)
			{
			//need at least 2 letters for hashing
			if (text.length() < 2)
				{ return; }

			bool in_vowel_block = string_util::is_one_of(text[0], vowel_string);

			for (size_t i = 1; i < text.length()-1; ++i)
				{
				if (in_vowel_block &&
					string_util::is_one_of(text[i], vowel_string) &&
					string_util::is_one_of(text[i+1], vowel_string) )
					{
					if (text[i] == LOWER_Y)
						{
						text[i] = LOWER_Y_HASH;
						}
					else if (text[i] == UPPER_Y)
						{
						text[i] = UPPER_Y_HASH;
						}
					else if (text[i] == LOWER_U)
						{
						text[i] = LOWER_U_HASH;
						}
					else if (text[i] == UPPER_U)
						{
						text[i] = UPPER_U_HASH;
						}
					}
				else if (string_util::is_one_of(text[i], vowel_string) )
					{
					in_vowel_block = true;
					}
				else
					{
					in_vowel_block = false;
					}
				}
			}

		//----------------------------------------------------------
		inline void unhash_german_yu(std::basic_string<Tchar_type>& text)
			{
			string_util::replace_all(text, LOWER_Y_HASH, LOWER_Y);
			string_util::replace_all(text, UPPER_Y_HASH, UPPER_Y);
			string_util::replace_all(text, LOWER_U_HASH, LOWER_U);
			string_util::replace_all(text, UPPER_U_HASH, UPPER_U);
			}

		//----------------------------------------------------------
		void hash_french_yui(std::basic_string<Tchar_type>& text,
					const Tchar_type* vowel_string)
			{
			//need at least 2 letters for hashing
			if (text.length() < 2)
				{ return; }

			bool in_vowel_block = false;

			size_t i = 1;
			for (i = 0; i < text.length()-1; ++i)
				{
				if (in_vowel_block &&
					string_util::is_one_of(text[i], vowel_string) &&
					string_util::is_one_of(text[i+1], vowel_string) )
					{
					if (text[i] == LOWER_Y)
						{
						text[i] = LOWER_Y_HASH;
						in_vowel_block = false;
						}
					else if (text[i] == UPPER_Y)
						{
						text[i] = UPPER_Y_HASH;
						in_vowel_block = false;
						}
					else if (text[i] == LOWER_U)
						{
						text[i] = LOWER_U_HASH;
						in_vowel_block = false;
						}
					else if (text[i] == UPPER_U)
						{
						text[i] = UPPER_U_HASH;
						in_vowel_block = false;
						}
					else if (text[i] == LOWER_I)
						{
						text[i] = LOWER_I_HASH;
						in_vowel_block = false;
						}
					else if (text[i] == UPPER_I)
						{
						text[i] = UPPER_I_HASH;
						in_vowel_block = false;
						}
					}
				//if just previous letter is a vowel then examine for LOWER_Y
				else if (in_vowel_block &&
						text[i] == LOWER_Y)
					{
					text[i] = LOWER_Y_HASH;
					in_vowel_block = false;
					}
				else if (in_vowel_block &&
						text[i] == UPPER_Y)
					{
					text[i] = UPPER_Y_HASH;
					in_vowel_block = false;
					}
				//if just following letter is a vowel then examine for LOWER_Y
				else if (text[i] == LOWER_Y &&
						string_util::is_one_of(text[i+1], vowel_string) &&
						is_neither(text[i+1], LOWER_Y, UPPER_Y) )
					{
					text[i] = LOWER_Y_HASH;
					in_vowel_block = false;
					}
				else if (text[i] == UPPER_Y &&
						string_util::is_one_of(text[i+1], vowel_string) &&
						is_neither(text[i+1], LOWER_Y, UPPER_Y) )
					{
					text[i] = UPPER_Y_HASH;
					in_vowel_block = false;
					}
				else if (string_util::is_one_of(text[i], vowel_string) )
					{
					if (text[i] == LOWER_U &&
						is_either(text[i-1], LOWER_Q, UPPER_Q) )
						{
						text[i] = LOWER_U_HASH;
						in_vowel_block = false;
						}
					else if (text[i] == UPPER_U &&
						is_either(text[i-1], LOWER_Q, UPPER_Q) )
						{
						text[i] = UPPER_U_HASH;
						in_vowel_block = false;
						}
					else
						{
						in_vowel_block = true;
						}
					}
				else
					{
					in_vowel_block = false;
					}
				}
			//verify that the last letter
			if (text[i] == LOWER_Y &&
				string_util::is_one_of(text[i-1], vowel_string)	)
				{
				text[i] = LOWER_Y_HASH;
				}
			else if (text[i] == UPPER_Y &&
					string_util::is_one_of(text[i-1], vowel_string) )
				{
				text[i] = UPPER_Y_HASH;
				}
			else if (text[i] == LOWER_U &&
					is_either(text[i-1], LOWER_Q, UPPER_Q) )
				{
				text[i] = LOWER_U_HASH;
				}
			else if (text[i] == UPPER_U &&
					is_either(text[i-1], LOWER_Q, UPPER_Q) )
				{
				text[i] = UPPER_U_HASH;
				}
			}

		void unhash_french_yui(std::basic_string<Tchar_type>& text)
			{
			string_util::replace_all(text, LOWER_Y_HASH, LOWER_Y);
			string_util::replace_all(text, UPPER_Y_HASH, UPPER_Y);
			string_util::replace_all(text, LOWER_U_HASH, LOWER_U);
			string_util::replace_all(text, UPPER_U_HASH, UPPER_U);
			string_util::replace_all(text, LOWER_I_HASH, LOWER_I);
			string_util::replace_all(text, UPPER_I_HASH, UPPER_I);
			}

		//----------------------------------------------------------
		void hash_y(std::basic_string<Tchar_type>& text,
					const Tchar_type* vowel_string)
			{
			//need at least 2 letters for hashing
			if (text.length() < 2)
				{ return; }

			//if first letter is a LOWER_Y, then it is not a vowel
			if (text[0] == LOWER_Y &&
				string_util::is_one_of(text[1], vowel_string) )
				{
				text[0] = LOWER_Y_HASH;
				}
			else if (text[0] == UPPER_Y &&
				string_util::is_one_of(text[1], vowel_string))
				{
				text[0] = UPPER_Y_HASH;
				}

			bool in_vowel_block = string_util::is_one_of(text[0], vowel_string);

			for (size_t i = 1; i < text.length(); ++i)
				{
				//LOWER_Y after vowel is a consonant
				if (in_vowel_block &&
					text[i] == LOWER_Y)
					{
					text[i] = LOWER_Y_HASH;
					in_vowel_block = false;
					}
				else if (in_vowel_block &&
					text[i] == UPPER_Y)
					{
					text[i] = UPPER_Y_HASH;
					in_vowel_block = false;
					}
				else if (string_util::is_one_of(text[i], vowel_string) )
					{
					in_vowel_block = true;
					}
				//we are on a consonant
				else
					{
					in_vowel_block = false;
					}
				}
			}

		//----------------------------------------------------------
		inline void unhash_y(std::basic_string<Tchar_type>& text)
			{
			string_util::replace_all(text, LOWER_Y_HASH, LOWER_Y);
			string_util::replace_all(text, UPPER_Y_HASH, UPPER_Y);
			}

		//----------------------------------------------------------
		void hash_italian_ui(std::basic_string<Tchar_type>& text,
					const Tchar_type* vowel_string)
			{
			//need at least 2 letters for hashing
			if (text.length() < 2)
				{ return; }

			bool in_vowel_block = string_util::is_one_of(text[0], vowel_string);

			size_t i = 1;
			for (i = 1; i < text.length()-1; ++i)
				{
				if (in_vowel_block &&
					string_util::is_one_of(text[i], vowel_string) &&
					string_util::is_one_of(text[i+1], vowel_string) )
					{
					if (text[i] == LOWER_I )
						{
						text[i] = LOWER_I_HASH;
						}
					else if (text[i] == UPPER_I )
						{
						text[i] = UPPER_I_HASH;
						}
					else if (text[i] == LOWER_U)
						{
						text[i] = LOWER_U_HASH;
						}
					else if (text[i] == UPPER_U)
						{
						text[i] = UPPER_U_HASH;
						}
					}
				else if (string_util::is_one_of(text[i], vowel_string) )
					{
					/*u after q should be encrypted and not be
					treated as a vowel*/
					if (text[i] == LOWER_U &&
						is_either(text[i-1], LOWER_Q, UPPER_Q) )
						{
						text[i] = LOWER_U_HASH;
						in_vowel_block = false;
						}
					else if (text[i] == UPPER_U &&
						is_either(text[i-1], LOWER_Q, UPPER_Q) )
						{
						text[i] = UPPER_U_HASH;
						in_vowel_block = false;
						}
					else
						{
						in_vowel_block = true;
						}
					}
				//we are on a consonant
				else
					{
					in_vowel_block = false;
					}
				}
			//verify the last letter
			if (text[i] == LOWER_U &&
				is_either(text[i-1], LOWER_Q, UPPER_Q) )
				{
				text[i] = LOWER_U_HASH;
				}
			else if (text[i] == UPPER_U &&
				is_either(text[i-1], LOWER_Q, UPPER_Q) )
				{
				text[i] = UPPER_U_HASH;
				}
			}

		//----------------------------------------------------------
		inline void unhash_italian_ui(std::basic_string<Tchar_type>& text)
			{
			string_util::replace_all(text, LOWER_I_HASH, LOWER_I);
			string_util::replace_all(text, UPPER_I_HASH, UPPER_I);
			string_util::replace_all(text, LOWER_U_HASH, LOWER_U);
			string_util::replace_all(text, UPPER_U_HASH, UPPER_U);
			}

		//----------------------------------------------------------
		void remove_dutch_umlauts(std::basic_string<Tchar_type>& text)
			{
			for (size_t i = 0; i < text.length(); ++i)
				{
				if (text[i] == assign_character(Tchar_type, 0xC4))
					{
					text[i] = UPPER_A;
					}
				else if (text[i] == assign_character(Tchar_type, 0xCB))
					{
					text[i] = UPPER_E;
					}
				else if (text[i] == assign_character(Tchar_type, 0xCF))
					{
					text[i] = UPPER_I;
					}
				else if (text[i] == assign_character(Tchar_type, 0xD6))
					{
					text[i] = UPPER_O;
					}
				else if (text[i] == assign_character(Tchar_type, 0xDC))
					{
					text[i] = UPPER_U;
					}
				else if (text[i] == assign_character(Tchar_type, 0xE4) )
					{
					text[i] = LOWER_A;
					}
				else if (text[i] == assign_character(Tchar_type, 0xEB))
					{
					text[i] = LOWER_E;
					}
				else if (text[i] == assign_character(Tchar_type, 0xEF))
					{
					text[i] = LOWER_I;
					}
				else if (text[i] == assign_character(Tchar_type, 0xF6))
					{
					text[i] = LOWER_O;
					}
				else if (text[i] == assign_character(Tchar_type, 0xFC))
					{
					text[i] = LOWER_U;
					}
				}
			}

		//----------------------------------------------------------
		void remove_dutch_acutes(std::basic_string<Tchar_type>& text)
			{
			for (size_t i = 0; i < text.length(); ++i)
				{
				if (text[i] == assign_character(Tchar_type, 0xC1))
					{
					text[i] = UPPER_A;
					}
				else if (text[i] == assign_character(Tchar_type, 0xC9))
					{
					text[i] = UPPER_E;
					}
				else if (text[i] == assign_character(Tchar_type, 0xCD))
					{
					text[i] = UPPER_I;
					}
				else if (text[i] == assign_character(Tchar_type, 0xD3))
					{
					text[i] = UPPER_O;
					}
				else if (text[i] == assign_character(Tchar_type, 0xDA))
					{
					text[i] = UPPER_U;
					}
				else if (text[i] == assign_character(Tchar_type, 0xE1))
					{
					text[i] = LOWER_A;
					}
				else if (text[i] == assign_character(Tchar_type, 0xE9))
					{
					text[i] = LOWER_E;
					}
				else if (text[i] == assign_character(Tchar_type, 0xED))
					{
					text[i] = LOWER_I;
					}
				else if (text[i] == assign_character(Tchar_type, 0xF3))
					{
					text[i] = LOWER_O;
					}
				else if (text[i] == assign_character(Tchar_type, 0xFA))
					{
					text[i] = LOWER_U;
					}
				}
			}

		//----------------------------------------------------------
		void remove_spanish_acutes(std::basic_string<Tchar_type>& text)
			{
			for (size_t i = 0; i < text.length(); ++i)
				{
				if (text[i] == assign_character(Tchar_type, 0xC1))
					{
					text[i] = UPPER_A;
					}
				else if (text[i] == assign_character(Tchar_type, 0xC9))
					{
					text[i] = UPPER_E;
					}
				else if (text[i] == assign_character(Tchar_type, 0xCD))
					{
					text[i] = UPPER_I;
					}
				else if (text[i] == assign_character(Tchar_type, 0xD3))
					{
					text[i] = UPPER_O;
					}
				else if (text[i] == assign_character(Tchar_type, 0xDA))
					{
					text[i] = UPPER_U;
					}
				else if (text[i] == assign_character(Tchar_type, 0xE1))
					{
					text[i] = LOWER_A;
					}
				else if (text[i] == assign_character(Tchar_type, 0xE9))
					{
					text[i] = LOWER_E;
					}
				else if (text[i] == assign_character(Tchar_type, 0xED))
					{
					text[i] = LOWER_I;
					}
				else if (text[i] == assign_character(Tchar_type, 0xF3))
					{
					text[i] = LOWER_O;
					}
				else if (text[i] == assign_character(Tchar_type, 0xFA))
					{
					text[i] = LOWER_U;
					}
				}
			}

		size_t m_r1;
		size_t m_r2;
		//only used for romance/russian languages
		size_t m_rv;
		};

	//------------------------------------------------------
	/*A non-operational stemmer that is used in place of regular stemmers when
	you don't want the system to actually stem anything.*/
	class no_op_stem : public stem<xchar>
		{
	public:
		no_op_stem() : stem<xchar>()
			{}
		//---------------------------------------------
		inline void operator()(std::basic_string<xchar>& /*text*/)
			{}
		};
	}

#endif //__STEM_H__
