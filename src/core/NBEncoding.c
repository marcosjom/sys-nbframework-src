//
//  XUCadena.c
//  MetricaMobilFramework
//
//  Created by Marcos Ortega on 07/10/13.
//  Copyright (c) 2013 logicoim@gmail.com. All rights reserved.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBEncoding.h"


//ASCII
char NBEncoding_asciiOtherCase(const char c){ //changing from uppercase to lowercase and viceversa
	char r = c;
	//ToDo: implement unicode support
	if(c >= 'A' && c <= 'Z'){
		r = c + 32;
	} else if(c >= 'a' && c <= 'z'){
		r = c - 32;
	/*} else {
		//ASCII extended
		switch (c) {
			case 130: r = 144; break; //é -> É
			case 144: r = 130; break; //É -> é
			case 160: r = 181; break; //á -> Á
			case 161: r = 214; break; //í -> Í
			case 162: r = 224; break; //ó -> Ó
			case 163: r = 233; break; //ú -> Ú
			case 164: r = 165; break; //ñ -> Ñ
			case 165: r = 164; break; //Ñ -> ñ
			case 181: r = 160; break; //Á -> á
			case 214: r = 161; break; //Í -> í
			case 224: r = 162; break; //Ó -> ó
			case 233: r = 163; break; //Ú -> ú
			default: break;
		}*/
	}
	return r;
}

char NBEncoding_asciiUpper(const char c){
	char r = c;
	//ToDo: implement unicode support
	if(c >= 'A' && c <= 'Z'){
		r = c;
	} else if(c >= 'a' && c <= 'z'){
		r = c - 32;
	/*} else {
		//ASCII extended
		switch (c) {
			case 130: r = 144; break; //é -> É
			case 160: r = 181; break; //á -> Á
			case 161: r = 214; break; //í -> Í
			case 162: r = 224; break; //ó -> Ó
			case 163: r = 233; break; //ú -> Ú
			case 164: r = 165; break; //ñ -> Ñ
			default: break;
		}*/
	}
	return r;
}

char NBEncoding_asciiLower(const char c){
	char r = c;
	//ToDo: implement unicode support
	if(c >= 'A' && c <= 'Z'){
		r = c + 32;
	} else if(c >= 'a' && c <= 'z'){
		r = c;
	/*} else {
		//ASCII extended
		switch (c) {
			case 144: r = 130; break; //É -> é
			case 165: r = 164; break; //Ñ -> ñ
			case 181: r = 160; break; //Á -> á
			case 214: r = 161; break; //Í -> í
			case 224: r = 162; break; //Ó -> ó
			case 233: r = 163; break; //Ú -> ú
			default: break;
		}*/
	}
	return r;
}

//UTF8

UI8 NBEncoding_utf8BytesExpected(const char first){
	/*
	 UTF8
	 1 byte  (0xxxxxxx)
	 2 bytes (110xxxxx 10xxxxxx)
	 3 bytes (1110xxxx 10xxxxxx 10xxxxxx)
	 4 bytes (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
	 5 bytes (111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx)
	 6 bytes (1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx)
	*/
	return ((first & 0x80) == 0 ? 1 : (first & 0xE0) == 0xC0 ? 2 : (first & 0xF0) == 0xE0 ? 3 : (first & 0xF8) == 0xF0 ? 4 : (first & 0xFC) == 0xF8 ? 5 : (first & 0xFE) == 0xFC ? 6 : 0);
}

