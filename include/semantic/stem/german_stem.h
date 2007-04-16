/***************************************************************************
                          german_stem.h  -  description
                             -------------------
    begin                : Sat May 18 2004
    copyright            : (C) 2004 by Blake Madden
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the BSD License.                                *
 *                                                                         *
 ***************************************************************************/

#ifndef __GERMAN_STEM_H__
#define __GERMAN_STEM_H__

#include "semantic/stem/stemming.h"

namespace stemming
	{
	/**German includes the following accented forms,
		-ä ö ü 
	and a special letter, ß, equivalent to double s. 

	The following letters are vowels:
		-a e i o u y ä ö ü*.*/
	//------------------------------------------------------
	class german_stem : stem<xchar>
		{
	public:
		//---------------------------------------------
		/**@param text: string to stem
		   @param contract_transliterated_umlauts: apply variant algorithm
		   that contracts "ä" to "ae", ect...*/
		void operator()(std::basic_string<xchar>& text,
						bool contract_transliterated_umlauts = false)
			{
			if (text.length() < 2)
				{
				remove_german_umlauts(text);
				return;
				}

			//reset internal data
			m_r1 = m_r2 = m_rv = 0;

			trim_western_punctuation(text);

			hash_german_yu(text, GERMAN_VOWELS);
			//change 'ß' to "ss"
			string_util::replace_all(text, _XTEXT("ß"), _XTEXT("ss"));
			//German variant addition
			if (contract_transliterated_umlauts)
				{
				string_util::replace_all(text, _XTEXT("ae"), _XTEXT("ä"));
				string_util::replace_all(text, _XTEXT("oe"), _XTEXT("ö"));
				//ue to ü, if not in front of 'q'
				size_t start = 1;
				while (start != std::basic_string<xchar>::npos)
					{
					start = text.find(_XTEXT("ue"), start);
					if (start == std::basic_string<xchar>::npos ||
						is_either(text[start-1], LOWER_Q, UPPER_Q) )
						{
						break;
						}
					text.replace(start, 2, _XTEXT("ü"));
					}
				}

			find_r1(text, GERMAN_VOWELS);
			if (m_r1 == text.length() )
				{
				remove_german_umlauts(text);
				unhash_german_yu(text);
				return;
				}
			find_r2(text, GERMAN_VOWELS);
			//R1 must have at least 3 characters in front of it
			if (m_r1 < 3)
				{
				m_r1 = 3;	
				}

			step_1(text);
			step_2(text);
			step_3(text);

			//unhash special 'u' and 'y' back, and remove the umlaut accent from a, o and u. 
			remove_german_umlauts(text);
			unhash_german_yu(text);
			}
	private:
		/**Search for the longest among the following suffixes, 
			-#e em en ern er es
			-#s (preceded by a valid s-ending) 
		and delete if in R1. (Of course the letter of the valid s-ending is not necessarily in R1) 

		(For example, äckern -> äck, ackers -> acker, armes -> arm) */
		//---------------------------------------------
		void step_1(std::basic_string<xchar>& text)
			{
			if (delete_if_is_in_r1(text,/*ern*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_N, UPPER_N) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*er*/LOWER_E, UPPER_E, LOWER_R, UPPER_R) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*es*/LOWER_E, UPPER_E, LOWER_S, UPPER_S) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*en*/LOWER_E, UPPER_E, LOWER_N, UPPER_N) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*em*/LOWER_E, UPPER_E, LOWER_M, UPPER_M) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text, LOWER_E, UPPER_E) )
				{
				return;
				}
			///Define a valid s-ending as one of b, d, f, g, h, k, l, m, n, r or t.
			else if (is_suffix_in_r1(text, LOWER_S, UPPER_S) )
				{
				if (string_util::is_one_of(text[text.length()-2], _XTEXT("bdfghklmnrtBDFGHKLMNRT")) )
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					}
				return;
				}
			}
		/**Search for the longest among the following suffixes,
			-#en er est
			-#st (preceded by a valid st-ending, itself preceded by at least 3 letters) 
		and delete if in R1. 

		(For example, derbsten -> derbst by step 1, and derbst -> derb by step 2,
		because b is a valid st-ending, and is preceded by just 3 letters)*/
		//---------------------------------------------
		void step_2(std::basic_string<xchar>& text)
			{
			if (delete_if_is_in_r1(text,/*est*/LOWER_E, UPPER_E, LOWER_S, UPPER_S, LOWER_T, UPPER_T) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*er*/LOWER_E, UPPER_E, LOWER_R, UPPER_R) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*en*/LOWER_E, UPPER_E, LOWER_N, UPPER_N) )
				{
				return;
				}
			///Define a valid st-ending as the same list, excluding letter r.
			else if (text.length() >= 6 &&
					is_suffix_in_r1(text,/*st*/LOWER_S, UPPER_S, LOWER_T, UPPER_T) )
				{
				if (string_util::is_one_of(text[text.length()-3], _XTEXT("bdfghklmntBDFGHKLMNT")) )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				return;
				}
			}
		/**Search for the longest among the following suffixes, and perform the action indicated. 
	
			-end ung 
				-delete if in R2 
				-if preceded by ig, delete if in R2 and not preceded by e 

			-ig ik isch 
				-delete if in R2 and not preceded by e 

			-lich heit 
				-delete if in R2 
				-if preceded by er or en, delete if in R1 

			-keit 
				-delete if in R2 
				-if preceded by lich or ig, delete if in R2*/
		//---------------------------------------------
		void step_3(std::basic_string<xchar>& text) 
			{
			if (delete_if_is_in_r2(text,/*heit*/LOWER_H, UPPER_H, LOWER_E, UPPER_E, LOWER_I, UPPER_I, LOWER_T, UPPER_T) ||
				delete_if_is_in_r2(text,/*lich*/LOWER_L, UPPER_L, LOWER_I, UPPER_I, LOWER_C, UPPER_C, LOWER_H, UPPER_H) )
				{
				if (delete_if_is_in_r1(text,/*er*/LOWER_E, UPPER_E, LOWER_R, UPPER_R) ||
					delete_if_is_in_r1(text,/*en*/LOWER_E, UPPER_E, LOWER_N, UPPER_N) )
					{
					return;
					}
				return;
				}
			else if (delete_if_is_in_r2(text,/*keit*/LOWER_K, UPPER_K, LOWER_E, UPPER_E, LOWER_I, UPPER_I, LOWER_T, UPPER_T) )
				{
				if (delete_if_is_in_r2(text,/*lich*/LOWER_L, UPPER_L, LOWER_I, UPPER_I, LOWER_C, UPPER_C, LOWER_H, UPPER_H) ||
					delete_if_is_in_r2(text,/*ig*/LOWER_I, UPPER_I, LOWER_G, UPPER_G) )
					{
					return;
					}
				return;
				}
			else if (is_suffix(text,/*isch*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_C, UPPER_C, LOWER_H, UPPER_H) )
				{
				if (m_r2 <= static_cast<int>(text.length()-4) &&
					is_neither(text[text.length()-5], LOWER_E, UPPER_E) )
					{
					text.erase(text.end()-4, text.end() );
					update_r_sections(text);
					}
				}
			else if (delete_if_is_in_r2(text,/*end*/LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_D, UPPER_D) )
				{
				if (text.length() >= 3 &&
					is_suffix_in_r2(text,/*ig*/LOWER_I, UPPER_I, LOWER_G, UPPER_G)  &&
					is_neither(text[text.length()-3], LOWER_E, UPPER_E) )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				return;
				}
			else if (delete_if_is_in_r2(text,/*ung*/LOWER_U, UPPER_U, LOWER_N, UPPER_N, LOWER_G, UPPER_G) )
				{
				if (text.length() >= 3 &&
					is_suffix_in_r2(text,/*ig*/LOWER_I, UPPER_I, LOWER_G, UPPER_G)  &&
					is_neither(text[text.length()-3], LOWER_E, UPPER_E) )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				return;
				}
			else if (is_suffix(text,/*ig*/LOWER_I, UPPER_I, LOWER_G, UPPER_G) )
				{
				if (text.length() >= 3 &&
					m_r2 <= text.length()-2 &&
					is_neither(text[text.length()-3], LOWER_E, UPPER_E) )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				}
			else if (is_suffix(text,/*ik*/LOWER_I, UPPER_I, LOWER_K, UPPER_K) )
				{
				if (text.length() >= 3 &&
					m_r2 <= text.length()-2 &&
					is_neither(text[text.length()-3], LOWER_E, UPPER_E) )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				}
			}
		};
	}

#endif //__GERMAN_STEM_H__
