/***************************************************************************
                          swedish_stem.h  -  description
                             -------------------
    begin                : Sat May 16 2004
    copyright            : (C) 2004 by Blake Madden
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the BSD License.                                *
 *                                                                         *
 ***************************************************************************/

#ifndef __SWEDISH_STEM_H__
#define __SWEDISH_STEM_H__

#include "semantic/stem/stemming.h"

namespace stemming
	{
	/**The Swedish alphabet includes the following additional letters, 
		-�   �   � 
	The following letters are vowels: 
		-a   e   i   o   u   y   �   �   � 
	R2 is not used: R1 is defined in the same way as in the German stemmer.*/
	//------------------------------------------------------
	class swedish_stem : stem<xchar>
		{
	public:
		//---------------------------------------------
        ///@param text: string to stem
		void operator()(std::basic_string<xchar>& text)
			{
			if (text.length() < 3)
				{
				return;
				}

			//reset internal data
			m_r1 = m_r2 = m_rv =0;

			trim_western_punctuation(text);

			//see where the R1 section begins
			//R1 is the first consonant after the first vowel
			find_r1(text, _XTEXT("aeiouy���AEIOUY���"));
			if (m_r1 == static_cast<int>(text.length() ) )
				{
				return;
				}

			//R1 must have at least 3 characters in front of it
			if (m_r1 < 3)
				{
				m_r1 = 3;	
				}
			//swedish does not use R2

            step_1(text);
			step_2(text);
			step_3(text);
			}
	private:
		/**Search for the longest among the following suffixes in R1, and perform the action indicated. 
			-#a   arna   erna   heterna   orna   ad   e   ade   ande   arne   are   aste   en   anden   aren   heten   ern   ar   er   heter   or   as   arnas   ernas   ornas   es   ades   andes   ens   arens   hetens   erns   at   andet   het   ast 
				-delete 

			-#s 
				-delete if preceded by a valid s-ending 

		(Of course the letter of the valid s-ending is not necessarily in R1)*/
		//---------------------------------------------
		void step_1(std::basic_string<xchar>& text)
			{
			if (delete_if_is_in_r1(text,/*heterna*/LOWER_H, UPPER_H, LOWER_E, UPPER_E, LOWER_T, UPPER_T, LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_N, UPPER_N, LOWER_A, UPPER_A, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*hetens*/LOWER_H, UPPER_H, LOWER_E, UPPER_E, LOWER_T, UPPER_T, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*arna*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_N, UPPER_N, LOWER_A, UPPER_A, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*erna*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_N, UPPER_N, LOWER_A, UPPER_A, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*orna*/LOWER_O, UPPER_O, LOWER_R, UPPER_R, LOWER_N, UPPER_N, LOWER_A, UPPER_A, false) )
				{
				return;
				}	
			else if (delete_if_is_in_r1(text,/*ande*/LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_D, UPPER_D, LOWER_E, UPPER_E, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*arne*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_N, UPPER_N, LOWER_E, UPPER_E, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*aste*/LOWER_A, UPPER_A, LOWER_S, UPPER_S, LOWER_T, UPPER_T, LOWER_E, UPPER_E, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*anden*/LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_D, UPPER_D, LOWER_E, UPPER_E, LOWER_N, UPPER_N, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*heten*/LOWER_H, UPPER_H, LOWER_E, UPPER_E, LOWER_T, UPPER_T, LOWER_E, UPPER_E, LOWER_N, UPPER_N, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*heter*/LOWER_H, UPPER_H, LOWER_E, UPPER_E, LOWER_T, UPPER_T, LOWER_E, UPPER_E, LOWER_R, UPPER_R, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*arnas*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_N, UPPER_N, LOWER_A, UPPER_A, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ernas*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_N, UPPER_N, LOWER_A, UPPER_A, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ornas*/LOWER_O, UPPER_O, LOWER_R, UPPER_R, LOWER_N, UPPER_N, LOWER_A, UPPER_A, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*arens*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*andet*/LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_D, UPPER_D, LOWER_E, UPPER_E, LOWER_T, UPPER_T, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*andes*/LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_D, UPPER_D, LOWER_E, UPPER_E, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*aren*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_N, UPPER_N, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text, /*erns*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_N, UPPER_N, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ades*/LOWER_A, UPPER_A, LOWER_D, UPPER_D, LOWER_E, UPPER_E, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*are*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_E, UPPER_E, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ade*/LOWER_A, UPPER_A, LOWER_D, UPPER_D, LOWER_E, UPPER_E, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*het*/LOWER_H, UPPER_H, LOWER_E, UPPER_E, LOWER_T, UPPER_T, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ast*/LOWER_A, UPPER_A, LOWER_S, UPPER_S, LOWER_T, UPPER_T, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text, /*ens*/LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ern*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_N, UPPER_N, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*at*/LOWER_A, UPPER_A, LOWER_T, UPPER_T, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*es*/LOWER_E, UPPER_E, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*or*/LOWER_O, UPPER_O, LOWER_R, UPPER_R, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ad*/LOWER_A, UPPER_A, LOWER_D, UPPER_D, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*as*/LOWER_A, UPPER_A, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ar*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*er*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*en*/LOWER_E, UPPER_E, LOWER_N, UPPER_N, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text, LOWER_A, UPPER_A, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text, LOWER_E, UPPER_E, false) )
				{
				return;
				}
			/**Define a valid s-ending as one of 
				-b c d f g h j k l m n o p r t v y*/
			else if (is_suffix_in_r1(text, LOWER_S, UPPER_S) )
				{
				if (string_util::is_one_of(text[text.length()-2],
					_XTEXT("bcdfghjklmnoprtvyBCDFGHJKLMNOPRTVY")) )
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					}
				return;
				}
			}
		/**Search for one of the following suffixes in R1, and if found delete the last letter. 

			-dd   gd   nn   dt   gt   kt   tt 

		(For example, friskt -> frisk, fr�knarnn -> fr�knarn)*/
		//---------------------------------------------
		void step_2(std::basic_string<xchar>& text)
			{			
			if (is_suffix_in_r1(text,/*dd*/LOWER_D, UPPER_D, LOWER_D, UPPER_D) )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				}
			else if (is_suffix_in_r1(text,/*gd*/LOWER_G, UPPER_G, LOWER_D, UPPER_D) )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				}
			else if (is_suffix_in_r1(text,/*nn*/LOWER_N, UPPER_N, LOWER_N, UPPER_N) )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				}
			else if (is_suffix_in_r1(text,/*dt*/LOWER_D, UPPER_D, LOWER_T, UPPER_T) )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				}
			else if (is_suffix_in_r1(text,/*gt*/LOWER_G, UPPER_G, LOWER_T, UPPER_T) )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				}
			else if (is_suffix_in_r1(text,/*kt*/LOWER_K, UPPER_K, LOWER_T, UPPER_T) )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				}
			else if (is_suffix_in_r1(text,/*tt*/LOWER_T, UPPER_T, LOWER_T, UPPER_T) )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				}
			}
		/**Search for the longest among the following suffixes in R1, and perform the action indicated. 
			-lig   ig   els 
				-delete 

			-l�st 
				-replace with l�s 

			-fullt 
				-replace with full*/
		//---------------------------------------------
		void step_3(std::basic_string<xchar>& text)
			{
			if (is_suffix_in_r1(text,/*fullt*/LOWER_F, UPPER_F, LOWER_U, UPPER_U, LOWER_L, UPPER_L, LOWER_L, UPPER_L, LOWER_T, UPPER_T) )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				}
			else if (is_suffix_in_r1(text,/*l�st*/LOWER_L, UPPER_L, LOWER_O_UMLAUTS, UPPER_O_UMLAUTS, LOWER_S, UPPER_S, LOWER_T, UPPER_T) )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				}
			else if (delete_if_is_in_r1(text,/*lig*/LOWER_L, UPPER_L, LOWER_I, UPPER_I, LOWER_G, UPPER_G, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*els*/LOWER_E, UPPER_E, LOWER_L, UPPER_L, LOWER_S, UPPER_S, false) )
				{
				return;
				}
			else if (delete_if_is_in_r1(text,/*ig*/LOWER_I, UPPER_I, LOWER_G, UPPER_G, false) )
				{
				return;
				}
			}
		};
	}

#endif //__SWEDISH_STEM_H__