UI8 NBEncoding_utf8FromUnicode(const UI32 unicode, char* dstChars7){
    UI8 r = 0;
	if(unicode < 0x007F){
		//1 byte  (0xxxxxxx); 7 bits of data
		dstChars7[0] = (char)unicode;
		dstChars7[1] = '\0';
        r = 1;
		//PRINTF_INFO("One byte unicode (%u).\n", *((unsigned char*)&dstChars7[0]));
	} else if(unicode < 0x07FF){
		//2 bytes (110xxxxx 10xxxxxx); 11 bits of data
		dstChars7[0] = (char)(0xC0 + (unicode >> 6));
		dstChars7[1] = (char)(0x80 + (unicode & 0x3F)); //6 bits
		dstChars7[2] = '\0';
        r = 2;
		//PRINTF_INFO("Two bytes unicode (%u, %u).\n", *((unsigned char*)&dstChars7[1]), *((unsigned char*)&dstChars7[0]));
	} else if(unicode < 0xFFFF){
		//3 bytes (1110xxxx 10xxxxxx 10xxxxxx); 16 bits of data
		dstChars7[0] = (char)(0xE0 + (unicode >> 12));
		dstChars7[1] = (char)(0x80 + ((unicode >> 6) & 0x3F)); //6 bits
		dstChars7[2] = (char)(0x80 + (unicode & 0x3F)); //6 bits
		dstChars7[3] = '\0';
        r = 3;
		//PRINTF_INFO("Three bytes unicode (%u, %u, %u).\n", *((unsigned char*)&dstChars7[2]), *((unsigned char*)&dstChars7[1]), *((unsigned char*)&dstChars7[0]));
	} else if(unicode < 0x1FFFFF){
		//4 bytes (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx); 21 bits of data
		dstChars7[0] = (char)(0xF0 + (unicode >> 18));
		dstChars7[1] = (char)(0x80 + ((unicode >> 12) & 0x3F)); //6 bits
		dstChars7[2] = (char)(0x80 + ((unicode >> 6) & 0x3F)); //6 bits
		dstChars7[3] = (char)(0x80 + (unicode & 0x3F)); //6 bits
		dstChars7[4] = '\0';
        r = 4;
		//PRINTF_INFO("Four bytes unicode (%u, %u, %u, %u).\n", *((unsigned char*)&dstChars7[3]), *((unsigned char*)&dstChars7[2]), *((unsigned char*)&dstChars7[1]), *((unsigned char*)&dstChars7[0]));
	} else if(unicode < 0x3FFFFFF){
		//5 bytes (111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx); 26 bits of data
		dstChars7[0] = (char)(0xF8 + (unicode >> 24));
		dstChars7[1] = (char)(0x80 + ((unicode >> 18) & 0x3F)); //6 bits
		dstChars7[2] = (char)(0x80 + ((unicode >> 12) & 0x3F)); //6 bits
		dstChars7[3] = (char)(0x80 + ((unicode >> 6) & 0x3F)); //6 bits
		dstChars7[4] = (char)(0x80 + (unicode & 0x3F)); //6 bits
		dstChars7[5] = '\0';
        r = 5;
		//PRINTF_INFO("Five bytes unicode (%u, %u, %u, %u, %u).\n", *((unsigned char*)&dstChars7[4]), *((unsigned char*)&dstChars7[3]), *((unsigned char*)&dstChars7[2]), *((unsigned char*)&dstChars7[1]), *((unsigned char*)&dstChars7[0]));
	} else {
		//6 bytes (1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx); 31 bits of data
		NBASSERT(unicode < 0x7FFFFFFF)
		dstChars7[0] = (char)(0xFC + (unicode >> 30));
		dstChars7[1] = (char)(0x80 + ((unicode >> 24) & 0x3F)); //6 bits
		dstChars7[2] = (char)(0x80 + ((unicode >> 18) & 0x3F)); //6 bits
		dstChars7[3] = (char)(0x80 + ((unicode >> 12) & 0x3F)); //6 bits
		dstChars7[4] = (char)(0x80 + ((unicode >> 6) & 0x3F)); //6 bits
		dstChars7[5] = (char)(0x80 + (unicode & 0x3F)); //6 bits
		dstChars7[6] = '\0';
        r = 6;
		//PRINTF_INFO("Six bytes unicode (%u, %u, %u, %u, %u, %u).\n", *((unsigned char*)&dstChars7[5]), *((unsigned char*)&dstChars7[4]), *((unsigned char*)&dstChars7[3]), *((unsigned char*)&dstChars7[2]), *((unsigned char*)&dstChars7[1]), *((unsigned char*)&dstChars7[0]));
	}
    return r;
}

//UTF16

UI8 NBEncoding_utf16BytesExpected(const UI16 firstSurrogate){
	/*
	 UTF16
	 2 bytes (00xx-xxxx xxxx-xxxx)
	 4 bytes (1101-10xx xxxx-xxxx 1101-11xx xxxx-xxxx)
	*/
	return ((firstSurrogate & 0xD800) != 0xD800 ? 2 : 4);
}

