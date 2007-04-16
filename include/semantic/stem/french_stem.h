/***************************************************************************
                          french_stem.h  -  description
                             -------------------
    begin                : Sat May 25 2004
    copyright            : (C) 2004 by Blake Madden
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the BSD License.                                *
 *                                                                         *
 ***************************************************************************/

#ifndef __FRENCH_STEM_H__
#define __FRENCH_STEM_H__

#include "semantic/stem/stemming.h"

namespace stemming
	{
	/**Letters in French include the following accented forms:

		-â à ç ë é ê è ï î ô û ù 

	The following letters are vowels:
	
		-a e i o u y â à ë é ê è ï î ô û ù*/
	//------------------------------------------------------
	class french_stem : stem<xchar>
		{
	public:
		french_stem() : m_step_1_successful(false) {}
		//---------------------------------------------
		///@param text: string to stem
		void operator()(std::basic_string<xchar>& text)
			{
			if (text.length() < 2)
				{
				return;
				}

			//reset internal data
			m_step_1_successful = false;
			m_r1 = m_r2 = m_rv =0;

			trim_western_punctuation(text);
			hash_french_yui(text, FRENCH_VOWELS);

			find_r1(text, FRENCH_VOWELS);
			find_r2(text, FRENCH_VOWELS);
			find_french_rv(text, FRENCH_VOWELS);

			size_t length = text.length();
			step_1(text);
			if (!m_step_1_successful)
				{
				step_2a(text);
				}
			if (length != text.length() )
				{
				step_3(text);
				}
			else
				{
				step_4(text);
				}
			step_5(text);
			step_6(text);

			unhash_french_yui(text);
			}
	private:
		bool ic_to_iqu(std::basic_string<xchar>& text)
			{
			if (is_suffix(text,/*ic*/LOWER_I, UPPER_I, LOWER_C, UPPER_C) )
				{
				if (m_r2 <= text.length()-2)
					{
					text.erase(text.length()-2);
					update_r_sections(text);
					return true;
					}
				else
					{
					text.erase(text.length()-2);
					text += _XTEXT("iq");
					text += LOWER_U_HASH;
					//need to search for r2 again because the 'iq' added here may change that
					find_r2(text, FRENCH_VOWELS);
			        find_french_rv(text, FRENCH_VOWELS);
					return true;
					}
				}
			return false;
			}
		/**Search for the longest among the following suffixes, and perform the action indicated. 

			-ance iqUe isme able iste eux ances iqUes ismes ables istes 
				-delete if in R2 

			-atrice ateur ation atrices ateurs ations 
				-delete if in R2 
				-if preceded by ic, delete if in R2, else replace by iqU 

			-logie logies 
				-replace with log if in R2 

			-usion ution usions utions 
				-replace with u if in R2 

			-ence ences 
				-replace with ent if in R2 

			-ement ements 
				-delete if in RV 
				-if preceded by iv, delete if in R2 (and if further preceded by at, delete if in R2), otherwise, 
				-if preceded by eus, delete if in R2, else replace by eux if in R1, otherwise, 
				-if preceded by abl or iqU, delete if in R2, otherwise, 
				-if preceded by ièr or Ièr, replace by i if in RV 

			-ité ités 
				-delete if in R2 
				-if preceded by abil, delete if in R2, else replace by abl, otherwise, 
				-if preceded by ic, delete if in R2, else replace by iqU, otherwise, 
				-if preceded by iv, delete if in R2 

			-if ive ifs ives 
				-delete if in R2 
				-if preceded by at, delete if in R2 (and if further preceded by ic, delete if in R2, else replace by iqU) 

			-eaux 
				-replace with eau 

			-aux 
				-replace with al if in R1 

			-euse euses 
				-delete if in R2, else replace by eux if in R1 

			-issement issements 
				-delete if in R1 and preceded by a non-vowel 

			-amment 
				-replace with ant if in RV 

			-emment 
				-replace with ent if in RV 

			-ment ments 
				-delete if preceded by a vowel in RV*/
		//---------------------------------------------
		void step_1(std::basic_string<xchar>& text)
			{
			size_t length = text.length();
			if (is_suffix(text,/*issements*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_M, UPPER_M, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_S, UPPER_S) )
				{
				if (m_r1 <= static_cast<unsigned int>(text.length()-9) &&
					!string_util::is_one_of(text[text.length()-10], FRENCH_VOWELS) )
					{
					text.erase(text.end()-9, text.end() );
					m_step_1_successful = true;
					}
				return;
				}
			else if (is_suffix(text,/*issement*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_M, UPPER_M, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_T, UPPER_T) )
				{
				if (m_r1 <= static_cast<unsigned int>(text.length()-8) &&
					!string_util::is_one_of(text[text.length()-9], FRENCH_VOWELS) )
					{
					text.erase(text.end()-8, text.end() );
					m_step_1_successful = true;
					}
				return;
				}
			//7
			else if (delete_if_is_in_r2(text,/*atrices*/LOWER_A, UPPER_A, LOWER_T, UPPER_T, LOWER_R, UPPER_R, LOWER_I, UPPER_I, LOWER_C, UPPER_C, LOWER_E, UPPER_E, LOWER_S, UPPER_S, false) )
				{
				if (length != text.length() )
					{
					ic_to_iqu(text);
					m_step_1_successful = true;
					}
				return;
				}
			//6
			else if (is_suffix(text,/*amment*/LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_M, UPPER_M, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_T, UPPER_T) )
				{
				if (m_rv <= static_cast<unsigned int>(text.length()-6) )
					{
					text.replace(text.end()-5, text.end(), _XTEXT("nt"));
					update_r_sections(text);
					}
				return;
				}
			else if (is_suffix(text,/*emment*/LOWER_E, UPPER_E, LOWER_M, UPPER_M, LOWER_M, UPPER_M, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_T, UPPER_T) )
				{
				if (m_rv <= static_cast<unsigned int>(text.length()-6) )
					{
					text.replace(text.end()-5, text.end(), _XTEXT("nt"));
					update_r_sections(text);
					}
				return;
				}
			else if (is_suffix(text,/*logies*/LOWER_L, UPPER_L, LOWER_O, UPPER_O, LOWER_G, UPPER_G, LOWER_I, UPPER_I, LOWER_E, UPPER_E, LOWER_S, UPPER_S) )
				{
				if (m_r2 <= static_cast<unsigned int>(text.length()-6) )
					{
					text.erase(text.end()-3, text.end() );
					update_r_sections(text);
					m_step_1_successful = true;
					}
				return;
				}
			else if (delete_if_is_in_r2(text,/*atrice*/LOWER_A, UPPER_A, LOWER_T, UPPER_T, LOWER_R, UPPER_R, LOWER_I, UPPER_I, LOWER_C, UPPER_C, LOWER_E, UPPER_E, false) ||
					delete_if_is_in_r2(text,/*ateurs*/LOWER_A, UPPER_A, LOWER_T, UPPER_T, LOWER_E, UPPER_E, LOWER_U, UPPER_U, LOWER_R, UPPER_R, LOWER_S, UPPER_S, false) ||
					delete_if_is_in_r2(text,/*ations*/LOWER_A, UPPER_A, LOWER_T, UPPER_T, LOWER_I, UPPER_I, LOWER_O, UPPER_O, LOWER_N, UPPER_N, LOWER_S, UPPER_S, false) )
				{				
				if (length != text.length() )
					{
					ic_to_iqu(text);
					m_step_1_successful = true;
					}
				return;
				}
			else if (is_suffix(text,/*usions*/LOWER_U, UPPER_U, LOWER_S, UPPER_S, LOWER_I, UPPER_I, LOWER_O, UPPER_O, LOWER_N, UPPER_N, LOWER_S, UPPER_S) ||
					is_suffix(text,/*utions*/LOWER_U, UPPER_U, LOWER_T, UPPER_T, LOWER_I, UPPER_I, LOWER_O, UPPER_O, LOWER_N, UPPER_N, LOWER_S, UPPER_S) )
				{
				if (m_r2 <= static_cast<unsigned int>(text.length()-6) )
					{
					text.erase(text.end()-5, text.end() );
					update_r_sections(text);
					m_step_1_successful = true;
					}
				return;
				}
			else if (delete_if_is_in_rv(text,/*ements*/LOWER_E, UPPER_E, LOWER_M, UPPER_M, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_S, UPPER_S, false) )
				{
				if (delete_if_is_in_r2(text,/*iv*/LOWER_I, UPPER_I, LOWER_V, UPPER_V) )
					{
					delete_if_is_in_r2(text,/*at*/LOWER_A, UPPER_A, LOWER_T, UPPER_T);
					}
				else if (is_suffix(text,/*eus*/LOWER_E, UPPER_E, LOWER_U, UPPER_U, LOWER_S, UPPER_S) )
					{
					if (m_r2 <= text.length()-3)
						{
						text.erase(text.length()-3);
						update_r_sections(text);
						}
					else if (m_r1 <= text.length()-3)
						{
						text[text.length()-1] = LOWER_X;
						}
					}
				else if (delete_if_is_in_r2(text,/*abl*/LOWER_A, UPPER_A, LOWER_B, UPPER_B, LOWER_L, UPPER_L) )
					{
					//NOOP
					}
				else if (text.length() >= 3 &&
					(text[text.length()-3] == LOWER_I || text[text.length()-3] == UPPER_I) &&
					(text[text.length()-2] == LOWER_Q || text[text.length()-2] == UPPER_Q) &&
					is_either(text[text.length()-1], LOWER_U_HASH, UPPER_U_HASH) )
					{
					if (m_r2 <= text.length()-3)
						{
						text.erase(text.end()-3, text.end() );
						update_r_sections(text);
						}
					}
				else if (is_suffix_in_r2(text,/*ièr*/LOWER_I, UPPER_I, 'è', 'È', LOWER_R, UPPER_R) )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				else if (m_r2 <= static_cast<int>(text.length()-3) &&
					(text[text.length()-2] == 'è' || text[text.length()-2] == 'È') &&
					(text[text.length()-1] == LOWER_R || text[text.length()-1] == UPPER_R) &&
					is_either(text[text.length()-3], LOWER_I_HASH, UPPER_I_HASH) )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				if (length != text.length() )
					{
					m_step_1_successful = true;
					}
				return;
				}
			//5
			else if (delete_if_is_in_r2(text,/*ateur*/LOWER_A, UPPER_A, LOWER_T, UPPER_T, LOWER_E, UPPER_E, LOWER_U, UPPER_U, LOWER_R, UPPER_R, false) ||
					delete_if_is_in_r2(text,/*ation*/LOWER_A, UPPER_A, LOWER_T, UPPER_T, LOWER_I, UPPER_I, LOWER_O, UPPER_O, LOWER_N, UPPER_N, false) )
				{
				if (length != text.length() )
					{
					ic_to_iqu(text);
					m_step_1_successful = true;
					}
				return;
				}
			else if (is_suffix(text,/*usion*/LOWER_U, UPPER_U, LOWER_S, UPPER_S, LOWER_I, UPPER_I, LOWER_O, UPPER_O, LOWER_N, UPPER_N) ||
					is_suffix(text,/*ution*/LOWER_U, UPPER_U, LOWER_T, UPPER_T, LOWER_I, UPPER_I, LOWER_O, UPPER_O, LOWER_N, UPPER_N) )
				{
				if (m_r2 <= static_cast<unsigned int>(text.length()-5) )
					{
					text.erase(text.end()-4, text.end() );
					update_r_sections(text);
					m_step_1_successful = true;
					}
				return;
				}
			else if (is_suffix(text,/*ences*/LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_C, UPPER_C, LOWER_E, UPPER_E, LOWER_S, UPPER_S) )
				{
				if (m_r2 <= static_cast<unsigned int>(text.length()-5) )
					{
					text.replace(text.end()-3, text.end(), _XTEXT("t"));
					update_r_sections(text);
					m_step_1_successful = true;
					}
				return;
				}
			else if (delete_if_is_in_r2(text,/*ables*/LOWER_A, UPPER_A, LOWER_B, UPPER_B, LOWER_L, UPPER_L, LOWER_E, UPPER_E, LOWER_S, UPPER_S, false) ||
					delete_if_is_in_r2(text,/*istes*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_T, UPPER_T, LOWER_E, UPPER_E, LOWER_S, UPPER_S, false) ||
					delete_if_is_in_r2(text,/*ismes*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_M, UPPER_M, LOWER_E, UPPER_E, LOWER_S, UPPER_S, false) ||
					delete_if_is_in_r2(text,/*ances*/LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_C, UPPER_C, LOWER_E, UPPER_E, LOWER_S, UPPER_S, false) )
				{
				if (length != text.length() )
					{
					m_step_1_successful = true;
					}
				return;
				}
			else if (text.length() >= 5 &&
					(text[text.length()-5] == LOWER_I || text[text.length()-5] == UPPER_I) &&
					(text[text.length()-4] == LOWER_Q || text[text.length()-4] == UPPER_Q) &&
					(text[text.length()-2] == LOWER_E || text[text.length()-2] == UPPER_E) &&
					(text[text.length()-1] == LOWER_S || text[text.length()-1] == UPPER_S) &&
					is_either(text[text.length()-3], LOWER_U_HASH, UPPER_U_HASH) )
					{
					if (m_r2 <= text.length()-5)
						{
						text.erase(text.end()-5, text.end() );
						update_r_sections(text);
						}
					if (length != text.length() )
						{
						m_step_1_successful = true;
						}
					return;
					}
			else if (is_suffix(text,/*logie*/LOWER_L, UPPER_L, LOWER_O, UPPER_O, LOWER_G, UPPER_G, LOWER_I, UPPER_I, LOWER_E, UPPER_E) )
				{
				if (m_r2 <= static_cast<unsigned int>(text.length()-5) )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					m_step_1_successful = true;
					}
				return;
				}
			else if (delete_if_is_in_rv(text,/*ement*/LOWER_E, UPPER_E, LOWER_M, UPPER_M, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_T, UPPER_T, false) )
				{
				if (delete_if_is_in_r2(text,/*iv*/LOWER_I, UPPER_I, LOWER_V, UPPER_V, false) )
					{
					delete_if_is_in_r2(text, /*at*/LOWER_A, UPPER_A, LOWER_T, UPPER_T);
					}
				else if (is_suffix(text,/*eus*/LOWER_E, UPPER_E, LOWER_U, UPPER_U, LOWER_S, UPPER_S) )
					{
					if (m_r2 <= text.length()-3)
						{
						text.erase(text.length()-3);
						update_r_sections(text);
						}
					else if (m_r1 <= text.length()-3)
						{
						text[text.length()-1] = LOWER_X;
						}
					}
				else if (delete_if_is_in_r2(text,/*abl*/LOWER_A, UPPER_A, LOWER_B, UPPER_B, LOWER_L, UPPER_L) )
					{
					//NOOP
					}
				else if (text.length() >= 3 &&
					(text[text.length()-3] == LOWER_I || text[text.length()-3] == UPPER_I) &&
					(text[text.length()-2] == LOWER_Q || text[text.length()-2] == UPPER_Q) &&
					is_either(text[text.length()-1], LOWER_U_HASH, UPPER_U_HASH) )
					{
					if (m_r2 <= text.length()-3)
						{
						text.erase(text.end()-3, text.end() );
						update_r_sections(text);
						}
					}
				else if (is_suffix_in_rv(text,/*ièr*/LOWER_I, UPPER_I, 'è', 'È', LOWER_R, UPPER_R) )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				else if (m_rv <= static_cast<int>(text.length()-3) &&
					(text[text.length()-2] == 'è' || text[text.length()-2] == 'È') &&
					(text[text.length()-1] == LOWER_R || text[text.length()-1] == UPPER_R) &&
					is_either(text[text.length()-3], LOWER_I_HASH, UPPER_I_HASH) )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				if (length != text.length() )
					{
					m_step_1_successful = true;
					}
				}
			else if (is_suffix(text,/*ments*/LOWER_M, UPPER_M, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_S, UPPER_S) )
				{
				//the proceeding vowel must also be n RV
				if (m_rv <= text.length()-6 &&
					string_util::is_one_of(text[text.length()-6], FRENCH_VOWELS) )
					{
					text.erase(text.end()-5, text.end() );
					update_r_sections(text);	
					}
				return;
				}
			else if (is_suffix(text,/*euses*/LOWER_E, UPPER_E, LOWER_U, UPPER_U, LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_S, UPPER_S) )
				{
				if (m_r2 <= text.length()-5)
					{
					text.erase(text.end()-5, text.end() );
					update_r_sections(text);
					}
				else if (m_r1 <= text.length()-5)
					{
					text.replace(text.end()-3, text.end(), _XTEXT("x"));
					update_r_sections(text);
					}
				m_step_1_successful = true;
				}
			//4
			else if (is_suffix(text,/*euse*/LOWER_E, UPPER_E, LOWER_U, UPPER_U, LOWER_S, UPPER_S, LOWER_E, UPPER_E) )
				{
				if (m_r2 <= text.length()-4)
					{
					text.erase(text.end()-4, text.end() );
					update_r_sections(text);
					}
				else if (m_r1 <= text.length()-4)
					{
					text.replace(text.end()-2, text.end(), _XTEXT("x"));
					update_r_sections(text);
					}
				m_step_1_successful = true;
				}
			else if (is_suffix(text,/*ment*/LOWER_M, UPPER_M, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_T, UPPER_T) )
				{
				//the proceeding vowel must also be n RV
				if (m_rv <= text.length()-5 &&
					string_util::is_one_of(text[text.length()-5], FRENCH_VOWELS) )
					{
					text.erase(text.end()-4, text.end() );
					update_r_sections(text);	
					}
				return;
				}
			else if (is_suffix(text,/*ence*/LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_C, UPPER_C, LOWER_E, UPPER_E) )
				{
				if (m_r2 <= static_cast<unsigned int>(text.length()-4) )
					{
					text.replace(text.end()-2, text.end(), _XTEXT("t"));
					update_r_sections(text);
					m_step_1_successful = true;
					}
				return;
				}
			else if (delete_if_is_in_r2(text,/*ance*/LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_C, UPPER_C, LOWER_E, UPPER_E, false) ||
					delete_if_is_in_r2(text,/*isme*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_M, UPPER_M, LOWER_E, UPPER_E, false) ||
					delete_if_is_in_r2(text,/*able*/LOWER_A, UPPER_A, LOWER_B, UPPER_B, LOWER_L, UPPER_L, LOWER_E, UPPER_E, false) ||
					delete_if_is_in_r2(text,/*iste*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_T, UPPER_T, LOWER_E, UPPER_E, false) )
				{
				if (length != text.length() )
					{
					m_step_1_successful = true;
					}
				return;
				}
			else if (text.length() >= 4 &&
					(text[text.length()-4] == LOWER_I || text[text.length()-4] == UPPER_I) &&
					(text[text.length()-3] == LOWER_Q || text[text.length()-3] == UPPER_Q) &&
					(text[text.length()-1] == LOWER_E || text[text.length()-1] == UPPER_E) &&
					is_either(text[text.length()-2], LOWER_U_HASH, UPPER_U_HASH) )
					{
					if (m_r2 <= text.length()-4)
						{
						text.erase(text.end()-4, text.end() );
						update_r_sections(text);
						}
					if (length != text.length() )
						{
						m_step_1_successful = true;
						}
					return;
					}
			else if (is_suffix(text,/*eaux*/LOWER_E, UPPER_E, LOWER_A, UPPER_A, LOWER_U, UPPER_U, LOWER_X, UPPER_X) )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				m_step_1_successful = true;
				return;
				}
			else if (delete_if_is_in_r2(text,/*ités*/LOWER_I, UPPER_I, LOWER_T, UPPER_T, LOWER_E_ACUTE, UPPER_E_ACUTE, LOWER_S, UPPER_S, false) )
				{
				if (is_suffix(text,/*abil*/LOWER_A, UPPER_A, LOWER_B, UPPER_B, LOWER_I, UPPER_I, LOWER_L, UPPER_L) )
					{
					if (m_r2 <= text.length()-4)
						{
						text.erase(text.length()-4);
						update_r_sections(text);
						}
					else
						{
						text.replace(text.end()-2, text.end(), _XTEXT("l"));
						}
					}
				else if (is_suffix(text,/*ic*/LOWER_I, UPPER_I, LOWER_C, UPPER_C) )
					{
					if (m_r2 <= text.length()-2)
						{
						text.erase(text.length()-2);
						update_r_sections(text);
						}
					else
						{
						text.erase(text.length()-2);
						text += _XTEXT("iq");
						text += LOWER_U_HASH;
						//need to search for r2 again because the 'iq' added here may change that
					    find_r2(text, FRENCH_VOWELS);
			            find_french_rv(text, FRENCH_VOWELS);
						}
					}
				else
					{
					delete_if_is_in_r2(text,/*iv*/LOWER_I, UPPER_I, LOWER_V, UPPER_V);
					}
				if (length != text.length() )
					{
					m_step_1_successful = true;
					}
				return;
				}
			else if (delete_if_is_in_r2(text,/*ives*/LOWER_I, UPPER_I, LOWER_V, UPPER_V, LOWER_E, UPPER_E, LOWER_S, UPPER_S, false) )
				{
				if (delete_if_is_in_r2(text,/*at*/LOWER_A, UPPER_A, LOWER_T, UPPER_T) )
					{
					ic_to_iqu(text);
					}
				if (length != text.length() )
					{
					m_step_1_successful = true;
					}
				return;
				}
			//3
			else if (delete_if_is_in_r2(text,/*ité*/LOWER_I, UPPER_I, LOWER_T, UPPER_T, LOWER_E_ACUTE, UPPER_E_ACUTE, false) )
				{
				if (is_suffix(text,/*abil*/LOWER_A, UPPER_A, LOWER_B, UPPER_B, LOWER_I, UPPER_I, LOWER_L, UPPER_L) )
					{
					if (m_r2 <= text.length()-4)
						{
						text.erase(text.length()-4);
						update_r_sections(text);
						}
					else
						{
						text.replace(text.end()-2, text.end(), _XTEXT("l"));
						}
					}
				else if (is_suffix(text,/*ic*/LOWER_I, UPPER_I, LOWER_C, UPPER_C) )
					{
					if (m_r2 <= text.length()-2)
						{
						text.erase(text.length()-2);
						update_r_sections(text);
						}
					else
						{
						text.erase(text.length()-2);
						text += _XTEXT("iq");
						text += LOWER_U_HASH;
						//need to search for r2 again because the 'iq' added here may change that
					    find_r2(text, FRENCH_VOWELS);
			            find_french_rv(text, FRENCH_VOWELS);
						}
					}
				else
					{
					delete_if_is_in_r2(text,/*iv*/LOWER_I, UPPER_I, LOWER_V, UPPER_V);
					}
				if (length != text.length() )
					{
					m_step_1_successful = true;
					}
				return;
				}
			else if (delete_if_is_in_r2(text,/*eux*/LOWER_E, UPPER_E, LOWER_U, UPPER_U, LOWER_X, UPPER_X, false) )
				{
				if (length != text.length() )
					{
					m_step_1_successful = true;
					}
				return;
				}
			else if (is_suffix(text,/*aux*/LOWER_A, UPPER_A, LOWER_U, UPPER_U, LOWER_X, UPPER_X) )
				{
				if (m_r1 <= static_cast<unsigned int>(text.length()-3) )
					{
					text.replace(text.end()-2, text.end(), _XTEXT("l"));
					update_r_sections(text);
					m_step_1_successful = true;
					}
				return;
				}
			else if (delete_if_is_in_r2(text,/*ive*/LOWER_I, UPPER_I, LOWER_V, UPPER_V, LOWER_E, UPPER_E, false) ||
					delete_if_is_in_r2(text,/*ifs*/LOWER_I, UPPER_I, LOWER_F, UPPER_F, LOWER_S, UPPER_S, false) )
				{
				if (delete_if_is_in_r2(text,/*at*/LOWER_A, UPPER_A, LOWER_T, UPPER_T, false) )
					{
					ic_to_iqu(text);
					}
				if (length != text.length() )
					{
					m_step_1_successful = true;
					}
				return;
				}
			//2
			else if (delete_if_is_in_r2(text,/*if*/LOWER_I, UPPER_I, LOWER_F, UPPER_F, false) )
				{
				if (delete_if_is_in_r2(text,/*at*/LOWER_A, UPPER_A, LOWER_T, UPPER_T, false) )
					{
					ic_to_iqu(text);
					}
				if (length != text.length() )
					{
					m_step_1_successful = true;
					}
				return;
				}
			}
		/**In steps 2a and 2b all tests are confined to the RV region. 

		Do step 2a if either no ending was removed by step 1, or if one of endings amment, emment, ment, ments was found. 

		Search for the longest among the following suffixes and if found, delete if preceded by a non-vowel. 

			-îmes ît îtes i ie ies ir ira irai iraIent irais irait iras irent irez iriez irions irons iront is issaIent
			issais issait issant issante issantes issants isse issent isses issez issiez issions issons it 

		(Note that the non-vowel itself must also be in RV.)*/
		//---------------------------------------------
		void step_2a(std::basic_string<xchar>& text)
			{
			if (m_rv <= static_cast<int>(text.length()-8) &&
				(text[text.length()-8] == LOWER_I || text[text.length()-8] == UPPER_I) &&
				(text[text.length()-7] == LOWER_S || text[text.length()-7] == UPPER_S) &&
				(text[text.length()-6] == LOWER_S || text[text.length()-6] == UPPER_S) &&
				(text[text.length()-5] == LOWER_A || text[text.length()-5] == UPPER_A) &&
				(text[text.length()-3] == LOWER_E || text[text.length()-3] == UPPER_E) &&
				(text[text.length()-2] == LOWER_N || text[text.length()-2] == UPPER_N) &&
				(text[text.length()-1] == LOWER_T || text[text.length()-1] == UPPER_T) &&
				is_either(text[text.length()-4], LOWER_I_HASH, UPPER_I_HASH) )
				{
				if (m_rv <= static_cast<int>(text.length()-7) &&
					!string_util::is_one_of(text[text.length()-9], FRENCH_VOWELS) )
					{
					text.erase(text.end()-8, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*issantes*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_E, UPPER_E, LOWER_S, UPPER_S) )
				{
				if (m_rv <= static_cast<int>(text.length()-9) &&
					!string_util::is_one_of(text[text.length()-9], FRENCH_VOWELS) )
					{
					text.erase(text.end()-8, text.end() );
					return;
					}
				}
			else if (m_rv <= static_cast<int>(text.length()-7) &&
				(text[text.length()-7] == LOWER_I || text[text.length()-7] == UPPER_I) &&
				(text[text.length()-6] == LOWER_R || text[text.length()-6] == UPPER_R) &&
				(text[text.length()-5] == LOWER_A || text[text.length()-5] == UPPER_A) &&
				(text[text.length()-3] == LOWER_E || text[text.length()-3] == UPPER_E) &&
				(text[text.length()-2] == LOWER_N || text[text.length()-2] == UPPER_N) &&
				(text[text.length()-1] == LOWER_T || text[text.length()-1] == UPPER_T) &&
				is_either(text[text.length()-4], LOWER_I_HASH, UPPER_I_HASH) )
				{
				if (m_rv <= static_cast<int>(text.length()-8) &&
					!string_util::is_one_of(text[text.length()-8], FRENCH_VOWELS) )
					{
					text.erase(text.end()-7, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*issante*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_E, UPPER_E) )
				{
				if (m_rv <= static_cast<int>(text.length()-8) &&
					!string_util::is_one_of(text[text.length()-8], FRENCH_VOWELS) )
					{
					text.erase(text.end()-7, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*issants*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_S, UPPER_S) )
				{
				if (m_rv <= static_cast<int>(text.length()-8) &&
					!string_util::is_one_of(text[text.length()-8], FRENCH_VOWELS) )
					{
					text.erase(text.end()-7, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*issions*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_I, UPPER_I, LOWER_O, UPPER_O, LOWER_N, UPPER_N, LOWER_S, UPPER_S) )
				{
				if (m_rv <= static_cast<int>(text.length()-8) &&
					!string_util::is_one_of(text[text.length()-8], FRENCH_VOWELS) )
					{
					text.erase(text.end()-7, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*irions*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_I, UPPER_I, LOWER_O, UPPER_O, LOWER_N, UPPER_N, LOWER_S, UPPER_S) )
				{
				if (m_rv <= static_cast<int>(text.length()-7) &&
					!string_util::is_one_of(text[text.length()-7], FRENCH_VOWELS) )
					{
					text.erase(text.end()-6, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*issais*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_A, UPPER_A, LOWER_I, UPPER_I, LOWER_S, UPPER_S) )
				{
				if (m_rv <= static_cast<int>(text.length()-7) &&
					!string_util::is_one_of(text[text.length()-7], FRENCH_VOWELS) )
					{
					text.erase(text.end()-6, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*issait*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_A, UPPER_A, LOWER_I, UPPER_I, LOWER_T, UPPER_T) )
				{
				if (m_rv <= static_cast<int>(text.length()-7) &&
					!string_util::is_one_of(text[text.length()-7], FRENCH_VOWELS) )
					{
					text.erase(text.end()-6, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*issant*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_T, UPPER_T) )
				{
				if (m_rv <= static_cast<int>(text.length()-7) &&
					!string_util::is_one_of(text[text.length()-7], FRENCH_VOWELS) )
					{
					text.erase(text.end()-6, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*issent*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_T, UPPER_T) )
				{
				if (m_rv <= static_cast<int>(text.length()-7) &&
					!string_util::is_one_of(text[text.length()-7], FRENCH_VOWELS) )
					{
					text.erase(text.end()-6, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*issiez*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_I, UPPER_I, LOWER_E, UPPER_E, LOWER_Z, UPPER_Z) )
				{
				if (m_rv <= static_cast<int>(text.length()-7) &&
					!string_util::is_one_of(text[text.length()-7], FRENCH_VOWELS) )
					{
					text.erase(text.end()-6, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*issons*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_O, UPPER_O, LOWER_N, UPPER_N, LOWER_S, UPPER_S) )
				{
				if (m_rv <= static_cast<int>(text.length()-7) &&
					!string_util::is_one_of(text[text.length()-7], FRENCH_VOWELS) )
					{
					text.erase(text.end()-6, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*irais*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_I, UPPER_I, LOWER_S, UPPER_S) )
				{
				if (m_rv <= static_cast<int>(text.length()-6) &&
					!string_util::is_one_of(text[text.length()-6], FRENCH_VOWELS) )
					{
					text.erase(text.end()-5, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*irait*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_I, UPPER_I, LOWER_T, UPPER_T) )
				{
				if (m_rv <= static_cast<int>(text.length()-6) &&
					!string_util::is_one_of(text[text.length()-6], FRENCH_VOWELS) )
					{
					text.erase(text.end()-5, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*irent*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_T, UPPER_T) )
				{
				if (m_rv <= static_cast<int>(text.length()-6) &&
					!string_util::is_one_of(text[text.length()-6], FRENCH_VOWELS) )
					{
					text.erase(text.end()-5, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*iriez*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_I, UPPER_I, LOWER_E, UPPER_E, LOWER_Z, UPPER_Z) )
				{
				if (m_rv <= static_cast<int>(text.length()-6) &&
					!string_util::is_one_of(text[text.length()-6], FRENCH_VOWELS) )
					{
					text.erase(text.end()-5, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*irons*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_O, UPPER_O, LOWER_N, UPPER_N, LOWER_S, UPPER_S) )
				{
				if (m_rv <= static_cast<int>(text.length()-6) &&
					!string_util::is_one_of(text[text.length()-6], FRENCH_VOWELS) )
					{
					text.erase(text.end()-5, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*iront*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_O, UPPER_O, LOWER_N, UPPER_N, LOWER_T, UPPER_T) )
				{
				if (m_rv <= static_cast<int>(text.length()-6) &&
					!string_util::is_one_of(text[text.length()-6], FRENCH_VOWELS) )
					{
					text.erase(text.end()-5, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*isses*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_S, UPPER_S) )
				{
				if (m_rv <= static_cast<int>(text.length()-6) &&
					!string_util::is_one_of(text[text.length()-6], FRENCH_VOWELS) )
					{
					text.erase(text.end()-5, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*issez*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_Z, UPPER_Z) )
				{
				if (m_rv <= static_cast<int>(text.length()-6)&&
					!string_util::is_one_of(text[text.length()-6], FRENCH_VOWELS) )
					{
					text.erase(text.end()-5, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*îmes*/'î', 'Î', LOWER_M, UPPER_M, LOWER_E, UPPER_E, LOWER_S, UPPER_S) )
				{
				if (m_rv <= static_cast<int>(text.length()-5) &&
					!string_util::is_one_of(text[text.length()-5], FRENCH_VOWELS) )
					{
					text.erase(text.end()-4, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*îtes*/'î', 'Î', LOWER_T, UPPER_T, LOWER_E, UPPER_E, LOWER_S, UPPER_S) )
				{
				if (m_rv <= static_cast<int>(text.length()-5) &&
					!string_util::is_one_of(text[text.length()-5], FRENCH_VOWELS) )
					{
					text.erase(text.end()-4, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*irai*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_I, UPPER_I) )
				{
				if (m_rv <= static_cast<int>(text.length()-5) &&
					!string_util::is_one_of(text[text.length()-5], FRENCH_VOWELS) )
					{
					text.erase(text.end()-4, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*iras*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_S, UPPER_S) )
				{
				if (m_rv <= static_cast<int>(text.length()-5) &&
					!string_util::is_one_of(text[text.length()-5], FRENCH_VOWELS) )
					{
					text.erase(text.end()-4, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*irez*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_Z, UPPER_Z) )
				{
				if (m_rv <= static_cast<int>(text.length()-5) &&
					!string_util::is_one_of(text[text.length()-5], FRENCH_VOWELS) )
					{
					text.erase(text.end()-4, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*isse*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_E, UPPER_E) )
				{
				if (m_rv <= static_cast<int>(text.length()-5) &&
					!string_util::is_one_of(text[text.length()-5], FRENCH_VOWELS) )
					{
					text.erase(text.end()-4, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*ies*/LOWER_I, UPPER_I, LOWER_E, UPPER_E, LOWER_S, UPPER_S) )
				{
				if (m_rv <= static_cast<int>(text.length()-4) &&
					!string_util::is_one_of(text[text.length()-4], FRENCH_VOWELS) )
					{
					text.erase(text.end()-3, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*ira*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_A, UPPER_A) )
				{
				if (m_rv <= static_cast<int>(text.length()-4) &&
					!string_util::is_one_of(text[text.length()-4], FRENCH_VOWELS) )
					{
					text.erase(text.end()-3, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*ît*/'î', 'Î', LOWER_T, UPPER_T) )
				{
				if (m_rv <= text.length()-3 &&
					!string_util::is_one_of(text[text.length()-3], FRENCH_VOWELS) )
					{
					text.erase(text.end()-2, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*ie*/LOWER_I, UPPER_I, LOWER_E, UPPER_E) )
				{
				if (m_rv <= text.length()-3 &&
					!string_util::is_one_of(text[text.length()-3], FRENCH_VOWELS) )
					{
					text.erase(text.end()-2, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*ir*/LOWER_I, UPPER_I, LOWER_R, UPPER_R) )
				{
				if (m_rv <= text.length()-3 &&
					!string_util::is_one_of(text[text.length()-3], FRENCH_VOWELS) )
					{
					text.erase(text.end()-2, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*is*/LOWER_I, UPPER_I, LOWER_S, UPPER_S) )
				{
				if (m_rv <= static_cast<int>(text.length()-3) &&
					!string_util::is_one_of(text[text.length()-3], FRENCH_VOWELS) )
					{
					text.erase(text.end()-2, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text,/*it*/LOWER_I, UPPER_I, LOWER_T, UPPER_T) )
				{
				if (m_rv <= static_cast<int>(text.length()-3) &&
					!string_util::is_one_of(text[text.length()-3], FRENCH_VOWELS) )
					{
					text.erase(text.end()-2, text.end() );
					return;
					}
				}
			else if (is_suffix_in_rv(text, LOWER_I, UPPER_I) )
				{
				if (m_rv <= static_cast<int>(text.length()-2) &&
					!string_util::is_one_of(text[text.length()-2], FRENCH_VOWELS) )
					{
					text.erase(text.end()-1, text.end() );
					return;
					}
				}
			//only called if 2a fails to remove a suffix
			step_2b(text);
			}
		/**Do step 2b if step 2a was done, but failed to remove a suffix.
		Search for the longest among the following suffixes, and perform the action indicated. 

			-ions 
				-delete if in R2 

			-é ée ées és èrent er era erai eraIent erais erait eras erez eriez erions erons eront ez iez 
				-delete 

			-âmes ât âtes a ai aIent ais ait ant ante antes ants as asse assent asses assiez assions 
				-delete 
				-if preceded by e, delete 

		(Note that the e that may be deleted in this last step must also be in RV.) */
		//---------------------------------------------
		void step_2b(std::basic_string<xchar>& text)
			{
			if (delete_if_is_in_rv(text,/*assions*/LOWER_A, UPPER_A, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_I, UPPER_I, LOWER_O, UPPER_O, LOWER_N, UPPER_N, LOWER_S, UPPER_S, false) )
				{
				delete_if_is_in_rv(text, LOWER_E, UPPER_E);
				return;
				}
			else if (delete_if_is_in_rv(text,/*assent*/LOWER_A, UPPER_A, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_T, UPPER_T, false) )
				{
				delete_if_is_in_rv(text, LOWER_E, UPPER_E);
				return;
				}
			else if (delete_if_is_in_rv(text,/*assiez*/LOWER_A, UPPER_A, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_I, UPPER_I, LOWER_E, UPPER_E, LOWER_Z, UPPER_Z, false) )
				{
				delete_if_is_in_rv(text, LOWER_E, UPPER_E);
				return;
				}
			else if (m_rv <= static_cast<int>(text.length()-7) &&
				(text[text.length()-7] == LOWER_E || text[text.length()-7] == UPPER_E) &&
				(text[text.length()-6] == LOWER_R || text[text.length()-6] == UPPER_R) &&
				(text[text.length()-5] == LOWER_A || text[text.length()-5] == UPPER_A) &&
				(text[text.length()-3] == LOWER_E || text[text.length()-3] == UPPER_E) &&
				(text[text.length()-2] == LOWER_N || text[text.length()-2] == UPPER_N) &&
				(text[text.length()-1] == LOWER_T || text[text.length()-1] == UPPER_T) &&
				is_either(text[text.length()-4], LOWER_I_HASH, UPPER_I_HASH) )
				{
				text.erase(text.end()-7, text.end() );
				return;
				}
			else if (delete_if_is_in_rv(text,/*erions*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_I, UPPER_I, LOWER_O, UPPER_O, LOWER_N, UPPER_N, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (m_rv <= static_cast<int>(text.length()-5) &&
				(text[text.length()-5] == LOWER_A || text[text.length()-5] == UPPER_A) &&
				(text[text.length()-3] == LOWER_E || text[text.length()-3] == UPPER_E) &&
				(text[text.length()-2] == LOWER_N || text[text.length()-2] == UPPER_N) &&
				(text[text.length()-1] == LOWER_T || text[text.length()-1] == UPPER_T) &&
				is_either(text[text.length()-4], LOWER_I_HASH, UPPER_I_HASH) )
				{
				text.erase(text.end()-5, text.end() );
				delete_if_is_in_rv(text, LOWER_E, UPPER_E);
				return;
				}
			else if (delete_if_is_in_rv(text,/*antes*/LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_E, UPPER_E, LOWER_S, UPPER_S, false) )
				{
				delete_if_is_in_rv(text, LOWER_E, UPPER_E);
				return;
				}
			else if (delete_if_is_in_rv(text,/*asses*/LOWER_A, UPPER_A, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_S, UPPER_S, false) )
				{
				delete_if_is_in_rv(text, LOWER_E, UPPER_E);
				return;
				}
			else if (delete_if_is_in_rv(text,/*èrent*/'è', 'È', LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_T, UPPER_T, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*erais*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_I, UPPER_I, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*erait*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_I, UPPER_I, LOWER_T, UPPER_T, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*eriez*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_I, UPPER_I, LOWER_E, UPPER_E, LOWER_Z, UPPER_Z, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*erons*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_O, UPPER_O, LOWER_N, UPPER_N, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*eront*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_O, UPPER_O, LOWER_N, UPPER_N, LOWER_T, UPPER_T, false) )
				{
				return;
				}
			else if (is_suffix_in_r1(text,/*ions*/LOWER_I, UPPER_I, LOWER_O, UPPER_O, LOWER_N, UPPER_N, LOWER_S, UPPER_S) &&
				delete_if_is_in_r2(text,/*ions*/LOWER_I, UPPER_I, LOWER_O, UPPER_O, LOWER_N, UPPER_N, LOWER_S, UPPER_S) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*âmes*/'â', 'Â', LOWER_M, UPPER_M, LOWER_E, UPPER_E, LOWER_S, UPPER_S, false) )
				{
				delete_if_is_in_rv(text, LOWER_E, UPPER_E);
				return;
				}
			else if (delete_if_is_in_rv(text,/*âtes*/'â', 'Â', LOWER_T, UPPER_T, LOWER_E, UPPER_E, LOWER_S, UPPER_S, false) )
				{
				delete_if_is_in_rv(text, LOWER_E, UPPER_E);
				return;
				}
			else if (delete_if_is_in_rv(text,/*ante*/LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_E, UPPER_E, false) )
				{
				delete_if_is_in_rv(text, LOWER_E, UPPER_E);
				return;
				}
			else if (delete_if_is_in_rv(text,/*ants*/LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_S, UPPER_S, false) )
				{
				delete_if_is_in_rv(text, LOWER_E, UPPER_E);
				return;
				}
			else if (delete_if_is_in_rv(text,/*asse*/LOWER_A, UPPER_A, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_E, UPPER_E, false) )
				{
				delete_if_is_in_rv(text, LOWER_E, UPPER_E);
				return;
				}
			else if (delete_if_is_in_rv(text,/*erai*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_I, UPPER_I, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*eras*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*erez*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_Z, UPPER_Z, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ais*/LOWER_A, UPPER_A, LOWER_I, UPPER_I, LOWER_S, UPPER_S, false) )
				{
				delete_if_is_in_rv(text, LOWER_E, UPPER_E);
				return;
				}
			else if (delete_if_is_in_rv(text,/*ait*/LOWER_A, UPPER_A, LOWER_I, UPPER_I, LOWER_T, UPPER_T, false) )
				{
				delete_if_is_in_rv(text, LOWER_E, UPPER_E);
				return;
				}
			else if (delete_if_is_in_rv(text,/*ant*/LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_T, UPPER_T, false) )
				{
				delete_if_is_in_rv(text, LOWER_E, UPPER_E);
				return;
				}
			else if (delete_if_is_in_rv(text,/*ées*/LOWER_E_ACUTE, UPPER_E_ACUTE, LOWER_E, UPPER_E, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*era*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_A, UPPER_A, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iez*/LOWER_I, UPPER_I, LOWER_E, UPPER_E, LOWER_Z, UPPER_Z, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ât*/'â', 'Â', LOWER_T, UPPER_T, false) )
				{
				delete_if_is_in_rv(text, LOWER_E, UPPER_E);
				return;
				}
			else if (delete_if_is_in_rv(text,/*ai*/LOWER_A, UPPER_A, LOWER_I, UPPER_I, false) )
				{
				delete_if_is_in_rv(text,LOWER_E, UPPER_E);
				return;
				}
			else if (delete_if_is_in_rv(text,/*as*/LOWER_A, UPPER_A, LOWER_S, UPPER_S, false) )
				{
				delete_if_is_in_rv(text, LOWER_E, UPPER_E);
				return;
				}
			else if (delete_if_is_in_rv(text,/*ée*/LOWER_E_ACUTE, UPPER_E_ACUTE, LOWER_E, UPPER_E, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*és*/LOWER_E_ACUTE, UPPER_E_ACUTE, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*er*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ez*/LOWER_E, UPPER_E, LOWER_Z, UPPER_Z, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text, LOWER_A, UPPER_A, false) )
				{
				delete_if_is_in_rv(text, LOWER_E, UPPER_E);
				return;
				}
			else if (delete_if_is_in_rv(text,/*é*/LOWER_E_ACUTE, UPPER_E_ACUTE, false) )
				{
				return;
				}
			}
		/**If the last step to be obeyed - either step 1, 2a or 2b - altered the word, do step 3 
		Replace final Y with i or final ç with c */
		//---------------------------------------------
		void step_3(std::basic_string<xchar>& text)
			{
			if (text[text.length()-1] == LOWER_Y_HASH)
				{
				text[text.length()-1] = LOWER_I;
				}
			else if (text[text.length()-1] == UPPER_Y_HASH)
				{
				text[text.length()-1] = UPPER_I;
				}
			else if (text[text.length()-1] == assign_character(xchar, 199) )
				{
				text[text.length()-1] = UPPER_C;
				}
			else if (text[text.length()-1] == assign_character(xchar, 231) )
				{
				text[text.length()-1] = LOWER_C;
				}
			}
		/**Alternatively, if the last step to be obeyed did not alter the word, do step 4

		If the word ends s, not preceded by a, i, o, u, è or s, delete it. 

		In the rest of step 4, all tests are confined to the RV region. 

		Search for the longest among the following suffixes, and perform the action indicated. 

			-ion 
				-delete if in R2 and preceded by s or t 

			-ier ière Ier Ière 
				-replace with i 

			-e 
				-delete 

			-ë 
				-if preceded by gu, delete 

		(So note that ion is removed only when it is in R2 - as well as being in RV -
		and preceded by s or t which must be in RV.)*/
		//---------------------------------------------
		void step_4(std::basic_string<xchar>& text)
			{
			if ((text[text.length()-1] == LOWER_S || text[text.length()-1] == UPPER_S) &&
				!string_util::is_one_of(text[text.length()-2], _XTEXT("aiouèsAIOUÈS")) )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				}

			if (is_suffix_in_rv(text,/*ière*/LOWER_I, UPPER_I, 'è', 'È', LOWER_R, UPPER_R, LOWER_E, UPPER_E) )
				{
				text.erase(text.end()-3, text.end() );
				text[text.length()-1] = LOWER_I;
				update_r_sections(text);
				return;
				}
			else if (m_rv <= static_cast<int>(text.length()-4) &&
				(text[text.length()-3] == 'è' || text[text.length()-3] == 'È') &&
				(text[text.length()-2] == LOWER_R || text[text.length()-2] == UPPER_R) &&
				(text[text.length()-1] == LOWER_E || text[text.length()-1] == UPPER_E) &&
				is_either(text[text.length()-4], LOWER_I_HASH, UPPER_I_HASH) )
				{
				text.erase(text.end()-3, text.end() );
				text[text.length()-1] = LOWER_I;
				update_r_sections(text);
				return;
				}
			else if (is_suffix_in_rv(text,/*ier*/LOWER_I, UPPER_I, LOWER_E, UPPER_E, LOWER_R, UPPER_R) )
				{
				text.erase(text.end()-2, text.end() );
				text[text.length()-1] = LOWER_I;
				update_r_sections(text);
				return;
				}
			else if (m_rv <= static_cast<int>(text.length()-3) &&
				(text[text.length()-2] == LOWER_E || text[text.length()-2] == UPPER_E) &&
				(text[text.length()-1] == LOWER_R || text[text.length()-1] == UPPER_R) &&
				is_either(text[text.length()-3], LOWER_I_HASH, UPPER_I_HASH) )
				{
				text.erase(text.end()-2, text.end() );
				text[text.length()-1] = LOWER_I;
				update_r_sections(text);
				return;
				}
			else if (is_suffix_in_rv(text,/*sion*/LOWER_S, UPPER_S, LOWER_I, UPPER_I, LOWER_O, UPPER_O, LOWER_N, UPPER_N) ||
				is_suffix_in_rv(text,/*tion*/LOWER_T, UPPER_T, LOWER_I, UPPER_I, LOWER_O, UPPER_O, LOWER_N, UPPER_N) )
				{
				if (m_r2 <= text.length()-3)
					{
					text.erase(text.end()-3, text.end() );
					update_r_sections(text);
					}
				return;
				}
			else if (is_suffix_in_rv(text,/*ë*/'ë', 'Ë') )
				{
				if (text.length() >= 3 &&
					(is_either(text[text.length()-3], LOWER_G, UPPER_G) &&
					is_either(text[text.length()-2], LOWER_U, UPPER_U) ) )
					{
					text.erase(text.end()-1, text.end() );
					}
				return;
				}
			else if (delete_if_is_in_rv(text, LOWER_E, UPPER_E) )
				{
				return;
				}
			}
		///If the word ends enn, onn, ett, ell or eill, delete the last letter 
		//---------------------------------------------
		void step_5(std::basic_string<xchar>& text)
			{
			if (is_suffix(text,/*enn*/LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_N, UPPER_N) ||
				is_suffix(text,/*onn*/LOWER_O, UPPER_O, LOWER_N, UPPER_N, LOWER_N, UPPER_N) ||
				is_suffix(text,/*ett*/LOWER_E, UPPER_E, LOWER_T, UPPER_T, LOWER_T, UPPER_T) ||
				is_suffix(text,/*ell*/LOWER_E, UPPER_E, LOWER_L, UPPER_L, LOWER_L, UPPER_L) ||
				is_suffix(text,/*eill*/LOWER_E, UPPER_E, LOWER_I, UPPER_I, LOWER_L, UPPER_L, LOWER_L, UPPER_L) )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				}
			}
		///If the words ends é or è followed by at least one non-vowel, remove the accent from the e. 
		//---------------------------------------------
		void step_6(std::basic_string<xchar>& text)
			{
			size_t last_e = text.find_last_of(_XTEXT("éèÉÈ"));
			size_t last_vowel = text.find_last_of(FRENCH_VOWELS);
			size_t last_consonant = text.find_last_not_of(FRENCH_VOWELS);
			if (last_e == std::basic_string<xchar>::npos ||
				last_consonant == std::basic_string<xchar>::npos)
				{
				return;
				}
			else if (last_e >= last_vowel &&
					last_e < last_consonant)
				{
				text[last_e] = LOWER_E;
				}
			}
		//internal data specific to French stemmer
		bool m_step_1_successful;
		};
	}

#endif //__FRENCH_STEM_H__
