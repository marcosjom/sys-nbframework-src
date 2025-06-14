// NBBase64.c: implementation of the NBBase64 class.
//
//////////////////////////////////////////////////////////////////////

#include "nb/NBFrameworkPch.h"
#include "nb/crypto/NBBase64.h"
#include "nb/core/NBMemory.h"

static char NBBase64Tkns[] = {
	/*0*/ 'A', /*1*/ 'B', /*2*/ 'C', /*3*/ 'D', /*4*/ 'E', /*5*/ 'F', /*6*/ 'G', /*7*/ 'H', /*8*/ 'I', /*9*/ 'J',
	/*10*/ 'K', /*11*/ 'L', /*12*/ 'M', /*13*/ 'N', /*14*/ 'O', /*15*/ 'P', /*16*/ 'Q', /*17*/ 'R', /*18*/ 'S', /*19*/ 'T',
	/*20*/ 'U', /*21*/ 'V', /*22*/ 'W', /*23*/ 'X', /*24*/ 'Y', /*25*/ 'Z', /*26*/ 'a', /*27*/ 'b', /*28*/ 'c', /*29*/ 'd',
	/*30*/ 'e', /*31*/ 'f', /*32*/ 'g', /*33*/ 'h', /*34*/ 'i', /*35*/ 'j', /*36*/ 'k', /*37*/ 'l', /*38*/ 'm', /*39*/ 'n',
	/*40*/ 'o', /*41*/ 'p', /*42*/ 'q', /*43*/ 'r', /*44*/ 's', /*45*/ 't', /*46*/ 'u', /*47*/ 'v', /*48*/ 'w', /*49*/ 'x',
	/*50*/ 'y', /*51*/ 'z', /*52*/ '0', /*53*/ '1', /*54*/ '2', /*55*/ '3', /*56*/ '4', /*57*/ '5', /*58*/ '6', /*59*/ '7',
	/*60*/ '8', /*61*/ '9', /*62*/ '+', /*63*/ '/'
};

static char NBBase64InvIdxs[] = {
	-1 /*0*/, -1 /*1*/, -1 /*2*/, -1 /*3*/, -1 /*4*/, -1 /*5*/, -1 /*6*/, -1 /*7*/, -1 /*8*/, -1 /*9*/, -1 /*10*/, -1 /*11*/, -1 /*12*/, -1 /*13*/, -1 /*14*/, -1 /*15*/, -1 /*16*/, -1 /*17*/, -1 /*18*/, -1 /*19*/, -1 /*20*/, -1 /*21*/, -1 /*22*/, -1 /*23*/, -1 /*24*/, -1 /*25*/, -1 /*26*/, -1 /*27*/, -1 /*28*/, -1 /*29*/, -1 /*30*/, -1 /*31*/, -1 /*32*/,
	-1 /*33*/, -1 /*34*/, -1 /*35*/, -1 /*36*/, -1 /*37*/, -1 /*38*/, -1 /*39*/, -1 /*40*/, -1 /*41*/, -1 /*42*/, 62 /*43*/, -1 /*44*/, -1 /*45*/, -1 /*46*/, 63 /*47*/, 52 /*48*/, 53 /*49*/, 54 /*50*/, 55 /*51*/, 56 /*52*/, 57 /*53*/, 58 /*54*/, 59 /*55*/, 60 /*56*/, 61 /*57*/, -1 /*58*/, -1 /*59*/, -1 /*60*/, -1 /*61*/, -1 /*62*/, -1 /*63*/, -1 /*64*/,
	0 /*65*/, 1 /*66*/, 2 /*67*/, 3 /*68*/, 4 /*69*/, 5 /*70*/, 6 /*71*/, 7 /*72*/, 8 /*73*/, 9 /*74*/, 10 /*75*/, 11 /*76*/, 12 /*77*/, 13 /*78*/, 14 /*79*/, 15 /*80*/, 16 /*81*/, 17 /*82*/, 18 /*83*/, 19 /*84*/, 20 /*85*/, 21 /*86*/, 22 /*87*/, 23 /*88*/, 24 /*89*/, 25 /*90*/, -1 /*91*/, -1 /*92*/, -1 /*93*/, -1 /*94*/, -1 /*95*/, -1 /*96*/,
	26 /*97*/, 27 /*98*/, 28 /*99*/, 29 /*100*/, 30 /*101*/, 31 /*102*/, 32 /*103*/, 33 /*104*/, 34 /*105*/, 35 /*106*/, 36 /*107*/, 37 /*108*/, 38 /*109*/, 39 /*110*/, 40 /*111*/, 41 /*112*/, 42 /*113*/, 43 /*114*/, 44 /*115*/, 45 /*116*/, 46 /*117*/, 47 /*118*/, 48 /*119*/, 49 /*120*/, 50 /*121*/, 51 /*122*/, -1 /*123*/, -1 /*124*/, -1 /*125*/, -1 /*126*/, -1 /*127*/, -1 /*128*/
};

