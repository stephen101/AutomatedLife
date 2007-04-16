/***************************************************************************
                          portuguese_stem.h  -  description
                             -------------------
    begin                : Sat May 17 2004
    copyright            : (C) 2004 by Blake Madden
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the BSD License.                                *
 *                                                                         *
 ***************************************************************************/

#ifndef __PORTUGUESE_STEM_H__
#define __PORTUGUESE_STEM_H__

#include "semantic/stem/stemming.h"

namespace stemming
	{
	/**Letters in Portuguese include the following accented forms,

		-á é í ó ú â ê ô ç ã õ ü ñ

	The following letters are vowels: 
		-a e i o u á é í ó ú â ê ô

	And the two nasalised vowel forms
	
		-ã õ

	should be treated as a vowel followed by a consonant. 

	ã and õ are therefore replaced by a~ and o~ in the word, where ~ is a separate character to be treated as a consonant.
	And then R2 and RV have the same definition as in the Spanish stemmer.*/
	//------------------------------------------------------
	class portuguese_stem : stem<xchar>
		{
	public:
		portuguese_stem() : m_step1_step2_altered(false), m_altered_suffix_index(0)
			{}
		//---------------------------------------------
        ///@param text: string to stem
		void operator()(std::basic_string<xchar>& text)
			{
			if (text.length() < 3)
				{
				return;
				}
			trim_western_punctuation(text);

			//reset internal data
			m_altered_suffix_index = 0;
			m_step1_step2_altered = false;
			m_r1 = m_r2 = m_rv =0;

			string_util::replace_all(text, _XTEXT("ã"), _XTEXT("a~"));
			string_util::replace_all(text, _XTEXT("Ã"), _XTEXT("A~"));
			string_util::replace_all(text, _XTEXT("õ"), _XTEXT("o~"));
			string_util::replace_all(text, _XTEXT("Õ"), _XTEXT("O~"));

			find_r1(text, PORTUGUESE_VOWELS);
			find_r2(text, PORTUGUESE_VOWELS);
			find_spanish_rv(text, PORTUGUESE_VOWELS);

			step_1(text);
			//intermediate steps handled by step 1
			if (!m_step1_step2_altered)
				{
				step_4(text);
				}
			step_5(text);

			//Turn a~, o~ back into ã, õ 
			string_util::replace_all(text, _XTEXT("a~"), _XTEXT("ã"));
			string_util::replace_all(text, _XTEXT("A~"), _XTEXT("Ã"));
			string_util::replace_all(text, _XTEXT("o~"), _XTEXT("õ"));
			string_util::replace_all(text, _XTEXT("O~"), _XTEXT("Õ"));
			}
	private:
		/**Search for the longest among the following suffixes, and perform the action indicated. 

			-eza   ezas   ico   ica   icos   icas   ismo   ismos   ável   ível   ista   istas
             oso   osa   osos   osas   amento   amentos   imento   imentos   adora   ador   aça~o
             adoras   adores   aço~es   ante   antes   ância 
				-delete if in R2 

			-logía logías 
				-replace with log if in R2 

			-ución uciones 
				-replace with u if in R2 

			-ência ências 
				-replace with ente if in R2 

			-amente 
				-delete if in R1 
				-if preceded by iv, delete if in R2 (and if further preceded by at, delete if in R2), otherwise, 
				-if preceded by os, ic or ad, delete if in R2 

			-mente 
				-delete if in R2 
				-if preceded by ante, avel or ível, delete if in R2 

			-idade idades 
				-delete if in R2 
				-if preceded by abil, ic or iv, delete if in R2 

			-iva ivo ivas ivos 
				-delete if in R2 
				-if preceded by at, delete if in R2 

			-ira iras 
				-replace with ir if in RV and preceded by e*/
		//---------------------------------------------
		void step_1(std::basic_string<xchar>& text)
			{
			size_t original_length = text.length();
			if (is_suffix_in_r2(text,/*uciones*/LOWER_U, UPPER_U, LOWER_C, UPPER_C, LOWER_I, UPPER_I, LOWER_O, UPPER_O, LOWER_N, UPPER_N, LOWER_E, UPPER_E, LOWER_S, UPPER_S) )
				{
				text.erase(text.end()-6, text.end() );
				m_altered_suffix_index = text.length()-1;
				update_r_sections(text);
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*amentos*/LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_O, UPPER_O, LOWER_S, UPPER_S) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*imentos*/LOWER_I, UPPER_I, LOWER_M, UPPER_M, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_O, UPPER_O, LOWER_S, UPPER_S) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*amento*/LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_O, UPPER_O) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*imento*/LOWER_I, UPPER_I, LOWER_M, UPPER_M, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_O, UPPER_O) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*adoras*/LOWER_A, UPPER_A, LOWER_D, UPPER_D, LOWER_O, UPPER_O, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_S, UPPER_S) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*adores*/LOWER_A, UPPER_A, LOWER_D, UPPER_D, LOWER_O, UPPER_O, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_S, UPPER_S) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*aço~es*/LOWER_A, UPPER_A, LOWER_C_CEDILLA, UPPER_C_CEDILLA, LOWER_O, UPPER_O, TILDE, TILDE, LOWER_E, UPPER_E, LOWER_S, UPPER_S) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (is_suffix_in_r2(text,/*ências*/LOWER_E_CIRCUMFLEX, UPPER_E_CIRCUMFLEX, LOWER_N, UPPER_N, LOWER_C, UPPER_C, LOWER_I, UPPER_I, LOWER_A, UPPER_A, LOWER_S, UPPER_S) )
				{
				text.replace(text.end()-6, text.end(), _XTEXT("ente"));
				m_altered_suffix_index = text.length()-4;
				update_r_sections(text);
				//NOOP (fall through to branching statement)
				}
			else if (is_suffix_in_r2(text,/*logías*/LOWER_L, UPPER_L, LOWER_O, UPPER_O, LOWER_G, UPPER_G, LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_A, UPPER_A, LOWER_S, UPPER_S) )
				{
				text.erase(text.end()-3, text.end() );
				m_altered_suffix_index = text.length()-3;
				update_r_sections(text);
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r1(text,/*amente*/LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_E, UPPER_E) )
				{
				if (delete_if_is_in_r2(text,/*iv*/LOWER_I, UPPER_I, LOWER_V, UPPER_V, false) )
					{
					delete_if_is_in_r2(text,/*at*/LOWER_A, UPPER_A, LOWER_T, UPPER_T, false);
					}
				else
					{
					if (delete_if_is_in_r2(text,/*os*/LOWER_O, UPPER_O, LOWER_S, UPPER_S) ||
						delete_if_is_in_r2(text,/*ic*/LOWER_I, UPPER_I, LOWER_C, UPPER_C) ||
						delete_if_is_in_r2(text,/*ad*/LOWER_A, UPPER_A, LOWER_D, UPPER_D) )
						{
						//NOOP (fall through to branching statement)
						}
					}
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*idades*/LOWER_I, UPPER_I, LOWER_D, UPPER_D, LOWER_A, UPPER_A, LOWER_D, UPPER_D, LOWER_E, UPPER_E, LOWER_S, UPPER_S) )
				{
				if (delete_if_is_in_r2(text,/*abil*/LOWER_A, UPPER_A, LOWER_B, UPPER_B, LOWER_I, UPPER_I, LOWER_L, UPPER_L) ||
					delete_if_is_in_r2(text,/*ic*/LOWER_I, UPPER_I, LOWER_C, UPPER_C) ||
					delete_if_is_in_r2(text,/*iv*/LOWER_I, UPPER_I, LOWER_V, UPPER_V) )
					{
					//NOOP (fall through to branching statement)
					}
				//NOOP (fall through to branching statement)
				}	
			else if (is_suffix_in_r2(text,/*logía*/LOWER_L, UPPER_L, LOWER_O, UPPER_O, LOWER_G, UPPER_G, LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_A, UPPER_A) )
				{
				text.erase(text.end()-2, text.end() );
				m_altered_suffix_index = text.length()-3;
				update_r_sections(text);
				//NOOP (fall through to branching statement)
				}	
			else if (is_suffix_in_r2(text,/*ución*/LOWER_U, UPPER_U, LOWER_C, UPPER_C, LOWER_I, UPPER_I, LOWER_O_ACUTE, UPPER_O_ACUTE, LOWER_N, UPPER_N) )
				{
				text.erase(text.end()-4, text.end() );
				m_altered_suffix_index = text.length()-1;
				update_r_sections(text);
				//NOOP (fall through to branching statement)
				}
			else if (is_suffix_in_r2(text,/*ência*/LOWER_E_CIRCUMFLEX, UPPER_E_CIRCUMFLEX, LOWER_N, UPPER_N, LOWER_C, UPPER_C, LOWER_I, UPPER_I, LOWER_A, UPPER_A) )
				{
				///make more case sensitive
				text.replace(text.end()-5, text.end(), _XTEXT("ente"));
				m_altered_suffix_index = text.length()-4;
				update_r_sections(text);
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*mente*/LOWER_M, UPPER_M, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_E, UPPER_E) )
				{
				if (delete_if_is_in_r2(text,/*ante*/LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_E, UPPER_E) ||
                    delete_if_is_in_r2(text,/*avel*/LOWER_A, UPPER_A, LOWER_V, UPPER_V, LOWER_E, UPPER_E, LOWER_L, UPPER_L) ||
					delete_if_is_in_r2(text,/*ível*/LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_V, UPPER_V, LOWER_E, UPPER_E, LOWER_L, UPPER_L) )
					{
					//NOOP (fall through to branching statement)
					}
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*idade*/LOWER_I, UPPER_I, LOWER_D, UPPER_D, LOWER_A, UPPER_A, LOWER_D, UPPER_D, LOWER_E, UPPER_E) )
				{
				if (delete_if_is_in_r2(text,/*abil*/LOWER_A, UPPER_A, LOWER_B, UPPER_B, LOWER_I, UPPER_I, LOWER_L, UPPER_L) ||
					delete_if_is_in_r2(text,/*ic*/LOWER_I, UPPER_I, LOWER_C, UPPER_C) ||
					delete_if_is_in_r2(text,/*iv*/LOWER_I, UPPER_I, LOWER_V, UPPER_V) )
					{
					//NOOP (fall through to branching statement)
					}
				//NOOP (fall through to branching statement)
				}
			else if (is_suffix(text,/*eiras*/LOWER_E, UPPER_E, LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_S, UPPER_S) )
				{
				if (m_rv <= static_cast<int>(text.length()-4) )
					{
					text.erase(text.end()-2, text.end() );
					m_altered_suffix_index = text.length()-3;
					update_r_sections(text);
					}
				//NOOP (fall through to branching statement)
				}
            else if (delete_if_is_in_r2(text,/*antes*/LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_E, UPPER_E, LOWER_S, UPPER_S) )
				{
				//NOOP (fall through to branching statement)
				}
            else if (delete_if_is_in_r2(text,/*ância*/LOWER_A_CIRCUMFLEX, UPPER_A_CIRCUMFLEX, LOWER_N, UPPER_N, LOWER_C, UPPER_C, LOWER_I, UPPER_I, LOWER_A, UPPER_A) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*ismos*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*istas*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_T, UPPER_T, LOWER_A, UPPER_A, LOWER_S, UPPER_S) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*adora*/LOWER_A, UPPER_A, LOWER_D, UPPER_D, LOWER_O, UPPER_O, LOWER_R, UPPER_R, LOWER_A, UPPER_A) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*aça~o*/LOWER_A, UPPER_A, LOWER_C_CEDILLA, UPPER_C_CEDILLA, LOWER_A, UPPER_A, TILDE, TILDE, LOWER_O, UPPER_O) )
				{
				//NOOP (fall through to branching statement)
				}
            else if (delete_if_is_in_r2(text,/*ante*/LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_E, UPPER_E) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*ezas*/LOWER_E, UPPER_E, LOWER_Z, UPPER_Z, LOWER_A, UPPER_A, LOWER_S, UPPER_S) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*icos*/LOWER_I, UPPER_I, LOWER_C, UPPER_C, LOWER_O, UPPER_O, LOWER_S, UPPER_S) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*icas*/LOWER_I, UPPER_I, LOWER_C, UPPER_C, LOWER_A, UPPER_A, LOWER_S, UPPER_S) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*ismo*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_M, UPPER_M, LOWER_O, UPPER_O) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*ável*/LOWER_A_ACUTE, UPPER_A_ACUTE, LOWER_V, UPPER_V, LOWER_E, UPPER_E, LOWER_L, UPPER_L) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*ível*/LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_V, UPPER_V, LOWER_E, UPPER_E, LOWER_L, UPPER_L) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*ista*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_T, UPPER_T, LOWER_A, UPPER_A) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*osos*/LOWER_O, UPPER_O, LOWER_S, UPPER_S, LOWER_O, UPPER_O, LOWER_S, UPPER_S) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*osas*/LOWER_O, UPPER_O, LOWER_S, UPPER_S, LOWER_A, UPPER_A, LOWER_S, UPPER_S) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*ador*/LOWER_A, UPPER_A, LOWER_D, UPPER_D, LOWER_O, UPPER_O, LOWER_R, UPPER_R) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*ivas*/LOWER_I, UPPER_I, LOWER_V, UPPER_V, LOWER_A, UPPER_A, LOWER_S, UPPER_S) )
				{
				if (delete_if_is_in_r2(text,/*at*/LOWER_A, UPPER_A, LOWER_T, UPPER_T) )
					{
					//NOOP (fall through to branching statement)
					}
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*ivos*/LOWER_I, UPPER_I, LOWER_V, UPPER_V, LOWER_O, UPPER_O, LOWER_S, UPPER_S) )
				{
				if (delete_if_is_in_r2(text,/*at*/LOWER_A, UPPER_A, LOWER_T, UPPER_T) )
					{
					//NOOP (fall through to branching statement)
					}
				//NOOP (fall through to branching statement)
				}
			else if (is_suffix(text,/*eira*/LOWER_E, UPPER_E, LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_A, UPPER_A) )
				{
				if (m_rv <= static_cast<int>(text.length()-3) )
					{
					text.erase(text.end()-1, text.end() );
					m_altered_suffix_index = text.length()-3;
					update_r_sections(text);
					}
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*iva*/LOWER_I, UPPER_I, LOWER_V, UPPER_V, LOWER_A, UPPER_A) )
				{
				if (delete_if_is_in_r2(text,/*at*/LOWER_A, UPPER_A, LOWER_T, UPPER_T) )
					{
					//NOOP (fall through to branching statement)
					}
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*ivo*/LOWER_I, UPPER_I, LOWER_V, UPPER_V, LOWER_O, UPPER_O) )
				{
				if (delete_if_is_in_r2(text,/*at*/LOWER_A, UPPER_A, LOWER_T, UPPER_T) )
					{
					//NOOP (fall through to branching statement)
					}
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*eza*/LOWER_E, UPPER_E, LOWER_Z, UPPER_Z, LOWER_A, UPPER_A) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*ico*/LOWER_I, UPPER_I, LOWER_C, UPPER_C, LOWER_O, UPPER_O) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*ica*/LOWER_I, UPPER_I, LOWER_C, UPPER_C, LOWER_A, UPPER_A) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*oso*/LOWER_O, UPPER_O, LOWER_S, UPPER_S, LOWER_O, UPPER_O) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_r2(text,/*osa*/LOWER_O, UPPER_O, LOWER_S, UPPER_S, LOWER_A, UPPER_A) )
				{
				//NOOP (fall through to branching statement)
				}
			//branch to the next appropriate step
			if (original_length == text.length() )
				{
				step_2(text);
				}
			else
				{
				m_step1_step2_altered = true;
				step_3(text);
				}
 			}
		/**Do step 2 if no ending was removed by step 1.
		Search for the longest among the following suffixes in RV, and if found, delete. 

			-ada ida ia aria eria iria ará ara erá era irá ava asse esse isse aste este iste
			ei arei erei irei am iam ariam eriam iriam aram eram iram avam em arem erem irem
			assem essem issem ado ido ando endo indo ara~o era~o ira~o ar er ir as adas idas
			ias arias erias irias arás aras erás eras irás avas es ardes erdes irdes ares eres
			ires asses esses isses astes estes istes is ais eis íeis aríeis eríeis iríeis áreis
			areis éreis ereis íreis ireis ásseis ésseis ísseis áveis ados idos ámos amos íamos
			aríamos eríamos iríamos áramos éramos íramos ávamos emos aremos eremos iremos ássemos
			êssemos íssemos imos armos ermos irmos eu iu ou ira iras

		If the last step to be obeyed - either step 1 or 2 - altered the word, do step 3*/
		//---------------------------------------------
		void step_2(std::basic_string<xchar>& text)
			{
			size_t original_length = text.length();

			if (delete_if_is_in_rv(text,/*aríamos*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*eríamos*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*iríamos*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ássemos*/LOWER_A_ACUTE, UPPER_A_ACUTE, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*êssemos*/LOWER_E_CIRCUMFLEX, UPPER_E_CIRCUMFLEX, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*íssemos*/LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*aríeis*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_E, UPPER_E, LOWER_I, UPPER_I, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*eríeis*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_E, UPPER_E, LOWER_I, UPPER_I, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*iríeis*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_E, UPPER_E, LOWER_I, UPPER_I, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ásseis*/LOWER_A_ACUTE, UPPER_A_ACUTE, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_I, UPPER_I, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ésseis*/LOWER_E_ACUTE, UPPER_E_ACUTE, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_I, UPPER_I, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ísseis*/LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_I, UPPER_I, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*áramos*/LOWER_A_ACUTE, UPPER_A_ACUTE, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*éramos*/LOWER_E_ACUTE, UPPER_E_ACUTE, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*íramos*/LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ávamos*/LOWER_A_ACUTE, UPPER_A_ACUTE, LOWER_V, UPPER_V, LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*aremos*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*eremos*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*iremos*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S ,false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ariam*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_I, UPPER_I, LOWER_A, UPPER_A, LOWER_M, UPPER_M, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*eriam*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_I, UPPER_I, LOWER_A, UPPER_A, LOWER_M, UPPER_M, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*iriam*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_I, UPPER_I, LOWER_A, UPPER_A, LOWER_M, UPPER_M, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*assem*/LOWER_A, UPPER_A, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_M, UPPER_M, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*essem*/LOWER_E, UPPER_E, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_M, UPPER_M, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*issem*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_M, UPPER_M,false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ara~o*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_A, UPPER_A, TILDE, TILDE, LOWER_O, UPPER_O,false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*era~o*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_A, UPPER_A, TILDE, TILDE, LOWER_O, UPPER_O,false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ira~o*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_A, UPPER_A, TILDE, TILDE, LOWER_O, UPPER_O,false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*arias*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_I, UPPER_I, LOWER_A, UPPER_A, LOWER_S, UPPER_S,false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*erias*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_I, UPPER_I, LOWER_A, UPPER_A, LOWER_S, UPPER_S,false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*irias*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_I, UPPER_I, LOWER_A, UPPER_A, LOWER_S, UPPER_S,false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ardes*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_D, UPPER_D, LOWER_E, UPPER_E, LOWER_S, UPPER_S,false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*erdes*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_D, UPPER_D, LOWER_E, UPPER_E, LOWER_S, UPPER_S,false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*irdes*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_D, UPPER_D, LOWER_E, UPPER_E, LOWER_S, UPPER_S,false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*asses*/LOWER_A, UPPER_A, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_S, UPPER_S,false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*esses*/LOWER_E, UPPER_E, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_S, UPPER_S,false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*isses*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_S, UPPER_S,false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*astes*/LOWER_A, UPPER_A, LOWER_S, UPPER_S, LOWER_T, UPPER_T, LOWER_E, UPPER_E, LOWER_S, UPPER_S,false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*estes*/LOWER_E, UPPER_E, LOWER_S, UPPER_S, LOWER_T, UPPER_T, LOWER_E, UPPER_E, LOWER_S, UPPER_S,false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*istes*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_T, UPPER_T, LOWER_E, UPPER_E, LOWER_S, UPPER_S,false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*áreis*/LOWER_A_ACUTE, UPPER_A_ACUTE, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_I, UPPER_I, LOWER_S, UPPER_S,false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*areis*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_I, UPPER_I, LOWER_S, UPPER_S,false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*éreis*/LOWER_E_ACUTE, UPPER_E_ACUTE, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_I, UPPER_I, LOWER_S, UPPER_S,false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ereis*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_I, UPPER_I, LOWER_S, UPPER_S,false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*íreis*/LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_I, UPPER_I, LOWER_S, UPPER_S,false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ireis*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_I, UPPER_I, LOWER_S, UPPER_S,false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*áveis*/LOWER_A_ACUTE, UPPER_A_ACUTE, LOWER_V, UPPER_V, LOWER_E, UPPER_E, LOWER_I, UPPER_I, LOWER_S, UPPER_S,false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*íamos*/LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S,false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*armos*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S,false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ermos*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S,false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*irmos*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S,false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*aria*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_I, UPPER_I, LOWER_A, UPPER_A, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*eria*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_I, UPPER_I, LOWER_A, UPPER_A, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*iria*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_I, UPPER_I, LOWER_A, UPPER_A, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*asse*/LOWER_A, UPPER_A, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_E, UPPER_E, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*esse*/LOWER_E, UPPER_E, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_E, UPPER_E, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*isse*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_E, UPPER_E, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*aste*/LOWER_A, UPPER_A, LOWER_S, UPPER_S, LOWER_T, UPPER_T, LOWER_E, UPPER_E, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*este*/LOWER_E, UPPER_E, LOWER_S, UPPER_S, LOWER_T, UPPER_T, LOWER_E, UPPER_E, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*iste*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_T, UPPER_T, LOWER_E, UPPER_E, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*arei*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_I, UPPER_I, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*erei*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_I, UPPER_I, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*irei*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_I, UPPER_I, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*aram*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_M, UPPER_M, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*eram*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_M, UPPER_M, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*iram*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_M, UPPER_M, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*avam*/LOWER_A, UPPER_A, LOWER_V, UPPER_V, LOWER_A, UPPER_A, LOWER_M, UPPER_M, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*arem*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_M, UPPER_M, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*erem*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_M, UPPER_M, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*irem*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_M, UPPER_M, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ando*/LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_D, UPPER_D, LOWER_O, UPPER_O, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*endo*/LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_D, UPPER_D, LOWER_O, UPPER_O, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*indo*/LOWER_I, UPPER_I, LOWER_N, UPPER_N, LOWER_D, UPPER_D, LOWER_O, UPPER_O, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*adas*/LOWER_A, UPPER_A, LOWER_D, UPPER_D, LOWER_A, UPPER_A, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*idas*/LOWER_I, UPPER_I, LOWER_D, UPPER_D, LOWER_A, UPPER_A, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*arás*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_A_ACUTE, UPPER_A_ACUTE, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*aras*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*erás*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_A_ACUTE, UPPER_A_ACUTE, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*eras*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*irás*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_A_ACUTE, UPPER_A_ACUTE, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*avas*/LOWER_A, UPPER_A, LOWER_V, UPPER_V, LOWER_A, UPPER_A, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ares*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*eres*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ires*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*íeis*/LOWER_I_ACUTE, UPPER_I_ACUTE, LOWER_E, UPPER_E, LOWER_I, UPPER_I, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ados*/LOWER_A, UPPER_A, LOWER_D, UPPER_D, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*idos*/LOWER_I, UPPER_I, LOWER_D, UPPER_D, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ámos*/LOWER_A_ACUTE, UPPER_A_ACUTE, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*amos*/LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*emos*/LOWER_E, UPPER_E, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*imos*/LOWER_I, UPPER_I, LOWER_M, UPPER_M, LOWER_O, UPPER_O, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*iras*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ada*/LOWER_A, UPPER_A, LOWER_D, UPPER_D, LOWER_A, UPPER_A, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ida*/LOWER_I, UPPER_I, LOWER_D, UPPER_D, LOWER_A, UPPER_A, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ará*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_A_ACUTE, UPPER_A_ACUTE, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ara*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_A, UPPER_A, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*erá*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_A_ACUTE, UPPER_A_ACUTE, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*era*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_A, UPPER_A, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*irá*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_A_ACUTE, UPPER_A_ACUTE, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ava*/LOWER_A, UPPER_A, LOWER_V, UPPER_V, LOWER_A, UPPER_A, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*iam*/LOWER_I, UPPER_I, LOWER_A, UPPER_A, LOWER_M, UPPER_M, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ado*/LOWER_A, UPPER_A, LOWER_D, UPPER_D, LOWER_O, UPPER_O, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ido*/LOWER_I, UPPER_I, LOWER_D, UPPER_D, LOWER_O, UPPER_O, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ias*/LOWER_I, UPPER_I, LOWER_A, UPPER_A, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ais*/LOWER_A, UPPER_A, LOWER_I, UPPER_I, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*eis*/LOWER_E, UPPER_E, LOWER_I, UPPER_I, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ira*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_A, UPPER_A, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ia*/LOWER_I, UPPER_I, LOWER_A, UPPER_A, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ei*/LOWER_E, UPPER_E, LOWER_I, UPPER_I, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*am*/LOWER_A, UPPER_A, LOWER_M, UPPER_M, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*em*/LOWER_E, UPPER_E, LOWER_M, UPPER_M, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ar*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*er*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ir*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*as*/LOWER_A, UPPER_A, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*es*/LOWER_E, UPPER_E, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*is*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*eu*/LOWER_E, UPPER_E, LOWER_U, UPPER_U, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*iu*/LOWER_I, UPPER_I, LOWER_U, UPPER_U, false) )
				{
				//NOOP (fall through to branching statement)
				}
			else if (delete_if_is_in_rv(text,/*ou*/LOWER_O, UPPER_O, LOWER_U, UPPER_U, false) )
				{
				//NOOP (fall through to branching statement)
				}

			if (original_length != text.length() )
				{
				m_step1_step2_altered = true;
				step_3(text);
				}
			}
		///Delete suffix i if in RV and preceded by c 
		//---------------------------------------------
		void step_3(std::basic_string<xchar>& text) 
			{
			if (is_suffix(text,/*ci*/LOWER_C, UPPER_C, LOWER_I, UPPER_I) &&
				m_rv <= text.length()-1)
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				}
			}
		/**Alternatively, if neither steps 1 nor 2 altered the word, do step 4.
		If the word ends with one of the suffixes 

			-os a i o á í ó 

		in RV, delete it*/
		//---------------------------------------------
		void step_4(std::basic_string<xchar>& text) 
			{
			if (delete_if_is_in_rv(text,/*os*/LOWER_O, UPPER_O, LOWER_S, UPPER_S) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text, LOWER_A, UPPER_A) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text, LOWER_I, UPPER_I) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text, LOWER_O, UPPER_O) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text, LOWER_A_ACUTE, UPPER_A_ACUTE) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text, LOWER_I_ACUTE, UPPER_I_ACUTE) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text, LOWER_O_ACUTE, UPPER_O_ACUTE) )
				{
				return;
				}
			}
		/**Always do step 5.
		If the word ends with one of 

			-e é ê 

		in RV, delete it, and if preceded by gu (or ci) with the u (or i) in RV, delete the u (or i). 
		Or if the word ends ç remove the cedilla*/
		//---------------------------------------------
		void step_5(std::basic_string<xchar>& text) 
			{
			if (delete_if_is_in_rv(text, LOWER_E, UPPER_E, false) )
				{
				if (m_rv <= text.length()-1 &&
					(is_suffix(text,/*gu*/LOWER_G, UPPER_G, LOWER_U, UPPER_U) ||
					is_suffix(text,/*ci*/LOWER_C, UPPER_C, LOWER_I, UPPER_I)) )
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					}
				return;
				}
			else if (delete_if_is_in_rv(text,/*é*/LOWER_E_ACUTE, UPPER_E_ACUTE, false) )
				{
				if (m_rv <= text.length()-1 &&
					(is_suffix(text,/*gu*/LOWER_G, UPPER_G, LOWER_U, UPPER_U) ||
					is_suffix(text,/*ci*/LOWER_C, UPPER_C, LOWER_I, UPPER_I)) )
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					}
				return;
				}
			else if (delete_if_is_in_rv(text,/*ê*/LOWER_E_CIRCUMFLEX, UPPER_E_CIRCUMFLEX, false) )
				{
				if (m_rv <= text.length()-1 &&
					(is_suffix(text,/*gu*/LOWER_G, UPPER_G, LOWER_U, UPPER_U) ||
					is_suffix(text,/*ci*/LOWER_C, UPPER_C, LOWER_I, UPPER_I)) )
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					}
				return;
				}
			else if (text[text.length()-1] == UPPER_C_CEDILLA)
				{
				text[text.length()-1] = UPPER_C;
				}
			else if (text[text.length()-1] == LOWER_C_CEDILLA)
				{
				text[text.length()-1] = LOWER_C;
				}
			}
		//internal data specific to Portuguese stemmer
		bool m_step1_step2_altered;
		size_t m_altered_suffix_index;
		};
	}

#endif //__PORTUGUESE_STEM_H__