UI8 NBEncoding_utf16SurrogatesExpected(const UI16 firstSurrogate){
	/*
	 UTF16
	 2 bytes (00xx-xxxx xxxx-xxxx)
	 4 bytes (1101-10xx xxxx-xxxx 1101-11xx xxxx-xxxx)
	 */
	return ((firstSurrogate & 0xD800) != 0xD800 ? 1 : 2);
}
	
UI16 NBEncoding_utf16SurrogateFromHex(const char* chars4){
	//Hex: AB12, ab12
	return (((chars4[0] > '9' ? 0xA : 0x0) + (chars4[0] - (chars4[0] <= '9' ? '0' : chars4[0] <= 'F' ? 'A' : 'a'))) << 12)
	+ (((chars4[1] > '9' ? 0xA : 0x0) + (chars4[1] - (chars4[1] <= '9' ? '0' : chars4[1] <= 'F' ? 'A' : 'a'))) << 8)
	+ (((chars4[2] > '9' ? 0xA : 0x0) + (chars4[2] - (chars4[2] <= '9' ? '0' : chars4[2] <= 'F' ? 'A' : 'a'))) << 4)
	+ ((chars4[3] > '9' ? 0xA : 0x0) + (chars4[3] - (chars4[3] <= '9' ? '0' : chars4[3] <= 'F' ? 'A' : 'a')));
}

//
//http://www.unicode.org/L2/L2013/13006-nonchar-clarif.pdf
//Noncharacter: A code point that is permanently reserved for internal use. Noncharacters consist of
//the values U+nFFFE and U+nFFFF (where n is from 0 to 1016) and the values U+FDD0..U+FDEF.
//
BOOL NBEncoding_utf16SurrogateIsNonCharacterHex(const char* chars4){
	return (
			//U+nFFFE and U+nFFFF
			((chars4[0] == 'f' || chars4[0] == 'F')
			&& (chars4[1] == 'f' || chars4[1] == 'F')
			&& (chars4[2] == 'f' || chars4[2] == 'F')
			&& (chars4[3] == 'e' || chars4[3] == 'E' || chars4[3] == 'f' || chars4[3] == 'F'))
			//U+FDD0..U+FDEF
			||
			(
			 (chars4[0] == 'f' || chars4[0] == 'F')
			 && (chars4[1] == 'd' || chars4[1] == 'D')
			 && (chars4[2] == 'd' || chars4[2] == 'D' || chars4[2] == 'e' || chars4[2] == 'e')
			 //any digit at chars4[3]
			 )
			);
}

//Unicode