char NBBase64_token(const char idx){
	char r = '\0';
	if(idx >= 0 && idx < (sizeof(NBBase64Tkns) / sizeof(NBBase64Tkns[0]))){
		r = NBBase64Tkns[idx];
	}
	return r;
}

BOOL NBBase64_isToken(const char c){
	BOOL r = FALSE;
	if(c >= 0 && c < (sizeof(NBBase64InvIdxs) / sizeof(NBBase64InvIdxs[0]))){
		r = (NBBase64InvIdxs[c] >= 0);
	}
	return r;
}

void NBBase64_code3Bytes(const void* pB3, const SI32 b3Sz, char* dst4){
	NBASSERT(b3Sz > 0)
	if(b3Sz > 0){
		const BYTE* b3 = (const BYTE*)pB3;
		UI8 bytes[3] = { b3[0], 0, 0 };
		if(b3Sz > 1) bytes[1] = b3[1];
		if(b3Sz > 2) bytes[2] = b3[2];
		//
		dst4[0]	= NBBase64Tkns[((bytes[0] >> 2) & 0x3F)];
		dst4[1]	= NBBase64Tkns[((bytes[0] & 0x3) << 4) | ((bytes[1] & 0xF0) >> 4)];
		dst4[2]	= (b3Sz < 2 ? '=' : NBBase64Tkns[((bytes[1] & 0xF) << 2) | ((bytes[2] & 0xC0) >> 6)]);
		dst4[3]	= (b3Sz < 3 ? '=' : NBBase64Tkns[(bytes[2] & 0x3F)]);
	}
}

UI8 NBBase64_decode4Bytes(const char* bytes4, char* buff3Bytes){
	UI8 values[4] = {0, 0, 0, 0};
	//
	if(bytes4[0] > 64 && bytes4[0] < 91) values[0] = (bytes4[0] - 65); // 65 ... 90 - A ... Z
	else if(bytes4[0] > 96 && bytes4[0] < 123) values[0] = 26 + (bytes4[0] - 97); // 97 ... 122 - A ... Z
	else if(bytes4[0] > 47 && bytes4[0] < 58) values[0] = 52 + (bytes4[0] - 48); // 48 ... 57 - 0 ... 9
	else if(bytes4[0] == '+') values[0] = 62;
	else if(bytes4[0] == '/') values[0] = 63;
	else return 0; //Cannot be '=' neither
	//
	if(bytes4[1] > 64 && bytes4[1] < 91) values[1] = (bytes4[1] - 65); // 65 ... 90 - A ... Z
	else if(bytes4[1] > 96 && bytes4[1] < 123) values[1] = 26 + (bytes4[1] - 97); // 97 ... 122 - A ... Z
	else if(bytes4[1] > 47 && bytes4[1] < 58) values[1] = 52 + (bytes4[1] - 48); // 48 ... 57 - 0 ... 9
	else if(bytes4[1] == '+') values[1] = 62;
	else if(bytes4[1] == '/') values[1] = 63;
	else return 0; //Cannot be '=' neither
	//
	if(bytes4[2] > 64 && bytes4[2] < 91) values[2] = (bytes4[2] - 65); // 65 ... 90 - A ... Z
	else if(bytes4[2] > 96 && bytes4[2] < 123) values[2] = 26 + (bytes4[2] - 97); // 97 ... 122 - A ... Z
	else if(bytes4[2] > 47 && bytes4[2] < 58) values[2] = 52 + (bytes4[2] - 48); // 48 ... 57 - 0 ... 9
	else if(bytes4[2] == '+') values[2] = 62;
	else if(bytes4[2] == '/') values[2] = 63;
	else if(bytes4[2] != '=') return 0; //Can be '='
	//
	if(bytes4[3] > 64 && bytes4[3] < 91) values[3] = (bytes4[3] - 65); // 65 ... 90 - A ... Z
	else if(bytes4[3] > 96 && bytes4[3] < 123) values[3] = 26 + (bytes4[3] - 97); // 97 ... 122 - A ... Z
	else if(bytes4[3] > 47 && bytes4[3] < 58) values[3] = 52 + (bytes4[3] - 48); // 48 ... 57 - 0 ... 9
	else if(bytes4[3] == '+') values[3] = 62;
	else if(bytes4[3] == '/') values[3] = 63;
	else if(bytes4[3] != '=') return 0; //Can be '='
	//Last must be '=' if prev is '='
	if(bytes4[2] == '=' && bytes4[3] != '=') return 0;
	//
    NBASSERT(values[0] < 64 && values[1] < 64 && values[2] < 64 && values[3] < 64)
	//
	buff3Bytes[0]	= (values[0] << 2) | ((values[1] & 0x30) >> 4);
	buff3Bytes[1]	= ((values[1] & 0xF) << 4) | ((values[2] & 0x3C) >> 2);
	buff3Bytes[2]	= ((values[2] & 0x3) << 6) | (values[3] & 0x3F);
	//
	return (bytes4[2] == '=' && bytes4[3] == '=') ? 1 : (bytes4[3] == '=') ? 2 : 3;
}

