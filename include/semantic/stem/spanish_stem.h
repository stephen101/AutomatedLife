/***************************************************************************
                          spanish_stem.h  -  description
                             -------------------
    begin                : Sat May 19 2004
    copyright            : (C) 2004 by Blake Madden
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the BSD License.                                *
 *                                                                         *
 ***************************************************************************/

#ifndef __SPANISH_STEM_H__
#define __SPANISH_STEM_H__

#include "semantic/stem/stemming.h"

namespace stemming
	{
	/**Letters in Spanish include the following accented forms, 
		-á é í ó ú ü ñ 
	
	The following letters are vowels: 
		-a e i o u á é í ó ú ü 
	
	R2 is defined in the usual way - see the note on R1 and R2. 

	RV is defined as follows (and this is not the same as the French stemmer definition): 

	If the second letter is a consonant, RV is the region after the next following vowel,
	or if the first two letters are vowels, RV is the region after the next consonant,
	and otherwise (consonant-vowel case) RV is the region after the third letter.
	But RV is the end of the word if these positions cannot be found. 

	For example, 

		m a c h o     o l i v a     t r a b a j o     á u r e o
			|...|         |...|         |.......|         |...|*/
	//------------------------------------------------------
	class spanish_stem : stem<xchar>
		{
	public:
		//---------------------------------------------
        ///@param text: string to stem
		void operator()(std::basic_string<xchar>& text)
			{
			if (text.length() < 3)
				{
				remove_spanish_acutes(text);
				return;
				}

			//reset internal data
			m_r1 = m_r2 = m_rv =0;

			trim_western_punctuation(text);

			find_r1(text, _XTEXT("aeiouáéíóúüAEIOUÁÉÍÓÚÜ"));
			find_r2(text, _XTEXT("aeiouáéíóúüAEIOUÁÉÍÓÚÜ"));
			find_spanish_rv(text, _XTEXT("aeiouáéíóúüAEIOUÁÉÍÓÚÜ"));

			step_0(text);
			step_1(text);
			///steps 2a and 2b and only called from step1
			step_3(text);

			remove_spanish_acutes(text);
			}
	private:
		///Always do steps 0 and 1.
		/**Search for the longest among the following suffixes 

			-me se sela selo selas selos la le lo las les los nos 

			and delete it, if comes after one of 

			-#iéndo ándo ár ér ír
			-#ando iendo ar er ir
			-#yendo following u 

		in RV. In the case of (c), yendo must lie in RV, but the preceding u can be outside it. 

		In the case of (a), deletion is followed by removing the acute accent
		(for example, haciéndola -> haciendo).*/
		//---------------------------------------------
		void step_0(std::basic_string<xchar>& text)
			{
			if (is_suffix_in_rv(text,/*selos*/LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_L, UPPER_L, LOWER_O, UPPER_O, LOWER_S, UPPER_S))
				{
				step_0a(text, 5);
				step_0b(text, 5);
				step_0c(text, 5);
				return;
				}
			else if (is_suffix_in_rv(text,/*selas*/LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_L, UPPER_L, LOWER_A, UPPER_A, LOWER_S, UPPER_S))
				{
				step_0a(text, 5);
				step_0b(text, 5);
				step_0c(text, 5);
				return;
				}
			else if (is_suffix_in_rv(text,/*sela*/LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_L, UPPER_L, LOWER_A, UPPER_A))
				{
				step_0a(text, 4);
				step_0b(text, 4);
				step_0c(text, 4);
				return;
				}
			else if (is_suffix_in_rv(text,/*selo*/LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_L, UPPER_L, LOWER_O, UPPER_O))
				{
				step_0a(text, 4);
				step_0b(text, 4);
				step_0c(text, 4);
				return;
				}
			else if (is_suffix_in_rv(text,/*las*/LOWER_L, UPPER_L, LOWER_A, UPPER_A, LOWER_S, UPPER_S))
				{
				step_0a(text, 3);
				step_0b(text, 3);
				step_0c(text, 3);
				return;
				}
			else if (is_suffix_in_rv(text,/*les*/LOWER_L, UPPER_L, LOWER_E, UPPER_E, LOWER_S, UPPER_S))
				{
				step_0a(text, 3);
				step_0b(text, 3);
				step_0c(text, 3);
				return;
				}
			else if (is_suffix_in_rv(text,/*los*/LOWER_L, UPPER_L, LOWER_O, UPPER_O, LOWER_S, UPPER_S))
				{
				step_0a(text, 3);
				step_0b(text, 3);
				step_0c(text, 3);
				return;
				}
			else if (is_suffix_in_rv(text,/*nos*/LOWER_N, UPPER_N, LOWER_O, UPPER_O, LOWER_S, UPPER_S))
				{
				step_0a(text, 3);
				step_0b(text, 3);
				step_0c(text, 3);
				return;
				}
			else if (is_suffix_in_rv(text,/*la*/LOWER_L, UPPER_L, LOWER_A, UPPER_A))
				{
				step_0a(text, 2);
				step_0b(text, 2);
				step_0c(text, 2);
				return;
				}
			else if (is_suffix_in_rv(text,/*le*/LOWER_L, UPPER_L, LOWER_E, UPPER_E))
				{
				step_0a(text, 2);
				step_0b(text, 2);
				step_0c(text, 2);
				return;
				}
			else if (is_suffix_in_rv(text,/*lo*/LOWER_L, UPPER_L, LOWER_O, UPPER_O))
				{
				step_0a(text, 2);
				step_0b(text, 2);
				step_0c(text, 2);
				return;
				}
			else if (is_suffix_in_rv(text,/*me*/LOWER_M, UPPER_M, LOWER_E, UPPER_E))
				{
				step_0a(text, 2);
				step_0b(text, 2);
				step_0c(text, 2);
				return;
				}
			else if (is_suffix_in_rv(text,/*se*/LOWER_S, UPPER_S, LOWER_E, UPPER_E))
				{
				step_0a(text, 2);
				step_0b(text, 2);
				step_0c(text, 2);
				return;
				}
			}
		//---------------------------------------------
		void step_0a(std::basic_string<xchar>& text, size_t suffix_length)
			{
			if ((text.length() >= suffix_length + 5) &&
				m_rv <= static_cast<int>(text.length()-5-suffix_length) &&
				/*iéndo*/
				(is_either(text[text.length()-5-suffix_length], LOWER_I, UPPER_I) &&
					is_either(text[text.length()-4-suffix_length], LOWER_E_ACUTE, UPPER_E_ACUTE) &&
					is_either(text[text.length()-3-suffix_length], LOWER_N, UPPER_N) &&
					is_either(text[text.length()-2-suffix_length], LOWER_D, UPPER_D) &&
					is_either(text[text.length()-1-suffix_length], LOWER_O, UPPER_O) ) )
				{
				text.erase(text.end()-suffix_length, text.end() );
				text[text.length()-4] = LOWER_E;
				update_r_sections(text);
				}
			else if ((text.length() >= suffix_length + 4) &&
				m_rv <= static_cast<int>(text.length()-4-suffix_length) &&
				/*ándo*/
				(is_either(text[text.length()-4-suffix_length], LOWER_A_ACUTE, UPPER_A_ACUTE) &&
					is_either(text[text.length()-3-suffix_length], LOWER_N, UPPER_N) &&
					is_either(text[text.length()-2-suffix_length], LOWER_D, UPPER_D) &&
					is_either(text[text.length()-1-suffix_length], LOWER_O, UPPER_O) ) )
				{
				text.erase(text.end()-suffix_length, text.end() );
				text[text.length()-4] = LOWER_A;
				update_r_sections(text);
				}
			else if ((text.length() >= suffix_length + 2) &&
				m_rv <= static_cast<int>(text.length()-2-suffix_length) &&
				/*ár*/
				(is_either(text[text.length()-2-suffix_length], LOWER_A_ACUTE, UPPER_A_ACUTE) &&
					is_either(text[text.length()-1-suffix_length], LOWER_R, UPPER_R) ) )
				{
				text.erase(text.end()-suffix_length, text.end() );
				text[text.length()-2] = LOWER_A;
				update_r_sections(text);
				}
			else if ((text.length() >= suffix_length + 2) &&
				m_rv <= static_cast<int>(text.length()-2-suffix_length) &&
				/*ér*/
				(is_either(text[text.length()-2-suffix_length], LOWER_E_ACUTE, UPPER_E_ACUTE) &&
					is_either(text[text.length()-1-suffix_length], LOWER_R, UPPER_R) ) )
				{
				text.erase(text.end()-suffix_length, text.end() );
				text[text.length()-2] = LOWER_E;
				update_r_sections(text);
				}
			else if ((text.length() >= suffix_length + 2) &&
				m_rv <= static_cast<int>(text.length()-2-suffix_length) &&
				/*ír*/
				(is_either(text[text.length()-2-suffix_length], LOWER_I_ACUTE, UPPER_I_ACUTE) &&
					is_either(text[text.length()-1-suffix_length], LOWER_R, UPPER_R) ) )
				{
				text.erase(text.end()-suffix_length, text.end() );
				text[text.length()-2] = LOWER_I;
				update_r_sections(text);
				}
			}

		//---------------------------------------------
		void step_0b(std::basic_string<xchar>& text, size_t suffix_length)
			{
			if ((text.length() >= suffix_length + 5) &&
				m_rv <= static_cast<int>(text.length()-5-suffix_length) &&
				/*iendo*/
				(is_either(text[text.length()-5-suffix_length], LOWER_I, UPPER_I) &&
					is_either(text[text.length()-4-suffix_length], LOWER_E, UPPER_E) &&
					is_either(text[text.length()-3-suffix_length], LOWER_N, UPPER_N) &&
					is_either(text[text.length()-2-suffix_length], LOWER_D, UPPER_D) &&
					is_either(text[text.length()-1-suffix_length], LOWER_O, UPPER_O) ) )
				{
				text.erase(text.end()-suffix_length, text.end() );
				update_r_sections(text);
				}
			else if ((text.length() >= suffix_length + 4) &&
				m_rv <= static_cast<int>(text.length()-4-suffix_length) &&
				/*ando*/
				(is_either(text[text.length()-4-suffix_length], LOWER_A, UPPER_A) &&
					is_either(text[text.length()-3-suffix_length], LOWER_N, UPPER_N) &&
					is_either(text[text.length()-2-suffix_length], LOWER_D, UPPER_D) &&
					is_either(text[text.length()-1-suffix_length], LOWER_O, UPPER_O) ) )
				{
				text.erase(text.end()-suffix_length, text.end() );
				update_r_sections(text);
				}
			else if ((text.length() >= suffix_length + 2) &&
				m_rv <= static_cast<int>(text.length()-2-suffix_length) &&
				/*ar*/
				(is_either(text[text.length()-2-suffix_length], LOWER_A, UPPER_A) &&
					is_either(text[text.length()-1-suffix_length], LOWER_R, UPPER_R) ) )
				{
				text.erase(text.end()-suffix_length, text.end() );
				update_r_sections(text);
				}
			else if ((text.length() >= suffix_length + 2) &&
				m_rv <= static_cast<int>(text.length()-2-suffix_length) &&
				/*er*/
				(is_either(text[text.length()-2-suffix_length], LOWER_E, UPPER_E) &&
					is_either(text[text.length()-1-suffix_length], LOWER_R, UPPER_R) ) )
				{
				text.erase(text.end()-suffix_length, text.end() );
				update_r_sections(text);
				}
			else if ((text.length() >= suffix_length + 2) &&
				m_rv <= static_cast<int>(text.length()-2-suffix_length) &&
				/*ir*/
				(is_either(text[text.length()-2-suffix_length], LOWER_I, UPPER_I) &&
					is_either(text[text.length()-1-suffix_length], LOWER_R, UPPER_R) ) )
				{
				text.erase(text.end()-suffix_length, text.end() );
				update_r_sections(text);
				}
			}

		//---------------------------------------------
		void step_0c(std::basic_string<xchar>& text, size_t suffix_length)
			{
			if ((text.length() >= suffix_length + 6) &&
				m_rv <= text.length()-(suffix_length+5) &&
				/*uyendo*/
				(is_either(text[text.length()-6-suffix_length], LOWER_U, UPPER_U) &&
					is_either(text[text.length()-5-suffix_length], LOWER_Y, UPPER_Y) &&
					is_either(text[text.length()-4-suffix_length], LOWER_E, UPPER_E) &&
					is_either(text[text.length()-3-suffix_length], LOWER_N, UPPER_N) &&
					is_either(text[text.length()-2-suffix_length], LOWER_D, UPPER_D) &&
					is_either(text[text.length()-1-suffix_length], LOWER_O, UPPER_O) ) )
				{
				text.erase(text.end()-suffix_length, text.end() );
				update_r_sections(text);
				}
			}
		/**Search for the longest among the following suffixes, and perform the action indicated. 

			-anza anzas ico ica icos icas ismo ismos able ables ible ibles ista istas oso osa osos osas amiento amientos imiento imientos 
				-delete if in R2 

			-adora ador ación adoras adores aciones ante antes ancia ancias
				-delete if in R2 
				-if preceded by ic, delete if in R2 

			-logía logías 
				-replace with log if in R2 

			-ución uciones 
				-replace with u if in R2 

			-encia encias 
				-replace with ente if in R2 

			-amente 
				-delete if in R1 
				-if preceded by iv, delete if in R2 (and if further preceded by at, delete if in R2), otherwise, 
				-if preceded by os, ic or ad, delete if in R2 

			-mente 
				-delete if in R2 
				-if preceded by ante, able or ible, delete if in R2 

			-idad idades 
				-delete if in R2 
				-if preceded by abil, ic or iv, delete if in R2 

			-iva ivo ivas ivos 
				-delete if in R2 
				-if preceded by at, delete if in R2*/
		//---------------------------------------------
		void step_1(std::basic_string<xchar>& text)
			{
			size_t original_length = text.length();
			if (delete_if_is_in_r2(text,/*imientos*/LOWER_I, UPPER_I, LOWER_M, UPPER_M, LOWER_I, UPPER_I, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_O, UPPER_O, LOWER_S, UPPER_S) )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*amientos*/LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_I, UPPER_I, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_O, UPPER_O, LOWER_S, UPPER_S) )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (is_suffix_in_r2(text,/*uciones*/LOWER_U, UPPER_U, LOWER_C, UPPER_C, LOWER_I, UPPER_I, LOWER_O, UPPER_O, LOWER_N, UPPER_N, LOWER_E, UPPER_E, LOWER_S, UPPER_S) )
				{
				text.erase(text.end()-6, text.end() );
				update_r_sections(text);
				return;
				}
			else if (delete_if_is_in_r2(text,/*amiento*/LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_I, UPPER_I, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_O, UPPER_O) )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*imiento*/LOWER_I, UPPER_I, LOWER_M, UPPER_M, LOWER_I, UPPER_I, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_O, UPPER_O) )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*aciones*/LOWER_A, UPPER_A, LOWER_C, UPPER_C, LOWER_I, UPPER_I, LOWER_O, UPPER_O, LOWER_N, UPPER_N, LOWER_E, UPPER_E, LOWER_S, UPPER_S) )
				{
				if (original_length > text.length() )
					{
					delete_if_is_in_r2(text,/*ic*/LOWER_I, UPPER_I, LOWER_C, UPPER_C);
					return;
					}
				step_2a(text);
				}
			else if (is_suffix_in_r2(text,/*logías*/LOWER_L, UPPER_L, LOWER_O, UPPER_O, LOWER_G, UPPER_G, LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_A, UPPER_A, LOWER_S, UPPER_S) )
				{
				text.erase(text.end()-3, text.end() );
				update_r_sections(text);
				return;
				}
			else if (is_suffix_in_r2(text,/*encias*/LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_C, UPPER_C, LOWER_I, UPPER_I, LOWER_A, UPPER_A, LOWER_S, UPPER_S) )
				{
				text.erase(text.end()-2, text.end() );
				text[text.length()-2] = LOWER_T;
				text[text.length()-1] = LOWER_E;
				update_r_sections(text);
				return;
				}
			else if (delete_if_is_in_r2(text,/*idades*/LOWER_I, UPPER_I, LOWER_D, UPPER_D, LOWER_A, UPPER_A, LOWER_D, UPPER_D, LOWER_E, UPPER_E, LOWER_S, UPPER_S) )
				{
				if (original_length > text.length() )
					{
					if (delete_if_is_in_r2(text,/*abil*/LOWER_A, UPPER_A, LOWER_B, UPPER_B, LOWER_I, UPPER_I, LOWER_L, UPPER_L) ||
						delete_if_is_in_r2(text,/*ic*/LOWER_I, UPPER_I, LOWER_C, UPPER_C) ||
						delete_if_is_in_r2(text,/*iv*/LOWER_I, UPPER_I, LOWER_V, UPPER_V) )
						{
						return;
						}
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r1(text,/*amente*/LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_E, UPPER_E) )
				{
				if (original_length > text.length() )
					{
					if (delete_if_is_in_r2(text,/*iv*/LOWER_I, UPPER_I, LOWER_V, UPPER_V) )
						{
						delete_if_is_in_r2(text,/*at*/LOWER_A, UPPER_A, LOWER_T, UPPER_T);
						return;
						}
					else
						{
						if (delete_if_is_in_r2(text,/*os*/LOWER_O, UPPER_O, LOWER_S, UPPER_S) ||
							delete_if_is_in_r2(text,/*ic*/LOWER_I, UPPER_I, LOWER_C, UPPER_C) ||
							delete_if_is_in_r2(text,/*ad*/LOWER_A, UPPER_A, LOWER_D, UPPER_D) )
							{
							return;
							}
						}
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*adores*/LOWER_A, UPPER_A, LOWER_D, UPPER_D, LOWER_O, UPPER_O, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_S, UPPER_S) ||
                     delete_if_is_in_r2(text,/*adoras*/LOWER_A, UPPER_A, LOWER_D, UPPER_D, LOWER_O, UPPER_O, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_S, UPPER_S) ||
                     delete_if_is_in_r2(text,/*ancias*/LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_C, UPPER_C, LOWER_I, UPPER_I, LOWER_A, UPPER_A, LOWER_S, UPPER_S))
				{
				if (original_length > text.length() )
					{
					delete_if_is_in_r2(text,/*ic*/LOWER_I, UPPER_I, LOWER_C, UPPER_C);
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*adora*/LOWER_A, UPPER_A, LOWER_D, UPPER_D, LOWER_O, UPPER_O, LOWER_R, UPPER_R, LOWER_A, UPPER_A) ||
                     delete_if_is_in_r2(text,/*ación*/LOWER_A, UPPER_A, LOWER_C, UPPER_C, LOWER_I, UPPER_I, LOWER_O_ACUTE, UPPER_O_ACUTE, LOWER_N, UPPER_N) ||
                     delete_if_is_in_r2(text,/*antes*/LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_E, UPPER_E, LOWER_S, UPPER_S) ||
                     delete_if_is_in_r2(text,/*ancia*/LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_C, UPPER_C, LOWER_I, UPPER_I, LOWER_A, UPPER_A) )
				{
				if (original_length > text.length() )
					{
					delete_if_is_in_r2(text,/*ic*/LOWER_I, UPPER_I, LOWER_C, UPPER_C);
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*ibles*/LOWER_I, UPPER_I, LOWER_B, UPPER_B, LOWER_L, UPPER_L, LOWER_E, UPPER_E, LOWER_S, UPPER_S) )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*istas*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_T, UPPER_T, LOWER_A, UPPER_A, LOWER_S, UPPER_S) )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*ables*/LOWER_A, UPPER_A, LOWER_B, UPPER_B, LOWER_L, UPPER_L, LOWER_E, UPPER_E, LOWER_S, UPPER_S) )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*ismos*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S) )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*anzas*/LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_Z, UPPER_Z, LOWER_A, UPPER_A, LOWER_S, UPPER_S) )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (is_suffix_in_r2(text,/*logía*/LOWER_L, UPPER_L, LOWER_O, UPPER_O, LOWER_G, UPPER_G, LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_A, UPPER_A) )
				{
				text.erase(text.end()-2, text.end() );
				update_r_sections(text);
				return;
				}
			else if (is_suffix_in_r2(text,/*ución*/LOWER_U, UPPER_U, LOWER_C, UPPER_C, LOWER_I, UPPER_I, LOWER_O_ACUTE, UPPER_O_ACUTE, LOWER_N, UPPER_N) )
				{
				text.erase(text.end()-4, text.end() );
				update_r_sections(text);
				return;
				}
			else if (is_suffix_in_r2(text,/*encia*/LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_C, UPPER_C, LOWER_I, UPPER_I, LOWER_A, UPPER_A) )
				{
				text.erase(text.end()-1, text.end() );
				text[text.length()-2] = LOWER_T;
				text[text.length()-1] = LOWER_E;
				update_r_sections(text);
				return;
				}
			else if (delete_if_is_in_r2(text,/*mente*/LOWER_M, UPPER_M, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_E, UPPER_E) )
				{
				if (original_length > text.length() )
					{
					if (delete_if_is_in_r2(text,/*ante*/LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_E, UPPER_E) ||
                        delete_if_is_in_r2(text,/*able*/LOWER_A, UPPER_A, LOWER_B, UPPER_B, LOWER_L, UPPER_L, LOWER_E, UPPER_E) ||
						delete_if_is_in_r2(text,/*ible*/LOWER_I, UPPER_I, LOWER_B, UPPER_B, LOWER_L, UPPER_L, LOWER_E, UPPER_E) )
						{
						return;
						}
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*anza*/LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_Z, UPPER_Z, LOWER_A, UPPER_A) )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*icos*/LOWER_I, UPPER_I, LOWER_C, UPPER_C, LOWER_O, UPPER_O, LOWER_S, UPPER_S) )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*icas*/LOWER_I, UPPER_I, LOWER_C, UPPER_C, LOWER_A, UPPER_A, LOWER_S, UPPER_S) )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*ismo*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_M, UPPER_M, LOWER_O, UPPER_O) )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*able*/LOWER_A, UPPER_A, LOWER_B, UPPER_B, LOWER_L, UPPER_L, LOWER_E, UPPER_E) )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*ible*/LOWER_I, UPPER_I, LOWER_B, UPPER_B, LOWER_L, UPPER_L, LOWER_E, UPPER_E) )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*ista*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_T, UPPER_T, LOWER_A, UPPER_A) )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*osos*/LOWER_O, UPPER_O, LOWER_S, UPPER_S, LOWER_O, UPPER_O, LOWER_S, UPPER_S) )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*osas*/LOWER_O, UPPER_O, LOWER_S, UPPER_S, LOWER_A, UPPER_A, LOWER_S, UPPER_S) )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*ivas*/LOWER_I, UPPER_I, LOWER_V, UPPER_V, LOWER_A, UPPER_A, LOWER_S, UPPER_S) )
				{
				if (original_length > text.length() )
					{
					delete_if_is_in_r2(text,/*at*/LOWER_A, UPPER_A, LOWER_T, UPPER_T);
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*ivos*/LOWER_I, UPPER_I, LOWER_V, UPPER_V, LOWER_O, UPPER_O, LOWER_S, UPPER_S) )
				{
				if (original_length > text.length() )
					{
					delete_if_is_in_r2(text,/*at*/LOWER_A, UPPER_A, LOWER_T, UPPER_T);
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*ador*/LOWER_A, UPPER_A, LOWER_D, UPPER_D, LOWER_O, UPPER_O, LOWER_R, UPPER_R) ||
                    delete_if_is_in_r2(text,/*ante*/LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_E, UPPER_E) )
				{
				if (original_length > text.length() )
					{
					delete_if_is_in_r2(text,/*ic*/LOWER_I, UPPER_I, LOWER_C, UPPER_C);
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*idad*/LOWER_I, UPPER_I, LOWER_D, UPPER_D, LOWER_A, UPPER_A, LOWER_D, UPPER_D) )
				{
				if (original_length > text.length() )
					{
					if (delete_if_is_in_r2(text,/*abil*/LOWER_A, UPPER_A, LOWER_B, UPPER_B, LOWER_I, UPPER_I, LOWER_L, UPPER_L) ||
						delete_if_is_in_r2(text,/*ic*/LOWER_I, UPPER_I, LOWER_C, UPPER_C) ||
						delete_if_is_in_r2(text,/*iv*/LOWER_I, UPPER_I, LOWER_V, UPPER_V) )
						{
						return;
						}
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*ico*/LOWER_I, UPPER_I, LOWER_C, UPPER_C, LOWER_O, UPPER_O) )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*ica*/LOWER_I, UPPER_I, LOWER_C, UPPER_C, LOWER_A, UPPER_A) )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*oso*/LOWER_O, UPPER_O, LOWER_S, UPPER_S, LOWER_O, UPPER_O) )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*osa*/LOWER_O, UPPER_O, LOWER_S, UPPER_S, LOWER_A, UPPER_A) )
				{
				if (original_length > text.length() )
					{
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*iva*/LOWER_I, UPPER_I, LOWER_V, UPPER_V, LOWER_A, UPPER_A) )
				{
				if (original_length > text.length() )
					{
					delete_if_is_in_r2(text,/*at*/LOWER_A, UPPER_A, LOWER_T, UPPER_T);
					return;
					}
				step_2a(text);
				}
			else if (delete_if_is_in_r2(text,/*ivo*/LOWER_I, UPPER_I, LOWER_V, UPPER_V, LOWER_O, UPPER_O) )
				{
				if (original_length > text.length() )
					{
					delete_if_is_in_r2(text,/*at*/LOWER_A, UPPER_A, LOWER_T, UPPER_T);
					return;
					}
				step_2a(text);
				}	
			//this should only be called from here if nothing was removed in step 1
			if (text.length() == original_length)
				{
				step_2a(text);
				}
			}
		///Do step 2a if no ending was removed by step 1.
		/**Search for the longest among the following suffixes in RV, and if found, delete if preceded by u. 

			-ya ye yan yen yeron yendo yo yó yas yes yais yamos 

		(Note that the preceding u need not be in RV.)*/
		//---------------------------------------------
		void step_2a(std::basic_string<xchar>& text)
			{
			size_t original_length = text.length();
			if (is_suffix_in_rv(text,/*yeron*/LOWER_Y, UPPER_Y, LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_O, UPPER_O, LOWER_N, UPPER_N))
				{
				if (is_either(text[text.length()-6], LOWER_U, UPPER_U) )
					{
					text.erase(text.end()-5, text.end() );
					update_r_sections(text);
					return;
					}
				step_2b(text);
				}
			else if (is_suffix_in_rv(text,/*yendo*/LOWER_Y, UPPER_Y, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_D, UPPER_D, LOWER_O, UPPER_O))
				{
				if (is_either(text[text.length()-6], LOWER_U, UPPER_U) )
						{
					text.erase(text.end()-5, text.end() );
					update_r_sections(text);
					return;
					}
				step_2b(text);
				}
			else if (is_suffix_in_rv(text,/*yamos*/LOWER_Y, UPPER_Y, LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S))
				{
				if (is_either(text[text.length()-6], LOWER_U, UPPER_U) )
					{
					text.erase(text.end()-5, text.end() );
					update_r_sections(text);
					return;
					}
				step_2b(text);
				}
			else if (is_suffix_in_rv(text,/*yais*/LOWER_Y, UPPER_Y, LOWER_A, UPPER_A, LOWER_I, UPPER_I, LOWER_S, UPPER_S))
				{
				if (is_either(text[text.length()-5], LOWER_U, UPPER_U) )
					{
					text.erase(text.end()-4, text.end() );
					update_r_sections(text);
					return;
					}
				step_2b(text);
				}
			else if (is_suffix_in_rv(text,/*yan*/LOWER_Y, UPPER_Y, LOWER_A, UPPER_A, LOWER_N, UPPER_N))
				{
				if (is_either(text[text.length()-4], LOWER_U, UPPER_U) )
					{
					text.erase(text.end()-3, text.end() );
					update_r_sections(text);
					return;
					}
				step_2b(text);
				}
			else if (is_suffix_in_rv(text,/*yen*/LOWER_Y, UPPER_Y, LOWER_E, UPPER_E, LOWER_N, UPPER_N))
				{
				if (is_either(text[text.length()-4], LOWER_U, UPPER_U) )
					{
					text.erase(text.end()-3, text.end() );
					update_r_sections(text);
					return;
					}
				step_2b(text);
				}
			else if (is_suffix_in_rv(text,/*yas*/LOWER_Y, UPPER_Y, LOWER_A, UPPER_A, LOWER_S, UPPER_S))
				{
				if (is_either(text[text.length()-4], LOWER_U, UPPER_U) )
					{
					text.erase(text.end()-3, text.end() );
					update_r_sections(text);
					return;
					}
				step_2b(text);
				}
			else if (is_suffix_in_rv(text,/*yes*/LOWER_Y, UPPER_Y, LOWER_E, UPPER_E, LOWER_S, UPPER_S))
				{
				if (is_either(text[text.length()-4], LOWER_U, UPPER_U) )
					{
					text.erase(text.end()-3, text.end() );
					update_r_sections(text);
					return;
					}
				step_2b(text);
				}
			else if (is_suffix_in_rv(text,/*ya*/LOWER_Y, UPPER_Y, LOWER_A, UPPER_A))
				{
				if (is_either(text[text.length()-3], LOWER_U, UPPER_U) )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					return;
					}
				step_2b(text);
				}
			else if (is_suffix_in_rv(text,/*ye*/LOWER_Y, UPPER_Y, LOWER_E, UPPER_E))
				{
				if (is_either(text[text.length()-3],LOWER_U, UPPER_U) )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					return;
					}
				step_2b(text);
				}
			else if (is_suffix_in_rv(text,/*yo*/LOWER_Y, UPPER_Y, LOWER_O, UPPER_O))
				{
				if (is_either(text[text.length()-3], LOWER_U, UPPER_U) )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					return;
					}
				step_2b(text);
				}
			else if (is_suffix_in_rv(text,/*yó*/LOWER_Y, UPPER_Y, LOWER_O_ACUTE, UPPER_O_ACUTE))
				{
				if (is_either(text[text.length()-3], LOWER_U, UPPER_U) )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					return;
					}
				step_2b(text);
				}
			//only called if 2a fails to remove a suffix
			if (text.length() == original_length)
				{
				step_2b(text);
				}
			}
		///Do Step 2b if step 2a was done, but failed to remove a suffix.
		/**Search for the longest among the following suffixes in RV, and perform the action indicated. 

			-en es éis emos 
				-delete, and if preceded by gu delete the u (the gu need not be in RV) 

			-arían arías arán arás aríais aría aréis aríamos aremos ará aré erían erías
			erán erás eríais ería eréis eríamos eremos erá eré irían irías irán irás iríais
			iría iréis iríamos iremos irá iré aba ada ida ía ara iera ad ed id ase iese aste
			iste an aban ían aran ieran asen iesen aron ieron ado ido ando iendo ió ar er ir
			as abas adas idas ías aras ieras ases ieses ís áis abais íais arais ierais
			aseis ieseis asteis isteis ados idos amos ábamos íamos imos áramos iéramos iésemos ásemos 
				-delete*/
		//---------------------------------------------
		void step_2b(std::basic_string<xchar>& text) 
			{
			if (delete_if_is_in_rv(text,/*aríamos*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*eríamos*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iríamos*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iéramos*/LOWER_I, UPPER_I, LOWER_E_ACUTE, UPPER_E_ACUTE, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iésemos*/LOWER_I, UPPER_I, LOWER_E_ACUTE, UPPER_E_ACUTE, LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*aríais*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_A, UPPER_A, LOWER_I, UPPER_I, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*aremos*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*eríais*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_A, UPPER_A, LOWER_I, UPPER_I, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*eremos*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iríais*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_A, UPPER_A, LOWER_I, UPPER_I, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iremos*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ierais*/LOWER_I, UPPER_I, LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_I, UPPER_I, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ieseis*/LOWER_I, UPPER_I, LOWER_E, UPPER_E, LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_I, UPPER_I, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*asteis*/LOWER_A, UPPER_A, LOWER_S, UPPER_S, LOWER_T, UPPER_T, LOWER_E, UPPER_E, LOWER_I, UPPER_I, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*isteis*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_T, UPPER_T, LOWER_E, UPPER_E, LOWER_I, UPPER_I, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ábamos*/LOWER_A_ACUTE, UPPER_A_ACUTE, LOWER_B, UPPER_B, LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*áramos*/LOWER_A_ACUTE, UPPER_A_ACUTE, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ásemos*/LOWER_A_ACUTE, UPPER_A_ACUTE, LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*arían*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_A, UPPER_A, LOWER_N, UPPER_N, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*arías*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_A, UPPER_A, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*aréis*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_E_ACUTE, UPPER_E_ACUTE, LOWER_I, UPPER_I, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*erían*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_A, UPPER_A, LOWER_N, UPPER_N, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*erías*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_A, UPPER_A, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*eréis*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_E_ACUTE, UPPER_E_ACUTE, LOWER_I, UPPER_I, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*irían*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_A, UPPER_A, LOWER_N, UPPER_N, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*irías*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_A, UPPER_A, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iréis*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_E_ACUTE, UPPER_E_ACUTE, LOWER_I, UPPER_I, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ieran*/LOWER_I, UPPER_I, LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_N, UPPER_N, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iesen*/LOWER_I, UPPER_I, LOWER_E, UPPER_E, LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_N, UPPER_N, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ieron*/LOWER_I, UPPER_I, LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_O, UPPER_O, LOWER_N, UPPER_N, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iendo*/LOWER_I, UPPER_I, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_D, UPPER_D, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ieras*/LOWER_I, UPPER_I, LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ieses*/LOWER_I, UPPER_I, LOWER_E, UPPER_E, LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*abais*/LOWER_A, UPPER_A, LOWER_B, UPPER_B, LOWER_A, UPPER_A, LOWER_I, UPPER_I, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*arais*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_I, UPPER_I, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*aseis*/LOWER_A, UPPER_A, LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_I, UPPER_I, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*íamos*/LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*emos*/LOWER_E, UPPER_E, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				if (is_suffix(text,/*gu*/LOWER_G, UPPER_G, LOWER_U, UPPER_U) )
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					}
				return;
				}
			else if (delete_if_is_in_rv(text,/*arán*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_A_ACUTE, UPPER_A_ACUTE, LOWER_N, UPPER_N, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*arás*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_A_ACUTE, UPPER_A_ACUTE, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*aría*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_A, UPPER_A, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*erán*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_A_ACUTE, UPPER_A_ACUTE, LOWER_N, UPPER_N, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*erás*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_A_ACUTE, UPPER_A_ACUTE, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ería*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_A, UPPER_A, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*irán*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_A_ACUTE, UPPER_A_ACUTE, LOWER_N, UPPER_N, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*irás*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_A_ACUTE, UPPER_A_ACUTE, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iría*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_A, UPPER_A, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iera*/LOWER_I, UPPER_I, LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_A, UPPER_A, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iese*/LOWER_I, UPPER_I, LOWER_E, UPPER_E, LOWER_S, UPPER_S, LOWER_E, UPPER_E, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*aste*/LOWER_A, UPPER_A, LOWER_S, UPPER_S, LOWER_T, UPPER_T, LOWER_E, UPPER_E, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iste*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_T, UPPER_T, LOWER_E, UPPER_E, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*aban*/LOWER_A, UPPER_A, LOWER_B, UPPER_B, LOWER_A, UPPER_A, LOWER_N, UPPER_N, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*aran*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_N, UPPER_N, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*asen*/LOWER_A, UPPER_A, LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_N, UPPER_N, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*aron*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_O, UPPER_O, LOWER_N, UPPER_N, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ando*/LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_D, UPPER_D, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*abas*/LOWER_A, UPPER_A, LOWER_B, UPPER_B, LOWER_A, UPPER_A, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*adas*/LOWER_A, UPPER_A, LOWER_D, UPPER_D, LOWER_A, UPPER_A, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*idas*/LOWER_I, UPPER_I, LOWER_D, UPPER_D, LOWER_A, UPPER_A, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*aras*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ases*/LOWER_A, UPPER_A, LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*íais*/LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_A, UPPER_A, LOWER_I, UPPER_I, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ados*/LOWER_A, UPPER_A, LOWER_D, UPPER_D, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*idos*/LOWER_I, UPPER_I, LOWER_D, UPPER_D, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*amos*/LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*imos*/LOWER_I, UPPER_I, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ará*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_A_ACUTE, UPPER_A_ACUTE, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*aré*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_E_ACUTE, UPPER_E_ACUTE, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*erá*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_A_ACUTE, UPPER_A_ACUTE, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*eré*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_E_ACUTE, UPPER_E_ACUTE, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*irá*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_A_ACUTE, UPPER_A_ACUTE, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iré*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_E_ACUTE, UPPER_E_ACUTE, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*aba*/LOWER_A, UPPER_A, LOWER_B, UPPER_B, LOWER_A, UPPER_A, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ada*/LOWER_A, UPPER_A, LOWER_D, UPPER_D, LOWER_A, UPPER_A, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ida*/LOWER_I, UPPER_I, LOWER_D, UPPER_D, LOWER_A, UPPER_A, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ara*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_A, UPPER_A, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ase*/LOWER_A, UPPER_A, LOWER_S, UPPER_S, LOWER_E, UPPER_E, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ían*/LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_A, UPPER_A, LOWER_N, UPPER_N, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ado*/LOWER_A, UPPER_A, LOWER_D, UPPER_D, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ido*/LOWER_I, UPPER_I, LOWER_D, UPPER_D, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ías*/LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_A, UPPER_A, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*áis*/LOWER_A_ACUTE, UPPER_A_ACUTE, LOWER_I, UPPER_I, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*éis*/LOWER_E_ACUTE, UPPER_E_ACUTE, LOWER_I, UPPER_I, LOWER_S, UPPER_S, false) )
				{
				if (is_suffix(text,/*gu*/LOWER_G, UPPER_G, LOWER_U, UPPER_U) )
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					}
				return;
				}
			else if (delete_if_is_in_rv(text,/*ía*/LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_A, UPPER_A, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ad*/LOWER_A, UPPER_A, LOWER_D, UPPER_D, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ed*/LOWER_E, UPPER_E, LOWER_D, UPPER_D, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*id*/LOWER_I, UPPER_I, LOWER_D, UPPER_D, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*an*/LOWER_A, UPPER_A, LOWER_N, UPPER_N, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ió*/LOWER_I, UPPER_I, LOWER_O_ACUTE, UPPER_O_ACUTE, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ar*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*er*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ir*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*as*/LOWER_A, UPPER_A, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ís*/LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*en*/LOWER_E, UPPER_E, LOWER_N, UPPER_N, false) )
				{
				if (is_suffix(text,/*gu*/LOWER_G, UPPER_G, LOWER_U, UPPER_U) )
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					}
				return;
				}
			else if (delete_if_is_in_rv(text,/*es*/LOWER_E, UPPER_E, LOWER_S, UPPER_S, false) )
				{
				if (is_suffix(text,/*gu*/LOWER_G, UPPER_G, LOWER_U, UPPER_U) )
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					}
				return;
				}
			}
		///Always do step 3.
		/**Search for the longest among the following suffixes in RV, and perform the action indicated. 

			-os a o á í ó 
				-delete if in RV 

			-e é 
				-delete if in RV, and if preceded by gu with the u in RV delete the u*/
		//---------------------------------------------
		void step_3(std::basic_string<xchar>& text) 
			{
			if (delete_if_is_in_rv(text,/*os*/LOWER_O, UPPER_O, LOWER_S, UPPER_S) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*a*/LOWER_A, UPPER_A) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*o*/LOWER_O, UPPER_O) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*á*/LOWER_A_ACUTE, UPPER_A_ACUTE) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*í*/LOWER_I_ACUTE, UPPER_I_ACUTE) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ó*/LOWER_O_ACUTE, UPPER_O_ACUTE) )
				{
				return;
				}

			else if (delete_if_is_in_rv(text,/*é*/LOWER_E_ACUTE, UPPER_E_ACUTE) ||
					delete_if_is_in_rv(text,/*e*/LOWER_E, UPPER_E) )
				{
				if (is_suffix_in_rv(text,/*u*/LOWER_U, UPPER_U))
					{
					if (is_either(text[text.length()-2], LOWER_G, UPPER_G) )
						{
						text.erase(text.end()-1, text.end() );
						}
					}
				return;
				}
			}
		};
	}

#endif //__SPANISH_STEM_H__