UI32 NBEncoding_unicodeFromUtf8(const char* charsUtf8, const UI32 errValue){
	UI32 r = errValue;
	if((charsUtf8[0] & 0x80) == 0){
		r = charsUtf8[0];
	} else if((charsUtf8[0] & 0xE0) == 0xC0){
		/*2 bytes (110xxxxx 10xxxxxx)*/
		NBASSERT((charsUtf8[1] & 0xC0) == 0x80) /*La secuencia debe con '10xx-xxxx'*/
		NBASSERT((charsUtf8[2] & 0xC0) != 0x80) /*No debe ser parte de esta secuencia '10xx-xxxx'*/
		if((charsUtf8[1] & 0xC0) == 0x80 && (charsUtf8[2] & 0xC0) != 0x80){
			r = ((charsUtf8[0] & 0x1F) << 6) | (charsUtf8[1] & 0x3F);
		}
	} else if((charsUtf8[0] & 0xF0) == 0xE0){
		/*3 bytes (1110xxxx 10xxxxxx 10xxxxxx)*/
		NBASSERT((charsUtf8[1] & 0xC0) == 0x80) /*La secuencia debe con '10xx-xxxx'*/
		NBASSERT((charsUtf8[2] & 0xC0) == 0x80) /*La secuencia debe con '10xx-xxxx'*/
		NBASSERT((charsUtf8[3] & 0xC0) != 0x80) /*No debe ser parte de esta secuencia '10xx-xxxx'*/
		if((charsUtf8[1] & 0xC0) == 0x80 && (charsUtf8[2] & 0xC0) == 0x80 && (charsUtf8[3] & 0xC0) != 0x80){
			r = ((charsUtf8[0] & 0xF) << 12) | ((charsUtf8[1] & 0x3F) << 6) | (charsUtf8[2] & 0x3F);
		}
	} else if((charsUtf8[0] & 0xF8) == 0xF0){
		/*4 bytes (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)*/
		NBASSERT((charsUtf8[1] & 0xC0) == 0x80) /*La secuencia debe con '10xx-xxxx'*/
		NBASSERT((charsUtf8[2] & 0xC0) == 0x80) /*La secuencia debe con '10xx-xxxx'*/
		NBASSERT((charsUtf8[3] & 0xC0) == 0x80) /*La secuencia debe con '10xx-xxxx'*/
		NBASSERT((charsUtf8[4] & 0xC0) != 0x80) /*No debe ser parte de esta secuencia '10xx-xxxx'*/
		if((charsUtf8[1] & 0xC0) == 0x80 && (charsUtf8[2] & 0xC0) == 0x80 && (charsUtf8[3] & 0xC0) == 0x80 && (charsUtf8[4] & 0xC0) != 0x80){
			r = ((charsUtf8[0] & 0x7) << 18) | ((charsUtf8[1] & 0x3F) << 12) | ((charsUtf8[2] & 0x3F) << 6) | (charsUtf8[3] & 0x3F);
		}
	} else if((charsUtf8[0] & 0xFC) == 0xF8){
		/*5 bytes (111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx)*/
		NBASSERT((charsUtf8[1] & 0xC0) == 0x80) /*La secuencia debe con '10xx-xxxx'*/
		NBASSERT((charsUtf8[2] & 0xC0) == 0x80) /*La secuencia debe con '10xx-xxxx'*/
		NBASSERT((charsUtf8[3] & 0xC0) == 0x80) /*La secuencia debe con '10xx-xxxx'*/
		NBASSERT((charsUtf8[4] & 0xC0) == 0x80) /*La secuencia debe con '10xx-xxxx'*/
		NBASSERT((charsUtf8[5] & 0xC0) != 0x80) /*No debe ser parte de esta secuencia '10xx-xxxx'*/
		if((charsUtf8[1] & 0xC0) == 0x80 && (charsUtf8[2] & 0xC0) == 0x80 && (charsUtf8[3] & 0xC0) == 0x80 && (charsUtf8[4] & 0xC0) == 0x80 && (charsUtf8[5] & 0xC0) != 0x80){
			r = ((charsUtf8[0] & 0x3) << 24) | ((charsUtf8[1] & 0x3F) << 18) | ((charsUtf8[2] & 0x3F) << 12) | ((charsUtf8[3] & 0x3F) << 6) | (charsUtf8[4] & 0x3F);
		}
	} else if((charsUtf8[0] & 0xFE) == 0xFC){
		/*6 bytes (1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx)*/
		NBASSERT((charsUtf8[1] & 0xC0) == 0x80) /*La secuencia debe con '10xx-xxxx'*/
		NBASSERT((charsUtf8[2] & 0xC0) == 0x80) /*La secuencia debe con '10xx-xxxx'*/
		NBASSERT((charsUtf8[3] & 0xC0) == 0x80) /*La secuencia debe con '10xx-xxxx'*/
		NBASSERT((charsUtf8[4] & 0xC0) == 0x80) /*La secuencia debe con '10xx-xxxx'*/
		NBASSERT((charsUtf8[5] & 0xC0) == 0x80) /*La secuencia debe con '10xx-xxxx'*/
		NBASSERT((charsUtf8[6] & 0xC0) != 0x80) /*No debe ser parte de esta secuencia '10xx-xxxx'*/
		if((charsUtf8[1] & 0xC0) == 0x80 && (charsUtf8[2] & 0xC0) == 0x80 && (charsUtf8[3] & 0xC0) == 0x80 && (charsUtf8[4] & 0xC0) == 0x80 && (charsUtf8[5] & 0xC0) == 0x80 && (charsUtf8[6] & 0xC0) != 0x80){
			r = ((charsUtf8[0] & 0x1) << 30) | ((charsUtf8[1] & 0x3F) << 24) | ((charsUtf8[2] & 0x3F) << 18) | ((charsUtf8[3] & 0x3F) << 12) | ((charsUtf8[4] & 0x3F) << 6) | (charsUtf8[5] & 0x3F);
		}
	}
	return r;
}