//Code

void NBBase64_code(STNBString* dst, const char* src){
	char buffIn[3], buffOut[4]; UI8 buffInUsed = 0;
	const char* c = src;
	while(*c != '\0'){
		if(buffInUsed == 3){
		    NBBase64_code3Bytes((BYTE*)buffIn, buffInUsed, buffOut);
		    NBString_concatBytes(dst, buffOut, 4);
			buffInUsed = 0;
		}
		buffIn[buffInUsed++] = *c;
		c++;
	}
	//Flush remaining
	if(buffInUsed > 0){
	    NBBase64_code3Bytes((BYTE*)buffIn, buffInUsed, buffOut);
	    NBString_concatBytes(dst, buffOut, 4);
	}
}

void NBBase64_codeBytes(STNBString* dst, const char* src, const UI32 len){
	UI32 i; char buffOut[4];
	const UI32 blocks = (len / 3);
	const UI32 remain = (len % 3);
	for(i = 0; i < blocks; i++){
		NBBase64_code3Bytes(&src[i * 3], 3, buffOut);
		NBString_concatBytes(dst, buffOut, 4);
	}
	if(remain > 0){
		NBBase64_code3Bytes(&src[blocks * 3], remain, buffOut);
		NBString_concatBytes(dst, buffOut, 4);
	}
}


//Decode
BOOL NBBase64_decode(STNBString* dst, const char* src){
	BOOL r = TRUE;
	char buffIn[4], buffOut[3]; UI8 buffInUsed = 0;
	const char* c = src;
	do {
		buffIn[buffInUsed++] = *c;
		c++;
		//
		if(buffInUsed == 4){
			const UI8 rr = NBBase64_decode4Bytes(buffIn, buffOut);
			if(rr == 0){
				r = FALSE; NBASSERT(FALSE)
				break;
			}
			NBString_concatBytes(dst, buffOut, rr);
			buffInUsed = 0;
		}
	} while(*c != '\0');
	NBASSERT(buffInUsed == 0)
	return r;
}

BOOL NBBase64_decodeBytes(STNBString* dst, const char* src, const UI32 len){
	BOOL r = TRUE;
	char buffIn[4], buffOut[3]; UI8 buffInUsed = 0;
	const char* c = src;
	const char* cAfterLast = (src + len);
	do {
		buffIn[buffInUsed++] = *c;
		c++;
		//
		if(buffInUsed == 4){
			const UI8 rr = NBBase64_decode4Bytes(buffIn, buffOut);
			if(rr == 0){
				r = FALSE; NBASSERT(FALSE)
				break;
			}
			NBString_concatBytes(dst, buffOut, rr);
			buffInUsed = 0;
		}
	} while(c < cAfterLast);
	NBASSERT(buffInUsed == 0)
	return r;
}

