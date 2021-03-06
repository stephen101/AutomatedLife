/***************************************************************************
                          italian_stem.h  -  description
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

#ifndef __ITALIAN_STEM_H__
#define __ITALIAN_STEM_H__

#include "semantic/stem/stemming.h"

namespace stemming
	{
	/**Italian can include the following accented forms: 
		-� � � � � � � � � � 
	First, replace all acute accents by grave accents.
	And, as in French, put u after q, and u, i between vowels into upper case. The vowels are then 
		-a e i o u � � � � � 
	R2 and RV have the same definition as in the Spanish stemmer.*/
	//------------------------------------------------------
	class italian_stem : stem<xchar>
		{
	public:
		//---------------------------------------------
        ///@param text: string to stem
		void operator()(std::basic_string<xchar>& text)
			{
			if (text.length() < 3)
				{
				italian_acutes_to_graves(text);
				return;
				}

			//reset internal data
			m_r1 = m_r2 = m_rv = 0;

			trim_western_punctuation(text);
			italian_acutes_to_graves(text);
			hash_italian_ui(text, _XTEXT("aeiou�����AEIOU�����"));

			find_r1(text, _XTEXT("aeiou�����AEIOU�����"));
			find_r2(text, _XTEXT("aeiou�����AEIOU�����"));
			find_spanish_rv(text, _XTEXT("aeiou�����AEIOU�����"));

			//step 0:
			step_0(text);
			//step 1:
			size_t text_length = text.length();
			step_1(text);

			//step 2 is called only if step 1 did not remove a suffix
			if (text_length == text.length() )
				{
				step_2(text);
				}

			//step 3:
			step_3a(text);
			step_3b(text);

			unhash_italian_ui(text);
			}
	private:
		/**Search for the longest among the following suffixes 

			-ci gli la le li lo mi ne si ti vi sene gliela gliele glieli glielo gliene mela
			mele meli melo mene tela tele teli telo tene cela cele celi celo cene vela vele veli velo vene 

		following one of

			-#ando endo
			-#ar er ir 

		in RV. In case of (a) the suffix is deleted, in case (b) it is replace by e
		(guardandogli -> guardando, accomodarci -> accomodare)*/
		//---------------------------------------------
		void step_0(std::basic_string<xchar>& text)
			{
			if (is_suffix(text,/*gliela*/LOWER_G, UPPER_G, LOWER_L, UPPER_L, LOWER_I, UPPER_I, LOWER_E, UPPER_E, LOWER_L, UPPER_L, LOWER_A, UPPER_A) )
				{
				if (step_0a(text, 6) || step_0b(text, 6) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*gliele*/LOWER_G, UPPER_G, LOWER_L, UPPER_L, LOWER_I, UPPER_I, LOWER_E, UPPER_E, LOWER_L, UPPER_L, LOWER_E, UPPER_E) )
				{
				if (step_0a(text, 6) || step_0b(text, 6) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*glieli*/LOWER_G, UPPER_G, LOWER_L, UPPER_L, LOWER_I, UPPER_I, LOWER_E, UPPER_E, LOWER_L, UPPER_L, LOWER_I, UPPER_I) )
				{
				if (step_0a(text, 6) || step_0b(text, 6) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*glielo*/LOWER_G, UPPER_G, LOWER_L, UPPER_L, LOWER_I, UPPER_I, LOWER_E, UPPER_E, LOWER_L, UPPER_L, LOWER_O, UPPER_O) )
				{
				if (step_0a(text, 6) || step_0b(text, 6) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*gliene*/LOWER_G, UPPER_G, LOWER_L, UPPER_L, LOWER_I, UPPER_I, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_E, UPPER_E) )
				{
				if (step_0a(text, 6) || step_0b(text, 6) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*sene*/LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_E, UPPER_E) )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*mela*/LOWER_M, UPPER_M, LOWER_E, UPPER_E, LOWER_L, UPPER_L, LOWER_A, UPPER_A) )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*mele*/LOWER_M, UPPER_M, LOWER_E, UPPER_E, LOWER_L, UPPER_L, LOWER_E, UPPER_E) )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*meli*/LOWER_M, UPPER_M, LOWER_E, UPPER_E, LOWER_L, UPPER_L, LOWER_I, UPPER_I) )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*melo*/LOWER_M, UPPER_M, LOWER_E, UPPER_E, LOWER_L, UPPER_L, LOWER_O, UPPER_O) )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*mene*/LOWER_M, UPPER_M, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_E, UPPER_E) )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*tela*/LOWER_T, UPPER_T, LOWER_E, UPPER_E, LOWER_L, UPPER_L, LOWER_A, UPPER_A) )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*tele*/LOWER_T, UPPER_T, LOWER_E, UPPER_E, LOWER_L, UPPER_L, LOWER_E, UPPER_E) )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*teli*/LOWER_T, UPPER_T, LOWER_E, UPPER_E, LOWER_L, UPPER_L, LOWER_I, UPPER_I) )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*telo*/LOWER_T, UPPER_T, LOWER_E, UPPER_E, LOWER_L, UPPER_L, LOWER_O, UPPER_O) )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*tene*/LOWER_T, UPPER_T, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_E, UPPER_E) )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*cela*/LOWER_C, UPPER_C, LOWER_E, UPPER_E, LOWER_L, UPPER_L, LOWER_A, UPPER_A) )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*cela*/LOWER_C, UPPER_C, LOWER_E, UPPER_E, LOWER_L, UPPER_L, LOWER_A, UPPER_A) )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*celi*/LOWER_C, UPPER_C, LOWER_E, UPPER_E, LOWER_L, UPPER_L, LOWER_I, UPPER_I) )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*celo*/LOWER_C, UPPER_C, LOWER_E, UPPER_E, LOWER_L, UPPER_L, LOWER_O, UPPER_O) )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*cene*/LOWER_C, UPPER_C, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_E, UPPER_E) )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*vela*/LOWER_V, UPPER_V, LOWER_E, UPPER_E, LOWER_L, UPPER_L, LOWER_A, UPPER_A) )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*vele*/LOWER_V, UPPER_V, LOWER_E, UPPER_E, LOWER_L, UPPER_L, LOWER_E, UPPER_E) )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*veli*/LOWER_V, UPPER_V, LOWER_E, UPPER_E, LOWER_L, UPPER_L, LOWER_I, UPPER_I) )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*velo*/LOWER_V, UPPER_V, LOWER_E, UPPER_E, LOWER_L, UPPER_L, LOWER_O, UPPER_O) )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*vene*/LOWER_V, UPPER_V, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_E, UPPER_E) )
				{
				if (step_0a(text, 4) || step_0b(text, 4) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*gli*/LOWER_G, UPPER_G, LOWER_L, UPPER_L, LOWER_I, UPPER_I) )
				{
				if (step_0a(text, 3) || step_0b(text, 3) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*ci*/LOWER_C, UPPER_C, LOWER_I, UPPER_I) )
				{
				if (step_0a(text, 2) || step_0b(text, 2) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*la*/LOWER_L, UPPER_L, LOWER_A, UPPER_A) )
				{
				if (step_0a(text, 2) || step_0b(text, 2) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*le*/LOWER_L, UPPER_L, LOWER_E, UPPER_E) )
				{
				if (step_0a(text, 2) || step_0b(text, 2) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*li*/LOWER_L, UPPER_L, LOWER_I, UPPER_I) )
				{
				if (step_0a(text, 2) || step_0b(text, 2) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*lo*/LOWER_L, UPPER_L, LOWER_O, UPPER_O) )
				{
				if (step_0a(text, 2) || step_0b(text, 2) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*mi*/LOWER_M, UPPER_M, LOWER_I, UPPER_I) )
				{
				if (step_0a(text, 2) || step_0b(text, 2) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*ne*/LOWER_N, UPPER_N, LOWER_E, UPPER_E) )
				{
				if (step_0a(text, 2) || step_0b(text, 2) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*si*/LOWER_S, UPPER_S, LOWER_I, UPPER_I) )
				{
				if (step_0a(text, 2) || step_0b(text, 2) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*ti*/LOWER_T, UPPER_T, LOWER_I, UPPER_I) )
				{
				if (step_0a(text, 2) || step_0b(text, 2) ) { /*NOOP*/ }
				return;
				}
			else if (is_suffix(text,/*vi*/LOWER_V, UPPER_V, LOWER_I, UPPER_I) )
				{
				if (step_0a(text, 2) || step_0b(text, 2) ) { /*NOOP*/ }
				return;
				}
			}
		//---------------------------------------------
		bool step_0a(std::basic_string<xchar>& text, size_t suffix_length)
			{
			if (text.length() >= suffix_length + 4 &&
				m_rv <= text.length()-4-suffix_length &&
				(/*ando*/(is_either(text[text.length()-4-suffix_length], LOWER_A, UPPER_A) &&
						is_either(text[text.length()-3-suffix_length], LOWER_N, UPPER_N) &&
						is_either(text[text.length()-2-suffix_length], LOWER_D, UPPER_D) &&
						is_either(text[text.length()-1-suffix_length], LOWER_O, UPPER_O) ) ||
				/*endo*/(is_either(text[text.length()-4-suffix_length], LOWER_E, UPPER_E) &&
						is_either(text[text.length()-3-suffix_length], LOWER_N, UPPER_N) &&
						is_either(text[text.length()-2-suffix_length], LOWER_D, UPPER_D) &&
						is_either(text[text.length()-1-suffix_length], LOWER_O, UPPER_O) ) ) )
				{
				text.erase(text.end()-suffix_length, text.end() );
				update_r_sections(text);
				return true;
				}
			return false;
			}
		//---------------------------------------------
		bool step_0b(std::basic_string<xchar>& text, size_t suffix_length)
			{
			if ((text.length() >= suffix_length + 2) &&
				m_rv <= text.length()-2-suffix_length &&
				(
				/*ar*/(is_either(text[text.length()-2-suffix_length], LOWER_A, UPPER_A) && is_either(text[text.length()-1-suffix_length], LOWER_R, UPPER_R) ) ||
				/*er*/(is_either(text[text.length()-2-suffix_length], LOWER_E, UPPER_E) && is_either(text[text.length()-1-suffix_length], LOWER_R, UPPER_R) ) ||
				/*or*/(is_either(text[text.length()-2-suffix_length], LOWER_I, UPPER_I) && is_either(text[text.length()-1-suffix_length], LOWER_R, UPPER_R) )
				) )
				{
				text.replace(text.end()-suffix_length, text.end(), _XTEXT("e"));
				update_r_sections(text);
				return true;
				}
			return false;
			}
		/**Search for the longest among the following suffixes, and perform the action indicated. 

			-anza   anze   ico   ici   ica   ice   iche   ichi   ismo   ismi   abile   abili   ibile
             ibili   ista   iste   isti   ist�   ist�   ist�   oso   osi   osa   ose   mente   atrice
             atrici  ante   anti
				-delete if in R2

			-azione   azioni   atore   atori delete if in R2 
				-if preceded by ic, delete if in R2

			-logia   logie 
				-replace with log if in R2

			-uzione   uzioni   usione   usioni 
				-replace with u if in R2

			-enza   enze 
				-replace with ente if in R2

			-amento   amenti   imento   imenti 
				-delete if in RV

			-amente 
				-delete if in R1 
				-if preceded by iv, delete if in R2 (and if further preceded by at, delete if in R2), otherwise, 
				-if preceded by os, ic or abil, delete if in R2

			-it� 
				-delete if in R2 
				-if preceded by abil, ic or iv, delete if in R2

			-ivo   ivi   iva   ive 
				-delete if in R2 
				-if preceded by at, delete if in R2 (and if further preceded by ic, delete if in R2)*/
		//---------------------------------------------
		void step_1(std::basic_string<xchar>& text)
			{
			if (delete_if_is_in_rv(text,/*amento*/LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_O, UPPER_O) ||
				delete_if_is_in_rv(text,/*amenti*/LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_I, UPPER_I) ||
				delete_if_is_in_rv(text,/*imento*/LOWER_I, UPPER_I, LOWER_M, UPPER_M, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_O, UPPER_O) ||
				delete_if_is_in_rv(text,/*imenti*/LOWER_I, UPPER_I, LOWER_M, UPPER_M, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_I, UPPER_I) )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*azione*/LOWER_A, UPPER_A, LOWER_Z, UPPER_Z, LOWER_I, UPPER_I, LOWER_O, UPPER_O, LOWER_N, UPPER_N, LOWER_E, UPPER_E) )
				{
				delete_if_is_in_r2(text,/*ic*/LOWER_I, UPPER_I, LOWER_C, UPPER_C);
				return;
				}
			else if (delete_if_is_in_r2(text,/*azioni*/LOWER_A, UPPER_A, LOWER_Z, UPPER_Z, LOWER_I, UPPER_I, LOWER_O, UPPER_O, LOWER_N, UPPER_N, LOWER_I, UPPER_I) )
				{
				delete_if_is_in_r2(text,/*ic*/LOWER_I, UPPER_I, LOWER_C, UPPER_C);
				return;
				}
			else if (is_suffix_in_r2(text,/*uzione*/LOWER_U, UPPER_U, LOWER_Z, UPPER_Z, LOWER_I, UPPER_I, LOWER_O, UPPER_O, LOWER_N, UPPER_N, LOWER_E, UPPER_E) ||
				is_suffix_in_r2(text,/*uzioni*/LOWER_U, UPPER_U, LOWER_Z, UPPER_Z, LOWER_I, UPPER_I, LOWER_O, UPPER_O, LOWER_N, UPPER_N, LOWER_I, UPPER_I) ||
				is_suffix_in_r2(text,/*usione*/LOWER_U, UPPER_U, LOWER_S, UPPER_S, LOWER_I, UPPER_I, LOWER_O, UPPER_O, LOWER_N, UPPER_N, LOWER_E, UPPER_E) ||
				is_suffix_in_r2(text,/*usioni*/LOWER_U, UPPER_U, LOWER_S, UPPER_S, LOWER_I, UPPER_I, LOWER_O, UPPER_O, LOWER_N, UPPER_N, LOWER_I, UPPER_I) )
				{
				text.erase(text.end()-5, text.end() );
				update_r_sections(text);
				return;
				}
			else if (delete_if_is_in_r1(text,/*amente*/LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_E, UPPER_E) )
				{
				if (delete_if_is_in_r2(text,/*iv*/LOWER_I, UPPER_I, LOWER_V, UPPER_V) )
					{
					delete_if_is_in_r2(text,/*at*/LOWER_A, UPPER_A, LOWER_T, UPPER_T);
					}
				else if (delete_if_is_in_r2(text,/*abil*/LOWER_A, UPPER_A, LOWER_B, UPPER_B, LOWER_I, UPPER_I, LOWER_L, UPPER_L) ||
						delete_if_is_in_r2(text,/*ic*/LOWER_I, UPPER_I, LOWER_C, UPPER_C) ||
						delete_if_is_in_r2(text,/*os*/LOWER_O, UPPER_O, LOWER_S, UPPER_S) )
					{ /*NOOP*/ }
				return;
				}
			else if (delete_if_is_in_r2(text,/*atrice*/LOWER_A, UPPER_A, LOWER_T, UPPER_T, LOWER_R, UPPER_R, LOWER_I, UPPER_I, LOWER_C, UPPER_C, LOWER_E, UPPER_E) ||
                    delete_if_is_in_r2(text,/*atrici*/LOWER_A, UPPER_A, LOWER_T, UPPER_T, LOWER_R, UPPER_R, LOWER_I, UPPER_I, LOWER_C, UPPER_C, LOWER_I, UPPER_I) )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*abile*/LOWER_A, UPPER_A, LOWER_B, UPPER_B, LOWER_I, UPPER_I, LOWER_L, UPPER_L, LOWER_E, UPPER_E) ||
                    delete_if_is_in_r2(text,/*abili*/LOWER_A, UPPER_A, LOWER_B, UPPER_B, LOWER_I, UPPER_I, LOWER_L, UPPER_L, LOWER_I, UPPER_I) ||
                    delete_if_is_in_r2(text,/*ibile*/LOWER_I, UPPER_I, LOWER_B, UPPER_B, LOWER_I, UPPER_I, LOWER_L, UPPER_L, LOWER_E, UPPER_E) ||
                    delete_if_is_in_r2(text,/*ibili*/LOWER_I, UPPER_I, LOWER_B, UPPER_B, LOWER_I, UPPER_I, LOWER_L, UPPER_L, LOWER_I, UPPER_I) ||
                    delete_if_is_in_r2(text,/*mente*/LOWER_M, UPPER_M, LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_E, UPPER_E) )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*atore*/LOWER_A, UPPER_A, LOWER_T, UPPER_T, LOWER_O, UPPER_O, LOWER_R, UPPER_R, LOWER_E, UPPER_E) )
				{
				delete_if_is_in_r2(text,/*ic*/LOWER_I, UPPER_I, LOWER_C, UPPER_C);
				return;
				}
			else if (delete_if_is_in_r2(text,/*atori*/LOWER_A, UPPER_A, LOWER_T, UPPER_T, LOWER_O, UPPER_O, LOWER_R, UPPER_R, LOWER_I, UPPER_I) )
				{
				delete_if_is_in_r2(text,/*ic*/LOWER_I, UPPER_I, LOWER_C, UPPER_C);
				return;
				}
			else if (is_suffix_in_r2(text,/*logia*/LOWER_L, UPPER_L, LOWER_O, UPPER_O, LOWER_G, UPPER_G, LOWER_I, UPPER_I, LOWER_A, UPPER_A) ||
				is_suffix_in_r2(text,/*logie*/LOWER_L, UPPER_L, LOWER_O, UPPER_O, LOWER_G, UPPER_G, LOWER_I, UPPER_I, LOWER_E, UPPER_E) )
				{
				text.erase(text.end()-2, text.end() );
				update_r_sections(text);
				return;
				}
			else if (is_suffix_in_r2(text,/*enza*/LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_Z, UPPER_Z, LOWER_A, UPPER_A) ||
				is_suffix_in_r2(text,/*enze*/LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_Z, UPPER_Z, LOWER_E, UPPER_E) )
				{
				text.replace(text.end()-2, text.end(), _XTEXT("te"));
				update_r_sections(text);
				return;
				}
			else if (delete_if_is_in_r2(text,/*ante*/LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_E, UPPER_E) ||
                    delete_if_is_in_r2(text,/*anti*/LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_T, UPPER_T, LOWER_I, UPPER_I) ||
                    delete_if_is_in_r2(text,/*anza*/LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_Z, UPPER_Z, LOWER_A, UPPER_A) ||
                    delete_if_is_in_r2(text,/*anze*/LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_Z, UPPER_Z, LOWER_E, UPPER_E) ||
                    delete_if_is_in_r2(text,/*iche*/LOWER_I, UPPER_I, LOWER_C, UPPER_C, LOWER_H, UPPER_H, LOWER_E, UPPER_E) ||
                    delete_if_is_in_r2(text,/*ichi*/LOWER_I, UPPER_I, LOWER_C, UPPER_C, LOWER_H, UPPER_H, LOWER_I, UPPER_I) ||
                    delete_if_is_in_r2(text,/*ismo*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_M, UPPER_M, LOWER_O, UPPER_O) ||
                    delete_if_is_in_r2(text,/*ismi*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_M, UPPER_M, LOWER_I, UPPER_I) ||
                    delete_if_is_in_r2(text,/*ista*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_T, UPPER_T, LOWER_A, UPPER_A) ||
                    delete_if_is_in_r2(text,/*iste*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_T, UPPER_T, LOWER_E, UPPER_E) ||
                    delete_if_is_in_r2(text,/*isti*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_T, UPPER_T, LOWER_I, UPPER_I) ||
                    delete_if_is_in_r2(text,/*ist�*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_T, UPPER_T, LOWER_A_GRAVE, UPPER_A_GRAVE) ||
                    delete_if_is_in_r2(text,/*ist�*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_T, UPPER_T, LOWER_E_GRAVE, UPPER_E_GRAVE) ||
                    delete_if_is_in_r2(text,/*ist�*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_T, UPPER_T, LOWER_I_GRAVE, UPPER_I_GRAVE) )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*ico*/LOWER_I, UPPER_I, LOWER_C, UPPER_C, LOWER_O, UPPER_O) )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*ici*/LOWER_I, UPPER_I, LOWER_C, UPPER_C, LOWER_I, UPPER_I) )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*ica*/LOWER_I, UPPER_I, LOWER_C, UPPER_C, LOWER_A, UPPER_A) )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*ice*/LOWER_I, UPPER_I, LOWER_C, UPPER_C, LOWER_E, UPPER_E) )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*oso*/LOWER_O, UPPER_O, LOWER_S, UPPER_S, LOWER_O, UPPER_O) )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*osi*/LOWER_O, UPPER_O, LOWER_S, UPPER_S, LOWER_I, UPPER_I) )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*osa*/LOWER_O, UPPER_O, LOWER_S, UPPER_S, LOWER_A, UPPER_A) )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*ose*/LOWER_O, UPPER_O, LOWER_S, UPPER_S, LOWER_E, UPPER_E) )
				{
				return;
				}
			else if (delete_if_is_in_r2(text,/*it�*/LOWER_I, UPPER_I, LOWER_T, UPPER_T, LOWER_A_GRAVE, UPPER_A_GRAVE) )
				{
				if (delete_if_is_in_r2(text,/*abil*/LOWER_A, UPPER_A, LOWER_B, UPPER_B, LOWER_I, UPPER_I, LOWER_L, UPPER_L) ||
					delete_if_is_in_r2(text,/*ic*/LOWER_I, UPPER_I, LOWER_C, UPPER_C) ||
					delete_if_is_in_r2(text,/*iv*/LOWER_I, UPPER_I, LOWER_V, UPPER_V) )
					{ /*NOOP*/ }
				return;
				}
			else if (delete_if_is_in_r2(text,/*ivo*/LOWER_I, UPPER_I, LOWER_V, UPPER_V, LOWER_O, UPPER_O) ||
				delete_if_is_in_r2(text,/*ivi*/LOWER_I, UPPER_I, LOWER_V, UPPER_V, LOWER_I, UPPER_I) ||
				delete_if_is_in_r2(text,/*iva*/LOWER_I, UPPER_I, LOWER_V, UPPER_V, LOWER_A, UPPER_A) ||
				delete_if_is_in_r2(text,/*ive*/LOWER_I, UPPER_I, LOWER_V, UPPER_V, LOWER_E, UPPER_E) )
				{
				if (delete_if_is_in_r2(text,/*at*/LOWER_A, UPPER_A, LOWER_T, UPPER_T) )
					{
					delete_if_is_in_r2(text,/*ic*/LOWER_I, UPPER_I, LOWER_C, UPPER_C);
					}
				return;
				}
			}
		/**Do step 2 if no ending was removed by step 1
		Search for the longest among the following suffixes in RV, and if found, delete. 

			-ammo ando ano are arono asse assero assi assimo ata ate ati ato
			ava avamo avano avate avi avo emmo enda ende endi endo er� erai
			eranno ere erebbe erebbero erei eremmo eremo ereste eresti erete
			er� erono essero ete eva evamo evano evate evi evo Yamo iamo immo 
			ir� irai iranno ire irebbe irebbero irei iremmo iremo ireste iresti
			irete ir� irono isca iscano isce isci isco iscono issero ita ite iti
			ito iva ivamo ivano ivate ivi ivo ono uta ute uti uto ar ir 

		Always do steps 3a and 3b. */
		//---------------------------------------------
		void step_2(std::basic_string<xchar>& text)
			{			
			if (delete_if_is_in_rv(text,/*erebbero*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_B, UPPER_B, LOWER_B, UPPER_B, LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*irebbero*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_B, UPPER_B, LOWER_B, UPPER_B, LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*assero*/LOWER_A, UPPER_A, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*assimo*/LOWER_A, UPPER_A, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_I, UPPER_I, LOWER_M, UPPER_M, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*eranno*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_N, UPPER_N, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*erebbe*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_B, UPPER_B, LOWER_B, UPPER_B, LOWER_E, UPPER_E, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*eremmo*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_M, UPPER_M, LOWER_M, UPPER_M, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ereste*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_S, UPPER_S, LOWER_T, UPPER_T, LOWER_E, UPPER_E, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*eresti*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_S, UPPER_S, LOWER_T, UPPER_T, LOWER_I, UPPER_I, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*essero*/LOWER_E, UPPER_E, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iranno*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_N, UPPER_N, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*irebbe*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_B, UPPER_B, LOWER_B, UPPER_B, LOWER_E, UPPER_E, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iremmo*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_M, UPPER_M, LOWER_M, UPPER_M, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ireste*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_S, UPPER_S, LOWER_T, UPPER_T, LOWER_E, UPPER_E, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iresti*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_S, UPPER_S, LOWER_T, UPPER_T, LOWER_I, UPPER_I, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iscano*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_C, UPPER_C, LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iscono*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_C, UPPER_C, LOWER_O, UPPER_O, LOWER_N, UPPER_N, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*issero*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*arono*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_O, UPPER_O, LOWER_N, UPPER_N, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*avamo*/LOWER_A, UPPER_A, LOWER_V, UPPER_V, LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*avano*/LOWER_A, UPPER_A, LOWER_V, UPPER_V, LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*avate*/LOWER_A, UPPER_A, LOWER_V, UPPER_V, LOWER_A, UPPER_A, LOWER_T, UPPER_T, LOWER_E, UPPER_E, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*eremo*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_M, UPPER_M, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*erete*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_T, UPPER_T, LOWER_E, UPPER_E, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*erono*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_O, UPPER_O, LOWER_N, UPPER_N, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*evamo*/LOWER_E, UPPER_E, LOWER_V, UPPER_V, LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*evano*/LOWER_E, UPPER_E, LOWER_V, UPPER_V, LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*evate*/LOWER_E, UPPER_E, LOWER_V, UPPER_V, LOWER_A, UPPER_A, LOWER_T, UPPER_T, LOWER_E, UPPER_E, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iremo*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_M, UPPER_M, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*irete*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_T, UPPER_T, LOWER_E, UPPER_E, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*irono*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_O, UPPER_O, LOWER_N, UPPER_N, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ivamo*/LOWER_I, UPPER_I, LOWER_V, UPPER_V, LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ivano*/LOWER_I, UPPER_I, LOWER_V, UPPER_V, LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ivate*/LOWER_I, UPPER_I, LOWER_V, UPPER_V, LOWER_A, UPPER_A, LOWER_T, UPPER_T, LOWER_E, UPPER_E, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ammo*/LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_M, UPPER_M, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ando*/LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_D, UPPER_D, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text, /*asse*/LOWER_A, UPPER_A, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_E, UPPER_E, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*assi*/LOWER_A, UPPER_A, LOWER_S, UPPER_S, LOWER_S, UPPER_S, LOWER_I, UPPER_I, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*emmo*/LOWER_E, UPPER_E, LOWER_M, UPPER_M, LOWER_M, UPPER_M, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*enda*/LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_D, UPPER_D, LOWER_A, UPPER_A, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ende*/LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_D, UPPER_D, LOWER_E, UPPER_E, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*endi*/LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_D, UPPER_D, LOWER_I, UPPER_I, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*endo*/LOWER_E, UPPER_E, LOWER_N, UPPER_N, LOWER_D, UPPER_D, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*erai*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_I, UPPER_I, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*erei*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_I, UPPER_I, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*yamo*/LOWER_Y, UPPER_Y, LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iamo*/LOWER_I, UPPER_I, LOWER_A, UPPER_A, LOWER_M, UPPER_M, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*immo*/LOWER_I, UPPER_I, LOWER_M, UPPER_M, LOWER_M, UPPER_M, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*irai*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_A, UPPER_A, LOWER_I, UPPER_I, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*irei*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_E, UPPER_E, LOWER_I, UPPER_I, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*isca*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_C, UPPER_C, LOWER_A, UPPER_A, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*isce*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_C, UPPER_C, LOWER_E, UPPER_E, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*isci*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_C, UPPER_C, LOWER_I, UPPER_I, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*isco*/LOWER_I, UPPER_I, LOWER_S, UPPER_S, LOWER_C, UPPER_C, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ano*/LOWER_A, UPPER_A, LOWER_N, UPPER_N, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*are*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, LOWER_E, UPPER_E, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ata*/LOWER_A, UPPER_A, LOWER_T, UPPER_T, LOWER_A, UPPER_A, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ate*/LOWER_A, UPPER_A, LOWER_T, UPPER_T, LOWER_E, UPPER_E, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ati*/LOWER_A, UPPER_A, LOWER_T, UPPER_T, LOWER_I, UPPER_I, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ato*/LOWER_A, UPPER_A, LOWER_T, UPPER_T, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ava*/LOWER_A, UPPER_A, LOWER_V, UPPER_V, LOWER_A, UPPER_A, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*avi*/LOWER_A, UPPER_A, LOWER_V, UPPER_V, LOWER_I, UPPER_I, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*avo*/LOWER_A, UPPER_A, LOWER_V, UPPER_V, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*er�*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_A_GRAVE, UPPER_A_GRAVE, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ere*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_E, UPPER_E, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*er�*/LOWER_E, UPPER_E, LOWER_R, UPPER_R, LOWER_O_GRAVE, UPPER_O_GRAVE, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ete*/LOWER_E, UPPER_E, LOWER_T, UPPER_T, LOWER_E, UPPER_E, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*eva*/LOWER_E, UPPER_E, LOWER_V, UPPER_V, LOWER_A, UPPER_A, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*evi*/LOWER_E, UPPER_E, LOWER_V, UPPER_V, LOWER_I, UPPER_I, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*evo*/LOWER_E, UPPER_E, LOWER_V, UPPER_V, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ir�*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_A_GRAVE, UPPER_A_GRAVE, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ire*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_E, UPPER_E, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ir�*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, LOWER_O_GRAVE, UPPER_O_GRAVE, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ita*/LOWER_I, UPPER_I, LOWER_T, UPPER_T, LOWER_A, UPPER_A, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ite*/LOWER_I, UPPER_I, LOWER_T, UPPER_T, LOWER_E, UPPER_E, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iti*/LOWER_I, UPPER_I, LOWER_T, UPPER_T, LOWER_I, UPPER_I, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ito*/LOWER_I, UPPER_I, LOWER_T, UPPER_T, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*iva*/LOWER_I, UPPER_I, LOWER_V, UPPER_V, LOWER_A, UPPER_A, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ivi*/LOWER_I, UPPER_I, LOWER_V, UPPER_V, LOWER_I, UPPER_I, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ivo*/LOWER_I, UPPER_I, LOWER_V, UPPER_V, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ono*/LOWER_O, UPPER_O, LOWER_N, UPPER_N, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*uta*/LOWER_U, UPPER_U, LOWER_T, UPPER_T, LOWER_A, UPPER_A, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ute*/LOWER_U, UPPER_U, LOWER_T, UPPER_T, LOWER_E, UPPER_E, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*uti*/LOWER_U, UPPER_U, LOWER_T, UPPER_T, LOWER_I, UPPER_I, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*uto*/LOWER_U, UPPER_U, LOWER_T, UPPER_T, LOWER_O, UPPER_O, false) )
				{
				return;
				}
			else if (delete_if_is_in_rv(text,/*ar*/LOWER_A, UPPER_A, LOWER_R, UPPER_R, false) )
				{
				return;
				}
			///'ir' not in original specification, but used in general implementation
			else if (delete_if_is_in_rv(text,/*ir*/LOWER_I, UPPER_I, LOWER_R, UPPER_R, false) )
				{
				return;
				}
			/**deletion or 'er' from rv is considered problematic,
			but part of the standard*/
			}
		/**Delete a final a, e, i, o, �, �, � or � if it is in RV,
		and a preceding i if it is in RV (crocchi -> crocch, crocchio -> crocch)*/
		//---------------------------------------------
		void step_3a(std::basic_string<xchar>& text)
			{
			if (m_rv <= text.length()-1 &&
				string_util::is_one_of(text[text.length()-1], _XTEXT("aeio����AEIO����")) )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				if (m_rv <= text.length()-1 &&					
					is_either(text[text.length()-1], LOWER_I, UPPER_I) )
					{
					text.erase(text.end()-1, text.end() );
					update_r_sections(text);
					}
				}
			}
		///Replace final ch (or gh) with c (or g) if in RV (crocch -> crocc) 
		//---------------------------------------------
		void step_3b(std::basic_string<xchar>& text)
			{
			if (is_suffix_in_rv(text,/*ch*/LOWER_C, UPPER_C, LOWER_H, UPPER_H) ||
				is_suffix_in_rv(text,/*gh*/LOWER_G, UPPER_G, LOWER_H, UPPER_H) )
				{
				text.erase(text.end()-1, text.end() );
				update_r_sections(text);
				}
			}
		};
	}

#endif //__ITALIAN_STEM_H__