UI32 NBEncoding_unicodeFromUtf8s(const char* charsUtf8, const UI8 surrogatesCount, const UI32 errValue){
	UI32 r = errValue;
	switch(surrogatesCount) {
		case 1:
			NBASSERT((charsUtf8[0] & 0x80) == 0)
			if((charsUtf8[0] & 0x80) == 0){
				r = charsUtf8[0];
			}
			break;
		case 2:
			/*2 bytes (110xxxxx 10xxxxxx)*/
			NBASSERT((charsUtf8[0] & 0xE0) == 0xC0)
			NBASSERT((charsUtf8[1] & 0xC0) == 0x80) /*La secuencia debe con '10xx-xxxx'*/
			NBASSERT((charsUtf8[2] & 0xC0) != 0x80) /*No debe ser parte de esta secuencia '10xx-xxxx'*/
			if((charsUtf8[0] & 0xE0) == 0xC0 && (charsUtf8[1] & 0xC0) == 0x80 && (charsUtf8[2] & 0xC0) != 0x80){
				r = ((charsUtf8[0] & 0x1F) << 6) | (charsUtf8[1] & 0x3F);
			}
			break;
		case 3:
			/*3 bytes (1110xxxx 10xxxxxx 10xxxxxx)*/
			NBASSERT((charsUtf8[0] & 0xF0) == 0xE0)
			NBASSERT((charsUtf8[1] & 0xC0) == 0x80) /*La secuencia debe con '10xx-xxxx'*/
			NBASSERT((charsUtf8[2] & 0xC0) == 0x80) /*La secuencia debe con '10xx-xxxx'*/
			NBASSERT((charsUtf8[3] & 0xC0) != 0x80) /*No debe ser parte de esta secuencia '10xx-xxxx'*/
			if((charsUtf8[0] & 0xF0) == 0xE0 && (charsUtf8[1] & 0xC0) == 0x80 && (charsUtf8[2] & 0xC0) == 0x80 && (charsUtf8[3] & 0xC0) != 0x80){
				r = ((charsUtf8[0] & 0xF) << 12) | ((charsUtf8[1] & 0x3F) << 6) | (charsUtf8[2] & 0x3F);
			}
			break;
		case 4:
			/*4 bytes (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)*/
			NBASSERT((charsUtf8[0] & 0xF8) == 0xF0)
			NBASSERT((charsUtf8[1] & 0xC0) == 0x80) /*La secuencia debe con '10xx-xxxx'*/
			NBASSERT((charsUtf8[2] & 0xC0) == 0x80) /*La secuencia debe con '10xx-xxxx'*/
			NBASSERT((charsUtf8[3] & 0xC0) == 0x80) /*La secuencia debe con '10xx-xxxx'*/
			NBASSERT((charsUtf8[4] & 0xC0) != 0x80) /*No debe ser parte de esta secuencia '10xx-xxxx'*/
			if((charsUtf8[0] & 0xF8) == 0xF0 && (charsUtf8[1] & 0xC0) == 0x80 && (charsUtf8[2] & 0xC0) == 0x80 && (charsUtf8[3] & 0xC0) == 0x80 && (charsUtf8[4] & 0xC0) != 0x80){
				r = ((charsUtf8[0] & 0x7) << 18) | ((charsUtf8[1] & 0x3F) << 12) | ((charsUtf8[2] & 0x3F) << 6) | (charsUtf8[3] & 0x3F);
			}
			break;
		case 5:
			/*5 bytes (111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx)*/
			NBASSERT((charsUtf8[0] & 0xFC) == 0xF8)
			NBASSERT((charsUtf8[1] & 0xC0) == 0x80) /*La secuencia debe con '10xx-xxxx'*/
			NBASSERT((charsUtf8[2] & 0xC0) == 0x80) /*La secuencia debe con '10xx-xxxx'*/
			NBASSERT((charsUtf8[3] & 0xC0) == 0x80) /*La secuencia debe con '10xx-xxxx'*/
			NBASSERT((charsUtf8[4] & 0xC0) == 0x80) /*La secuencia debe con '10xx-xxxx'*/
			NBASSERT((charsUtf8[5] & 0xC0) != 0x80) /*No debe ser parte de esta secuencia '10xx-xxxx'*/
			if((charsUtf8[0] & 0xFC) == 0xF8 && (charsUtf8[1] & 0xC0) == 0x80 && (charsUtf8[2] & 0xC0) == 0x80 && (charsUtf8[3] & 0xC0) == 0x80 && (charsUtf8[4] & 0xC0) == 0x80 && (charsUtf8[5] & 0xC0) != 0x80){
				r = ((charsUtf8[0] & 0x3) << 24) | ((charsUtf8[1] & 0x3F) << 18) | ((charsUtf8[2] & 0x3F) << 12) | ((charsUtf8[3] & 0x3F) << 6) | (charsUtf8[4] & 0x3F);
			}
			break;
		case 6:
			/*6 bytes (1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx)*/
			NBASSERT((charsUtf8[0] & 0xFE) == 0xFC)
			NBASSERT((charsUtf8[1] & 0xC0) == 0x80) /*La secuencia debe con '10xx-xxxx'*/
			NBASSERT((charsUtf8[2] & 0xC0) == 0x80) /*La secuencia debe con '10xx-xxxx'*/
			NBASSERT((charsUtf8[3] & 0xC0) == 0x80) /*La secuencia debe con '10xx-xxxx'*/
			NBASSERT((charsUtf8[4] & 0xC0) == 0x80) /*La secuencia debe con '10xx-xxxx'*/
			NBASSERT((charsUtf8[5] & 0xC0) == 0x80) /*La secuencia debe con '10xx-xxxx'*/
			NBASSERT((charsUtf8[6] & 0xC0) != 0x80) /*No debe ser parte de esta secuencia '10xx-xxxx'*/
			if((charsUtf8[0] & 0xFE) == 0xFC && (charsUtf8[1] & 0xC0) == 0x80 && (charsUtf8[2] & 0xC0) == 0x80 && (charsUtf8[3] & 0xC0) == 0x80 && (charsUtf8[4] & 0xC0) == 0x80 && (charsUtf8[5] & 0xC0) == 0x80 && (charsUtf8[6] & 0xC0) != 0x80){
				r = ((charsUtf8[0] & 0x1) << 30) | ((charsUtf8[1] & 0x3F) << 24) | ((charsUtf8[2] & 0x3F) << 18) | ((charsUtf8[3] & 0x3F) << 12) | ((charsUtf8[4] & 0x3F) << 6) | (charsUtf8[5] & 0x3F);
			}
			break;
		default:
			break;
	}
	return r;
}

