/***************************************************************************
                          finnish_stem.h  -  description
                             -------------------
    begin                : Sat May 11 2004
    copyright            : (C) 2004 by Blake Madden
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the BSD License.                                *
 *                                                                         *
 ***************************************************************************/

#ifndef __FINNISH_STEM_H__
#define __FINNISH_STEM_H__

#include "semantic/stem/stemming.h"

namespace stemming
	{
	/**Finnish is not an Indo-European language, but belongs to the Finno-Ugric group,
	which again belongs to the Uralic group. Distinctions between a-, i- and d-suffixes
	can be made in Finnish, but they are much less sharply separated than in an
	Indo-European language. The system of endings is extremely elaborate, but strictly
	defined, and applies equally to all nominals, that is, to nouns, adjectives and pronouns.
	Verb endings have a close similarity to nominal endings, which again makes Finnish
	very different from any Indo-European language. 

	More problematical than the endings themselves is the change that can be effected in a
	stem as a result of taking a particular ending. A stem typically has two forms
	,strong and weak, where one class of ending follows the strong form and the complementary
	class the weak. Normalising strong and weak forms after ending removal is not generally possible,
	although the common case where strong and weak forms only differ in the single or double form of
	a final consonant can be dealt with. 

	Finnish includes the following accented forms, 
		-ä   ö 
	The following letters are vowels: 
		-a   e   i   o   u   y   ä   ö

	R1 and R2 are then defined in the usual way.*/
	//------------------------------------------------------
	class finnish_stem : stem<xchar>
		{
	public:
		finnish_stem() : m_step_3_successful(false) {}
		//---------------------------------------------
        ///@param text: string to stem
		void operator()(std::basic_string<xchar>& text)
			{
			if (text.length() < 2)
				{
				return;
				}

			//reset internal data
			m_step_3_successful = false;
			m_r1 = m_r2 = m_rv =0;

			trim_western_punctuation(text);

			find_r1(text, FINNISH_VOWELS);
			find_r2(text, FINNISH_VOWELS);

			step_1(text);
			step_2(text);
			step_3(text);
			step_4(text);
			step_5(text);
			step_6(text);
			}
	private:
		/**Step 1: particles etc 
		Search for the longest among the following suffixes in R1, and perform the action indicated 

			-# kin   kaan   kään   ko   kö   han   hän   pa   pä
				- delete if preceded by n, t or a vowel
			-# sti
				- delete if in R2
			(Of course, the n, t or vowel of 1(a) need not be in R1:
			only the suffix removed must be in R1. And similarly below*/
		//---------------------------------------------
		void step_1(std::basic_string<xchar>& text)
			{
			if (is_suffix_in_r1(text,/*kaan*/LOWER_K, UPPER_K, LOWER_A, UPPER_A, LOWER_A, UPPER_A, LOWER_N, UPPER_N) ||
				is_suffix_in_r1(text,/*kään*/LOWER_K, UPPER_K, LOWER_A_UMLAUTS, UPPER_A_UMLAUTS, LOWER_A_UMLAUTS, UPPER_A_UMLAUTS, LOWER_N, UPPER_N) )
				{
				if (text.length() >= 5 &&
					string_util::is_one_of(text[text.length()-5], _XTEXT("ntaeiouyäöNTAEIOUYÄÖ")) )
					{
					text.erase(text.end()-4, text.end() );
					update_r_sections(text);
					}
				return;
				}
			else if (is_suffix_in_r1(text,/*kin*/LOWER_K, UPPER_K, LOWER_I, UPPER_I, LOWER_N, UPPER_N) ||
				is_suffix_in_r1(text,/*han*/LOWER_H, UPPER_H, LOWER_A, UPPER_A, LOWER_N, UPPER_N) ||
				is_suffix_in_r1(text,/*hän*/LOWER_H, UPPER_H, LOWER_A_UMLAUTS, UPPER_A_UMLAUTS, LOWER_N, UPPER_N) )
				{
				if (text.length() >= 4 &&
					string_util::is_one_of(text[text.length()-4], _XTEXT("ntaeiouyäöNTAEIOUYÄÖ")) )
					{
					text.erase(text.end()-3, text.end() );
					update_r_sections(text);
					}
				return;
				}
			else if (is_suffix_in_r1(text,/*sti*/LOWER_S, UPPER_S, LOWER_T, UPPER_T, LOWER_I, UPPER_I) )
				{
				delete_if_is_in_r2(text,/*sti*/LOWER_S, UPPER_S, LOWER_T, UPPER_T, LOWER_I, UPPER_I);
				return;
				}
			else if (is_suffix_in_r1(text,/*ko*/LOWER_K, UPPER_K, LOWER_O, UPPER_O) ||
				is_suffix_in_r1(text,/*kö*/LOWER_K, UPPER_K, LOWER_O_UMLAUTS, UPPER_O_UMLAUTS) ||
				is_suffix_in_r1(text,/*pa*/LOWER_P, UPPER_P, LOWER_A, UPPER_A) ||
				is_suffix_in_r1(text,/*pä*/LOWER_P, UPPER_P, LOWER_A_UMLAUTS, UPPER_A_UMLAUTS) )
				{
				if (text.length() >= 3 &&
					string_util::is_one_of(text[text.length()-3], _XTEXT("ntaeiouyäöNTAEIOUYÄÖ")) )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				return;
				}
			}
		/**Step 2: possessives 
		Search for the longest among the following suffixes in R1, and perform the action indicated 

			- si 
				- delete if not preceded by k 
			- ni 
				- delete 
				- if preceded by kse, replace with ksi 
			- nsa   nsä   mme   nne 
				- delete 
			- an 
				- delete if preceded by one of   ta   ssa   sta   lla   lta   na 
			- än 
				- delete if preceded by one of   tä   ssä   stä   llä   ltä   nä 
			- en 
				- delete if preceded by one of   lle   ine*/
		//---------------------------------------------
		void step_2(std::basic_string<xchar>& text)
			{
			if (delete_if_is_in_r1(text,/*nsa*/LOWER_N, UPPER_N, LOWER_S, UPPER_S, LOWER_A, UPPER_A, false) ||
				delete_if_is_in_r1(text,/*nsä*/LOWER_N, UPPER_N, LOWER_S, UPPER_S, LOWER_A_UMLAUTS, UPPER_A_UMLAUTS, false) ||
				delete_if_is_in_r1(text,/*mme*/LOWER_M, UPPER_M, LOWER_M, UPPER_M, LOWER_E, UPPER_E, false) ||
				delete_if_is_in_r1(text,/*nne*/LOWER_N, UPPER_N, LOWER_N, UPPER_N, LOWER_E, UPPER_E, false) )
				{
				return;
				}
			else if (is_suffix_in_r1(text,/*si*/LOWER_S, UPPER_S, LOWER_I, UPPER_I) )
				{
				if (text.length() >= 3 &&
					!(text[text.length()-3] == LOWER_K || text[text.length()-3] == UPPER_K))
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				return;
				}
			else if (delete_if_is_in_r1(text,/*ni*/LOWER_N, UPPER_N, LOWER_I, UPPER_I, false) )
				{
				if (is_suffix(text, /*kse*/LOWER_K, UPPER_K, LOWER_S, UPPER_S, LOWER_E, UPPER_E) )
					{
					text[text.length()-1] = LOWER_I;
					}
				return;
				}
			else if (is_suffix_in_r1(text,/*an*/LOWER_A, UPPER_A, LOWER_N, UPPER_N) )
				{
				if ((text.length() >= 4 &&
                    (string_util::strnicmp(text.c_str()+(text.length()-4), _XTEXT("ta"), 2) == 0 ||
					string_util::strnicmp(text.c_str()+(text.length()-4), _XTEXT("na"), 2) == 0) ) ||
					(text.length() >= 5 &&
					(string_util::strnicmp(text.c_str()+(text.length()-5), _XTEXT("ssa"), 3) == 0 ||
					string_util::strnicmp(text.c_str()+(text.length()-5), _XTEXT("sta"), 3) == 0 ||
					string_util::strnicmp(text.c_str()+(text.length()-5), _XTEXT("lla"), 3) == 0 ||
					string_util::strnicmp(text.c_str()+(text.length()-5), _XTEXT("lta"), 3) == 0) ) )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				return;
				}
			else if (is_suffix_in_r1(text,/*än*/LOWER_A_UMLAUTS, UPPER_A_UMLAUTS, LOWER_N, UPPER_N) )
				{
				if ((text.length() >= 4 &&
					(string_util::strnicmp(text.c_str()+(text.length()-4), _XTEXT("tä"), 2) == 0 ||
					string_util::strnicmp(text.c_str()+(text.length()-4), _XTEXT("nä"), 2) == 0) ) ||
					(text.length() >= 5 &&
					(string_util::strnicmp(text.c_str()+(text.length()-5), _XTEXT("ssä"), 3) == 0 ||
					string_util::strnicmp(text.c_str()+(text.length()-5), _XTEXT("stä"), 3) == 0 ||
					string_util::strnicmp(text.c_str()+(text.length()-5), _XTEXT("llä"), 3) == 0 ||
					string_util::strnicmp(text.c_str()+(text.length()-5), _XTEXT("ltä"), 3) == 0) ) )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				return;
				}
			else if (is_suffix_in_r1(text,/*en*/LOWER_E, UPPER_E, LOWER_N, UPPER_N) )
				{
				if (text.length() >= 5 &&
					(string_util::strnicmp(text.c_str()+(text.length()-5), _XTEXT("lle"), 3) == 0 ||
					string_util::strnicmp(text.c_str()+(text.length()-5), _XTEXT("ine"), 3) == 0 ) )
					{
					text.erase(text.end()-2, text.end() );
					update_r_sections(text);
					}
				return;
				}
			}

		//////////////////////////////////////////////////////////////////////////
		// Define a v (vowel) as one of   a   e   i   o   u   y   ä   ö.        //
		// Define a V (restricted vowel) as one of   a   e   i   o   u   ä   ö. //
		// So Vi means a V followed by letter i.                                //
		// Define LV (long vowel) as one of   aa   ee   ii   oo   uu   ää   öö. // 
		// Define a c (consonant) as a character other than a v.                //
		// So cv means a c followed by a v.                                     //
		//////////////////////////////////////////////////////////////////////////

		/**Step 3: cases 
		Search for the longest among the following suffixes in R1, and perform the action indicated 

			- hXn   preceded by X, where X is a V other than u (a/han, e/hen etc) 
			- siin   den   tten   preceded by Vi
			- seen   preceded by LV
			- a   ä   preceded by cv
			- tta   ttä   preceded by e
			- ta   tä   ssa   ssä   sta   stä   lla   llä   lta   ltä   lle   na   nä   ksi   ine 
				- delete 
			- n
				- delete, and if preceded by LV or ie, delete the last vowel*/
		//---------------------------------------------
		void step_3(std::basic_string<xchar>& text)
			{
			//seen followed by LV
			if (is_suffix_in_r1(text,/*seen*/LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_E, UPPER_E, LOWER_N, UPPER_N) &&
				string_util::is_one_of(text[text.length()-5], _XTEXT("aeiouäöAEIOUÄÖ")) &&
				string_util::tolower_western(text[text.length()-5]) == string_util::tolower_western(text[text.length()-6]) )
				{
				text.erase(text.end()-4, text.end() );
				update_r_sections(text);
				m_step_3_successful = true;
				return;
				}
			//suffix followed by Vi
			else if (is_either(text[text.length()-5], LOWER_I, UPPER_I) &&
					(is_suffix_in_r1(text,/*siin*/LOWER_S, UPPER_S, LOWER_I, UPPER_I, LOWER_I, UPPER_I, LOWER_N, UPPER_N) ||
					is_suffix_in_r1(text,/*tten*/LOWER_T, UPPER_T, LOWER_T, UPPER_T, LOWER_E, UPPER_E, LOWER_N, UPPER_N) ) &&
					string_util::is_one_of(text[text.length()-6], _XTEXT("aeiouäöAEIOUÄÖ")) )
				{
				text.erase(text.end()-4, text.end() );
				update_r_sections(text);
				m_step_3_successful = true;
				return;
				}
			//suffix followed by Vi
			else if (is_either(text[text.length()-4], LOWER_I, UPPER_I) &&
					is_suffix_in_r1(text,/*den*/LOWER_D, UPPER_D, LOWER_E, UPPER_E, LOWER_N, UPPER_N) &&
					string_util::is_one_of(text[text.length()-5], _XTEXT("aeiouäöAEIOUÄÖ")) )
				{
				text.erase(text.end()-3, text.end() );
				update_r_sections(text);
				m_step_3_successful = true;
				return;
				}
			else if ((is_suffix_in_r1(text,/*tta*/LOWER_T, UPPER_T, LOWER_T, UPPER_T, LOWER_A, UPPER_A) ||
				is_suffix_in_r1(text,/*ttä*/LOWER_T, UPPER_T, LOWER_T, UPPER_T, LOWER_A_UMLAUTS, UPPER_A_UMLAUTS)) &&
				is_either(text[text.length()-4], LOWER_E, UPPER_E) )
				{
				text.erase(text.end()-3, text.end() );
				update_r_sections(text);
				m_step_3_successful = true;
				return;
				}
			//ends if VHVN
			else if (
				(is_suffix_in_r1(text,/*han*/LOWER_H, UPPER_H, LOWER_A, UPPER_A, LOWER_N, UPPER_N) ||
				is_suffix_in_r1(text,/*hen*/LOWER_H, UPPER_H, LOWER_E, UPPER_E, LOWER_N, UPPER_N) ||
				is_suffix_in_r1(text,/*hin*/LOWER_H, UPPER_H, LOWER_I, UPPER_I, LOWER_N, UPPER_N) ||
				is_suffix_in_r1(text,/*hon*/LOWER_H, UPPER_H, LOWER_O, UPPER_O, LOWER_N, UPPER_N) ||
				is_suffix_in_r1(text,/*hän*/LOWER_H, UPPER_H, LOWER_A_UMLAUTS, UPPER_A_UMLAUTS, LOWER_N, UPPER_N) ||
				is_suffix_in_r1(text,/*hön*/LOWER_H, UPPER_H, LOWER_O_UMLAUTS, UPPER_O_UMLAUTS, LOWER_N, UPPER_N) ) )
				{
				if (string_util::tolower_western(text[text.length()-2]) == string_util::tolower_western(text[text.length()-4]) )
					{
					text.erase(text.end()-3, text.end() );
					update_r_sections(text);
					m_step_3_successful = true;
					}
				return;
				}
			else if (delete_if_is_in_r1(text,/*ssa*/LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_A, UPPER_A, false) ||
				delete_if_is_in_r1(text,/*ssä*/LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_A_UMLAUTS, UPPER_A_UMLAUTS, false) ||
				delete_if_is_in_r1(text,/*sta*/LOWER_S, UPPER_S, LOWER_T, UPPER_T, LOWER_A, UPPER_A, false) ||
				delete_if_is_in_r1(text,/*stä*/LOWER_S, UPPER_S, LOWER_T, UPPER_T, LOWER_A_UMLAUTS, UPPER_A_UMLAUTS, false) ||
				delete_if_is_in_r1(text,/*lla*/LOWER_L, UPPER_L, LOWER_L, UPPER_L, LOWER_A, UPPER_A, false) ||
				delete_if_is_in_r1(text,/*llä*/LOWER_L, UPPER_L, LOWER_L, UPPER_L, LOWER_A_UMLAUTS, UPPER_A_UMLAUTS, false) ||
				delete_if_is_in_r1(text,/*lta*/LOWER_L, UPPER_L, LOWER_T, UPPER_T, LOWER_A, UPPER_A, false) ||
				delete_if_is_in_r1(text,/*ltä*/LOWER_L, UPPER_L, LOWER_T, UPPER_T, LOWER_A_UMLAUTS, UPPER_A_UMLAUTS, false) ||
				delete_if_is_in_r1(text,/*lle*/LOWER_L, UPPER_L, LOWER_L, UPPER_L, LOWER_E, UPPER_E, false) ||
				delete_if_is_in_r1(text,/*ksi*/LOWER_K, UPPER_K, LOWER_S, UPPER_S, LOWER_I, UPPER_I, false) ||
				delete_if_is_in_r1(text,/*ine*/LOWER_I, UPPER_I, LOWER_N, UPPER_N, LOWER_E, UPPER_E, false) ||
				delete_if_is_in_r1(text,/*na*/LOWER_N, UPPER_N, LOWER_A, UPPER_A, false) ||
				delete_if_is_in_r1(text,/*nä*/LOWER_N, UPPER_N, LOWER_A_UMLAUTS, UPPER_A_UMLAUTS, false) )
				{
				m_step_3_successful = true;
				return;
				}
			else if (delete_if_is_in_r1(text,/*ta*/LOWER_T, UPPER_T, LOWER_A, UPPER_A, false) ||
				delete_if_is_in_r1(text,/*tä*/LOWER_T, UPPER_T, LOWER_A_UMLAUTS, UPPER_A_UMLAUTS, false) )
				{
				m_step_3_successful = true;
				return;
				}
			//suffix followed by cv
			else if ((is_suffix_in_r1(text, LOWER_A, UPPER_A) || is_suffix_in_r1(text, LOWER_A_UMLAUTS, UPPER_A_UMLAUTS) )&&
					!string_util::is_one_of(text[text.length()-3], FINNISH_VOWELS) &&
					string_util::is_one_of(text[text.length()-2], FINNISH_VOWELS) )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				m_step_3_successful = true;
				return;
				}
			//suffix followed by LV or ie
			else if (is_suffix_in_r1(text, LOWER_N, UPPER_N) )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				if ((string_util::is_one_of(text[text.length()-1], _XTEXT("aeiouäöAEIOUÄÖ")) &&
					string_util::tolower_western(text[text.length()-1]) == string_util::tolower_western(text[text.length()-2])) ||
					is_suffix_in_r1(text,/*ie*/LOWER_I, UPPER_I, LOWER_E, UPPER_E) ) 
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					}
				m_step_3_successful = true;
				return;
				}
			}
		/**Step 4: other endings 
		Search for the longest among the following suffixes in R2, and perform the action indicated 

			- mpi   mpa   mpä   mmi   mma   mmä 
				- delete if not preceded by po 
			- impi   impa   impä   immi   imma   immä   eja   ejä 
				- delete*/
		//---------------------------------------------
		void step_4(std::basic_string<xchar>& text)
			{
			if (delete_if_is_in_r2(text,/*impi*/LOWER_I, UPPER_I, LOWER_M, UPPER_M, LOWER_P, UPPER_P, LOWER_I, UPPER_I, false) ||
				delete_if_is_in_r2(text,/*impa*/LOWER_I, UPPER_I, LOWER_M, UPPER_M, LOWER_P, UPPER_P, LOWER_A, UPPER_A, false) ||
				delete_if_is_in_r2(text,/*impä*/LOWER_I, UPPER_I, LOWER_M, UPPER_M, LOWER_P, UPPER_P, LOWER_A_UMLAUTS, UPPER_A_UMLAUTS, false) ||
				delete_if_is_in_r2(text,/*immi*/LOWER_I, UPPER_I, LOWER_M, UPPER_M, LOWER_M, UPPER_M, LOWER_I, UPPER_I, false) ||
				delete_if_is_in_r2(text,/*imma*/LOWER_I, UPPER_I, LOWER_M, UPPER_M, LOWER_M, UPPER_M, LOWER_A, UPPER_A, false) ||
				delete_if_is_in_r2(text,/*immä*/LOWER_I, UPPER_I, LOWER_M, UPPER_M, LOWER_M, UPPER_M, LOWER_A_UMLAUTS, UPPER_A_UMLAUTS, false) ||
				delete_if_is_in_r2(text,/*eja*/LOWER_E, UPPER_E, LOWER_J, UPPER_J, LOWER_A, UPPER_A, false) ||
				delete_if_is_in_r2(text,/*ejä*/LOWER_E, UPPER_E, LOWER_J, UPPER_J, LOWER_A_UMLAUTS, UPPER_A_UMLAUTS, false) )
				{
				return;
				}
			else if (text.length() >= 5 &&
					(is_suffix_in_r2(text,/*mpi*/LOWER_M, UPPER_M, LOWER_P, UPPER_P, LOWER_I, UPPER_I) ||
					is_suffix_in_r2(text,/*mpa*/LOWER_M, UPPER_M, LOWER_P, UPPER_P, LOWER_A, UPPER_A) ||
					is_suffix_in_r2(text,/*mpä*/LOWER_M, UPPER_M, LOWER_P, UPPER_P, LOWER_A_UMLAUTS, UPPER_A_UMLAUTS) ||
					is_suffix_in_r2(text,/*mmi*/LOWER_M, UPPER_M, LOWER_M, UPPER_M, LOWER_I, UPPER_I) ||
					is_suffix_in_r2(text,/*mma*/LOWER_M, UPPER_M, LOWER_M, UPPER_M, LOWER_A, UPPER_A) ||
					is_suffix_in_r2(text,/*mmä*/LOWER_M, UPPER_M, LOWER_M, UPPER_M, LOWER_A_UMLAUTS, UPPER_A_UMLAUTS) ) )
				{
				if (!(is_either(text[text.length()-5], LOWER_P, UPPER_P) &&
					is_either(text[text.length()-4], LOWER_O, UPPER_O) ) )
					{
					text.erase(text.end()-3, text.end() );
					update_r_sections(text);
					}
				}
			return;
			}
		/**Step 5: plurals 
		If an ending was removed in step 3, delete a final i or j if in R1;
		otherwise, if an ending was not removed in step 3,
		then delete the final t in R1 if it follows a vowel, and, if a t is removed,
		then delete a final mma or imma in R2, unless the mma is preceded by po.*/
		//---------------------------------------------
		void step_5(std::basic_string<xchar>& text)
			{
			//if step 3 was successful in removing a suffix
			if (m_step_3_successful)
				{
				if (delete_if_is_in_r1(text, LOWER_I, UPPER_I) ||
					delete_if_is_in_r1(text, LOWER_J, UPPER_J) )
					{
					//NOOP
					}
				}
			else
				{
				if (string_util::is_one_of(text[text.length()-2], FINNISH_VOWELS) )
					{
					if (delete_if_is_in_r1(text, LOWER_T, UPPER_T) )
						{
						if (!delete_if_is_in_r2(text,/*imma*/LOWER_I, UPPER_I,LOWER_M, UPPER_M, LOWER_M, UPPER_M, LOWER_A, UPPER_A) )
							{
							if (is_suffix_in_r2(text,/*mma*/LOWER_M, UPPER_M, LOWER_M, UPPER_M, LOWER_A, UPPER_A) &&
								//isn't proceeded by "po"
								!(is_either(text[text.length()-5], LOWER_P, UPPER_P) &&
								is_either(text[text.length()-4], LOWER_O, UPPER_O) ) )
								{
								text.erase(text.end()-3, text.end() );
								update_r_sections(text);
								}
							}
						}
					}
				}
			}
		/**Step 6: tidying up 
		Perform turn steps (a), (b), (c), (d), restricting all tests to the R1 region.*/
		//---------------------------------------------
		void step_6(std::basic_string<xchar>& text)
			{
			step_6a(text);
			step_6b(text);
			step_6c(text);
			step_6d(text);
			step_6e(text);
			}
		///Step a) If R1 ends with LV then delete the last letter
		//---------------------------------------------
		void step_6a(std::basic_string<xchar>& text)
			{
			if (m_r1 <= text.length()-2 &&
				string_util::is_one_of(text[text.length()-1], _XTEXT("aeiouäöAEIOUÄÖ")) &&
				string_util::tolower_western(text[text.length()-1]) == string_util::tolower_western(text[text.length()-2]))
				{
				text.erase(text.end()-1);
				update_r_sections(text);
				}
			}
		///Step b) If R1 ends with cX, c a consonant and X one of   a   ä   e   i, then delete the last letter
		//---------------------------------------------
		void step_6b(std::basic_string<xchar>& text)
			{
			if (m_r1 <= text.length()-2 &&
				!string_util::is_one_of(text[text.length()-2], FINNISH_VOWELS) &&
				string_util::is_one_of(text[text.length()-1], _XTEXT("aeiäAEIÄ")) )
				{
				text.erase(text.end()-1);
				update_r_sections(text);
				}
			}
		///Step c) If R1 ends with oj or uj then delete the last letter
		//---------------------------------------------
		void step_6c(std::basic_string<xchar>& text)
			{
			if (is_suffix_in_r1(text,/*oj*/LOWER_O, UPPER_O, LOWER_J, UPPER_J) ||
				is_suffix_in_r1(text,/*uj*/LOWER_U, UPPER_U, LOWER_J, UPPER_J) )
				{
				text.erase(text.end()-1);
				update_r_sections(text);
				}
			}
		///Step d) If R1 ends with jo then delete the last letter 
		//---------------------------------------------
		void step_6d(std::basic_string<xchar>& text)
			{
			if (is_suffix_in_r1(text,/*jo*/LOWER_J, UPPER_J, LOWER_O, UPPER_O) )
				{
				text.erase(text.end()-1);
				update_r_sections(text);
				}
			}
		/**Do step (e), which is not restricted to R1.
		Step e) If the word ends with a double consonant followed by zero or more vowels,
		then remove the last consonant (so eläkk -> eläk, aatonaatto -> aatonaato)*/
		//---------------------------------------------
		void step_6e(std::basic_string<xchar>& text)
			{
			//find the last consonant
			size_t index = text.find_last_not_of(FINNISH_VOWELS);
			if (index == std::basic_string<xchar>::npos ||
				index < 1)
				{
				return;
				}
			if (string_util::tolower_western(text[index]) == string_util::tolower_western(text[index-1]))
				{
				text.erase(text.begin()+(index) );
				update_r_sections(text);
				}
			}
		//internal data specific to Finnish stemmer
		bool m_step_3_successful;
		};
	}

#endif //__FINNISH_STEM_H__