UI32 NBEncoding_unicodeFromUtf16(const UI16* charsUtf16, const UI32 errValue){
	UI32 r = errValue;
	if((charsUtf16[0] & 0xD800) != 0xD800){
		r = charsUtf16[0];
	} else if((charsUtf16[1] & 0xDC00) == 0xDC00){
		r = 0x10000 + (((UI32)(charsUtf16[0] & 0x03FF)) << 10) + (UI32)(charsUtf16[1] & 0x03FF);
	}
	return r;
}

UI32 NBEncoding_unicodeFromUtf16s(const UI16* charsUtf16, const UI8 surrogatesCount, const UI32 errValue){
	UI32 r = errValue;
	if(surrogatesCount == 1){
		NBASSERT((charsUtf16[0] & 0xD800) != 0xD800)
		if((charsUtf16[0] & 0xD800) != 0xD800){
			r = charsUtf16[0];
		}
	} else if(surrogatesCount == 2){
		NBASSERT((charsUtf16[0] & 0xD800) == 0xD800)
		NBASSERT((charsUtf16[1] & 0xDC00) == 0xDC00)
		if(((charsUtf16[0] & 0xD800) == 0xD800) && ((charsUtf16[1] & 0xDC00) == 0xDC00)){
			r = 0x10000 + (((UI32)(charsUtf16[0] & 0x03FF)) << 10) + (UI32)(charsUtf16[1] & 0x03FF);
		}
	}
	return r;
}

