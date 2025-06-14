//
//  XUXml.c
//  XUMonitorLogicaNegocios_OSX
//
//  Created by Marcos Ortega on 21/01/14.
//  Copyright (c) 2014 XurpasLatam. All rights reserved.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBNumParser.h"
#include "nb/core/NBMemory.h"

//

/*typedef enum ENNumericTypeSub_ {
	ENNumericTypeSub_Int = 0,		//integer signed
	ENNumericTypeSub_IntU,			//integer unsigned
	ENNumericTypeSub_Long,			//long signed
	ENNumericTypeSub_LongU,			//long unsigned
	ENNumericTypeSub_LongLong,		//long long signed
	ENNumericTypeSub_LongLongU,		//long long unsigned
	ENNumericTypeSub_Float,			//float
	ENNumericTypeSub_Double,		//double
	ENNumericTypeSub_DoubleLong,	//long double
	ENNumericTypeSub_Count
} ENNumericTypeSub;*/

const char* __NBNumericTypeSubDefs[] = {
	"int",			//integer signed
	"uint",			//integer unsigned
	"long",			//long signed
	"ulong",		//long unsigned
	"longlong",		//long long signed
	"ulonglong",	//long long unsigned
	"float",		//float
	"double",		//double
	"doublelong",	//long double
};

const char* NBNumericTypeSub_getNameByUid(const ENNumericTypeSub uid){
	NBASSERT((sizeof(__NBNumericTypeSubDefs) / sizeof(__NBNumericTypeSubDefs[0])) == ENNumericTypeSub_Count)
	return (uid >= 0 && uid < ENNumericTypeSub_Count ? __NBNumericTypeSubDefs[uid] : NULL);
} 

//

void NBNumParser_init(STNBNumParser* obj){
	obj->isErr			= FALSE;
	//
	obj->charsTotal		= 0;
	obj->charsPrefx		= 0;
	obj->charsBody		= 0;
	obj->charsSufix		= 0;
	//
	obj->charsBodyExp	= 0;
	obj->charsBodyExpDigits = 0;
	obj->implicitExp	= 0;
	obj->isExpNeg		= 0;
	obj->partInt		= 0;
	obj->partExp		= 0;
	//
	obj->format			= ENNumericFormat_Count;
	obj->type			= ENNumericType_Count;
	obj->typeSub		= ENNumericTypeSub_Count;
}

void NBNumParser_release(STNBNumParser* obj){
	//Nothing
}

/*void NBNumParser_printf(const STNBNumParser* state){
	/ *if(!state->isErr){
	 switch (state->typeSub) {
	 case ENNumericTypeSub_Int: PRINTF_INFO("Value(%d).\n", state->valInt); break;
	 case ENNumericTypeSub_IntU: PRINTF_INFO("Value(%u).\n", state->valIntU); break;
	 case ENNumericTypeSub_Long: PRINTF_INFO("Value(%ld).\n", state->valLong); break;
	 case ENNumericTypeSub_LongU: PRINTF_INFO("Value(%lu).\n", state->valLongU); break;
	 case ENNumericTypeSub_LongLong: PRINTF_INFO("Value(%lld).\n", state->valLongLong); break;
	 case ENNumericTypeSub_LongLongU: PRINTF_INFO("Value(%llu).\n", state->valLongLongU); break;
	 case ENNumericTypeSub_Float: PRINTF_INFO("Value(%f).\n", state->valFloat); break;
	 case ENNumericTypeSub_Double: PRINTF_INFO("Value(%f).\n", state->valDouble); break;
	 case ENNumericTypeSub_DoubleLong: PRINTF_INFO("Value(%Lf).\n", state->valDoubleLong); break;
	 default: PRINTF_INFO("Value(unknown).\n"); break;
	 }
	 }* /
	PRINTF_INFO("Format(%s) Type(%s) subType(%s) ... intPart(%llu) expImplicit(-%d) expExplicit(%s%d).\n", STR_NumericFormat(state->format), STR_NumericType(state->type), STR_NumericTypeSub(state->typeSub), state->partInt, (SI32)state->implicitExp, (state->isExpNeg ? "-" : "+"), (SI32)state->partExp);
}*/

//
//ASCII 48 = '0' ... 57='9'
//ASCII 65 = 'A' ... 70 = 'F'
//ASCII 97 = 'a' ... 102 = 'f'
BOOL NBNumParser_feedSufix(STNBNumParser* state, const char c){
	BOOL r = FALSE;
	//Move the only zero from the prefix to the body
	if(state->charsPrefx == 1 && state->charsBody == 0 && state->charsSufix == 0){
		state->charsPrefx	= 0;
		state->charsBody	= 1;
		state->format		= ENNumericFormat_DigSeq;
		state->type			= ENNumericType_Integer;
	}
	//Process posible sufix
	if(state->charsBody != 0){
		switch(c){
			case 'f': case 'F':
				if(state->type == ENNumericType_Floating){
					if(state->typeSub == ENNumericTypeSub_Count){
						state->typeSub = ENNumericTypeSub_Float;
						state->charsSufix++;
						r = TRUE;
					} else {
						//NBASSERT(FALSE) //More than one sufix
					}
				} else {
					//NBASSERT(FALSE) //Invalid sufix
				}
				break;
			case 'u': case 'U':
				if(state->type == ENNumericType_Integer){
					if(state->typeSub == ENNumericTypeSub_Count){
						state->typeSub = ENNumericTypeSub_IntU;
						state->charsSufix++;
						r = TRUE;
					} else if(state->typeSub == ENNumericTypeSub_Long){
						state->typeSub = ENNumericTypeSub_LongU;
						state->charsSufix++;
						r = TRUE;
					} else if(state->typeSub == ENNumericTypeSub_LongLong){
						state->typeSub = ENNumericTypeSub_LongLongU;
						state->charsSufix++;
						r = TRUE;
					} else {
						//NBASSERT(FALSE) //More than one sufix
					}
				} else {
					//NBASSERT(FALSE) //Invalid sufix
				}
				break;
			case 'l': case 'L':
				if(state->type == ENNumericType_Floating){
					if(state->typeSub == ENNumericTypeSub_Count){
						state->typeSub = ENNumericTypeSub_DoubleLong;
						state->charsSufix++;
						r = TRUE;
					} else {
						//NBASSERT(FALSE) //More than one sufix
					}
				} else if(state->type == ENNumericType_Integer){
					if(state->typeSub == ENNumericTypeSub_Count){
						state->typeSub = ENNumericTypeSub_Long;
						state->charsSufix++;
						r = TRUE;
					} else if(state->typeSub == ENNumericTypeSub_IntU){
						state->typeSub = ENNumericTypeSub_LongU;
						state->charsSufix++;
						r = TRUE;
					} else if(state->typeSub == ENNumericTypeSub_Long){
						state->typeSub = ENNumericTypeSub_LongLong;
						state->charsSufix++;
						r = TRUE;
					} else if(state->typeSub == ENNumericTypeSub_LongU){
						state->typeSub = ENNumericTypeSub_LongLongU;
						state->charsSufix++;
						r = TRUE;
					} else {
						//NBASSERT(FALSE) //Invalid sufix
					}
				} else {
					//NBASSERT(FALSE) //Invalid type
				}
				break;
			default:
				//NBASSERT(FALSE)
				break;
		}
	} else {
	    //NBASSERT(FALSE) //Sufix without body
	}
	return r;
}
//ASCII 48='0' ... 57='9'
//ASCII 65 = 'A' ... 70 = 'F'
//ASCII 97 = 'a' ... 102 = 'f'
BOOL NBNumParser_feedByte(STNBNumParser* state, const char c){
	BOOL r = FALSE;
	if(state != NULL){
		if(!state->isErr){
			r = TRUE;
			if(state->charsBody == 0 && state->format == ENNumericFormat_Count){
				//
				//Reading prefix
				//
				if(state->charsPrefx == 0) {
					//First char
					switch(c){
						case '0':
							state->charsPrefx++; //Octal, hexadecimal or binary start
							break;
						case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': //Integer or float start (base-10)
							state->charsBody++;
							state->format	= ENNumericFormat_DigSeq;
							state->type		= ENNumericType_Integer;
							state->partInt	= (c - '0');
							break;
						case '.': //Float start (no integer part)
							state->charsBody++;
							state->format	= ENNumericFormat_DigSeq;
							state->type		= ENNumericType_Floating;
							break;
						default:
							r = FALSE; //NBASSERT(FALSE) //First char should be digit or '.'
							break;
					}
				} else if(state->charsPrefx == 1){
					//Second char
					switch(c){
						case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': //Octal confirmed
							state->charsBody++;
							state->format	= ENNumericFormat_Octal;
							state->type		= ENNumericType_Integer;
							//NBASSERT(state->partInt == 0)
							state->partInt	= (c - '0');
							break;
						case '.': //Float starting with zero ('0.')
							state->charsPrefx--;	//Move the zero from prefix
							state->charsBody += 2;	//to body
							state->format	= ENNumericFormat_DigSeq;
							state->type		= ENNumericType_Floating;
							break;
						case 'x': case 'X': //Hexadecimal confirmed
							state->charsPrefx++;
							state->format	= ENNumericFormat_Hexadec;
							state->type		= ENNumericType_Integer;
							break;
						case 'b': case 'B': //Binary
							state->charsPrefx++;
							state->format	= ENNumericFormat_Binary;
							state->type		= ENNumericType_Integer;
							break;
						case 'u': case 'U':
						case 'l': case 'L':
							if(!NBNumParser_feedSufix(state, c)){
								r = FALSE; //NBASSERT(FALSE) //Invalid sufix
							}
							break;
						default:
							r = FALSE; //NBASSERT(FALSE) //Second char should be digit, '.', 'x', 'X', 'b', 'B' or sufix
							break;
					}
				} else {
					r = FALSE; //NBASSERT(FALSE) //This code's logic error?
				}
			} else if(state->charsSufix != 0){
				//
				//Reading sufix
				//
				switch(c){
					case 'f': case 'F':
					case 'u': case 'U':
					case 'l': case 'L':
						if(!NBNumParser_feedSufix(state, c)){
							r = FALSE; //NBASSERT(FALSE) //Invalid sufix
						}
						break;
					default:
						r = FALSE; //NBASSERT(FALSE) //Char must be a sufix
						break;
				}
			} else {
				//
				//Reading body
				//
			    NBASSERT(state->charsSufix == 0)
			    NBASSERT(state->type == ENNumericType_Integer || state->type == ENNumericType_Floating)
			    NBASSERT(state->format >= 0 && state->format < ENNumericFormat_Count)
				switch(state->format){
					case ENNumericFormat_DigSeq:
						if(state->charsBodyExp == 0){
							//Reading body
							switch(c){
								case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
									state->partInt = (state->partInt * 10) + (c - '0');
									if(state->type == ENNumericType_Floating){
										state->implicitExp++; //each decimal digit is equivalent to 'e-1'
									}
									state->charsBody++;
									break;
								case '.':
									if(state->type == ENNumericType_Integer){
										state->type = ENNumericType_Floating;
									} else {
										r = FALSE; //NBASSERT(FALSE) //Second '.' found
									}
									state->charsBody++;
									break;
								case 'e': case 'E':
									state->type = ENNumericType_Floating;
									state->charsBodyExp++;
									state->charsBody++;
									break;
								case 'f': case 'F':
								case 'u': case 'U':
								case 'l': case 'L':
									if(!NBNumParser_feedSufix(state, c)){
										r = FALSE; //NBASSERT(FALSE) //Invalid sufix
									}
									break;
								default:
									r = FALSE; //NBASSERT(FALSE) //Char must be digit, '.', 'e', 'E' or sufix
									break;
							}
						} else {
							//Reading body's exponent
							switch(c){
								case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
									//Process exponent part
									state->partExp = (state->partExp * 10) + (c - '0');
									state->charsBodyExpDigits++;
									state->charsBodyExp++;
									state->charsBody++;
									break;
								case '-': case '+':
									if(state->charsBodyExp == 1){
										state->isExpNeg = (c == '-');
										state->charsBodyExp++;
										state->charsBody++;
									} else {
										r = FALSE; //NBASSERT(FALSE) //the exponent sign must be the second char (after the 'e')
									}
									break;
								case 'f': case 'F':
								case 'u': case 'U':
								case 'l': case 'L':
									if(!NBNumParser_feedSufix(state, c)){
										r = FALSE; //NBASSERT(FALSE) //Invalid sufix
									}
									break;
								default:
									r = FALSE; //NBASSERT(FALSE) //Char must be digit, '.', '-', '+' or sufix
									break;
							}
						}
						break;
					case ENNumericFormat_Hexadec:
						if(state->charsBodyExp == 0){
							//Reading body
							switch(c){
								case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
									state->partInt = (state->partInt * 0x10) + (c - '0');
									if(state->type == ENNumericType_Floating){
										state->implicitExp += 4; //each hexadecimal digit is equivalent to 'e-4'
									}
									state->charsBody++;
									break;
								case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
									state->partInt = (state->partInt * 0x10) + (c - 'a' + 10);
									if(state->type == ENNumericType_Floating){
										state->implicitExp += 4; //each hexadecimal digit is equivalent to 'e-4'
									}
									state->charsBody++;
									break;
								case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
									state->partInt = (state->partInt * 0x10) + (c - 'A' + 10);
									if(state->type == ENNumericType_Floating){
										state->implicitExp += 4; //each hexadecimal digit is equivalent to 'e-4'
									}
									state->charsBody++;
									break;
								case '.':
									if(state->type == ENNumericType_Integer){
										state->type = ENNumericType_Floating;
									} else {
										r = FALSE; //NBASSERT(FALSE) //Second '.' found
									}
									state->charsBody++;
									break;
								case 'p': case 'P':
									state->type = ENNumericType_Floating;
									state->charsBodyExp++;
									state->charsBody++;
									break;
								case 'u': case 'U': //unsigned (if integer)
								case 'l': case 'L': //long (if integer), long double (if floating-point)
									if(!NBNumParser_feedSufix(state, c)){
										r = FALSE; //NBASSERT(FALSE) //Invalid sufix
									}
									break;
								default:
									r = FALSE; //NBASSERT(FALSE) //Char must be digit, '.', 'p', 'P' or sufix
									break;
							}
						} else {
							//Reading body's exponent
							switch(c){
								case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
									state->partExp = (state->partExp * 10) + (c - '0');
									state->charsBodyExpDigits++;
									state->charsBodyExp++;
									state->charsBody++;
									break;
								case '-': case '+':
									if(state->charsBodyExp == 1){
										state->isExpNeg = (c == '-');
										state->charsBodyExp++;
										state->charsBody++;
									} else {
										r = FALSE; //NBASSERT(FALSE) //the exponent sign must be the second char (after the 'p')
									}
									break;
								case 'f': case 'F': //float
								case 'l': case 'L': //long double
									if(!NBNumParser_feedSufix(state, c)){
										r = FALSE; //NBASSERT(FALSE) //Invalid sufix
									}
									break;
								default:
									r = FALSE; //NBASSERT(FALSE) //Char must be digit, '.', '-', '+' or sufix
									break;
							}
						}
						break;
					case ENNumericFormat_Octal:
					    NBASSERT(state->type == ENNumericType_Integer) //Never floating-point octals
						//Reading body
						switch(c){
							case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
								//Process integer part
								state->partInt = (state->partInt * 8) + (c - '0');
								state->charsBody++;
								break;
							case 'u': case 'U':
							case 'l': case 'L':
								if(!NBNumParser_feedSufix(state, c)){
									r = FALSE; //NBASSERT(FALSE) //Invalid sufix
								}
								break;
							default:
								r = FALSE; //NBASSERT(FALSE) //Char must be digit, '.', 'e', 'E' or sufix
								break;
						}
						break;
					case ENNumericFormat_Binary:
						//Reading body
						switch(c){
							case '0': case '1':
								//Process integer part
								state->partInt = (state->partInt * 2) + (c - '0');
								state->charsBody++;
								break;
							case 'u': case 'U':
							case 'l': case 'L':
								if(!NBNumParser_feedSufix(state, c)){
									r = FALSE; //NBASSERT(FALSE) //Invalid sufix
								}
								break;
							default:
								r = FALSE; //NBASSERT(FALSE) //Char must be digit, '.', 'e', 'E' or sufix
								break;
						}
						break;
					default:
						r = FALSE; //NBASSERT(FALSE) //Logic error
						break;
				}
			}
			//Add char count
			state->charsTotal++;
			//Set error flag
			if(!r) state->isErr = TRUE;
		}
	}
	return r;
}

//

BOOL NBNumParser_feed(STNBNumParser* state, const char* str){
	BOOL r = FALSE;
	if(state != NULL && str != NULL){
		if(!state->isErr){
			r = TRUE;
			const char* c = str;
			while(*c != '\0'){
				if(!NBNumParser_feedByte(state, *c)){
					r = FALSE;
					break;
				}
				c++;
			} //while
		    NBASSERT(state->isErr == !r)
		}
	}
	return r;
}

//

BOOL NBNumParser_feedBytes(STNBNumParser* state, const char* str, const SI32 len){
	BOOL r = FALSE;
	if(state != NULL && str != NULL){
		if(!state->isErr){
			r = TRUE;
			const char* c = str;
			const char* cEnd = str + len;
			while(c < cEnd){
				if(!NBNumParser_feedByte(state, *c)){
					r = FALSE;
					break;
				}
				c++;
			} //while
		    NBASSERT(state->isErr == !r)
		}
	}
	return r;
}

//

BOOL NBNumParser_hasValidEnd(const STNBNumParser* state, ENNumericType* dstType){
	BOOL r = FALSE;
	//NBASSERT(!state->isErr)
	if(!state->isErr){
		UI8 charsPrefx			= state->charsPrefx;
		UI8 charsBody			= state->charsBody;
		ENNumericFormat format	= state->format;
		ENNumericType type		= state->type;
		//If content is only a zero, move it from prefix to body
		if(charsPrefx == 1 && charsBody == 0 && state->charsSufix == 0){
			charsPrefx	= 0;
			charsBody	= 1;
			format		= ENNumericFormat_DigSeq;
			type		= ENNumericType_Integer;
		}
		//
		//NBASSERT(state->charsTotal == (charsPrefx + charsBody + state->charsSufix))
		//NBASSERT(charsBody > 0)
		//NBASSERT(type == ENNumericType_Integer || type == ENNumericType_Floating)
		if(charsBody > 0 && (type == ENNumericType_Integer || type == ENNumericType_Floating)){
			if(type == ENNumericType_Floating){
				//Octal and binary floating-points are not supported
			    NBASSERT(format == ENNumericFormat_Hexadec || format == ENNumericFormat_DigSeq)
				if(format == ENNumericFormat_Hexadec){
					//Floating hexadecimal (exponent is not optional)
					//NBASSERT(state->charsBodyExp != 0 && state->charsBodyExpDigits != 0)
					r = (state->charsBodyExp != 0 && state->charsBodyExpDigits != 0);
				} else if(format == ENNumericFormat_DigSeq){
					//Floating digits (no exponent defined or exponent must have digits)
					//NBASSERT(state->charsBodyExp == 0 || state->charsBodyExpDigits != 0)
					r = (charsBody > 1 /*not only the decimal point*/ && (state->charsBodyExp == 0 || state->charsBodyExpDigits != 0));
				} else {
					r = FALSE; NBASSERT(FALSE) //unsupported floating-point format
				}
			} else if(type == ENNumericType_Integer){
				//Integer
			    NBASSERT(state->charsBodyExp == 0)
			    NBASSERT(state->charsBodyExpDigits == 0)
				r = TRUE;
			} else {
				//undefined type?
			}
		}
		if(dstType != NULL) *dstType = type;
	}
	return r;
}

//

BOOL NBNumParser_end(STNBNumParser* state){
	BOOL r = FALSE;
	//NBASSERT(!state->isErr)
	if(!state->isErr){
		//If content is only a zero, move it from prefix to body
		if(state->charsPrefx == 1 && state->charsBody == 0 && state->charsSufix == 0){
			state->charsPrefx	= 0;
			state->charsBody	= 1;
			state->format		= ENNumericFormat_DigSeq;
			state->type			= ENNumericType_Integer;
		}
		//
	    NBASSERT(state->charsTotal == (state->charsPrefx + state->charsBody + state->charsSufix))
	    //NBASSERT(state->charsBody > 0 && (state->type == ENNumericType_Integer || state->type == ENNumericType_Floating)) //testing
		if(state->charsBody > 0 && (state->type == ENNumericType_Integer || state->type == ENNumericType_Floating)){
			if(state->type == ENNumericType_Floating){
				//Octal and binary floating-points are not supported
			    NBASSERT(state->format == ENNumericFormat_Hexadec || state->format == ENNumericFormat_DigSeq)
				if(state->format == ENNumericFormat_Hexadec){
					//Floating hexadecimal (exponent is not optional)
				    NBASSERT(state->charsBodyExp != 0 && state->charsBodyExpDigits != 0)
					r = (state->charsBodyExp != 0 && state->charsBodyExpDigits != 0);
				} else if(state->format == ENNumericFormat_DigSeq){
					//Floating digits (no exponent defined or exponent must have digits)
				    //NBASSERT(state->charsBody > 1 && state->charsBodyExp == 0 || state->charsBodyExpDigits != 0)
					r = (state->charsBody > 1 && (state->charsBodyExp == 0 || state->charsBodyExpDigits != 0));
				} else {
					r = FALSE; NBASSERT(FALSE) //unsupported floating-point format
				}
			} else if(state->type == ENNumericType_Integer){
				//Integer
			    NBASSERT(state->charsBodyExp == 0)
			    NBASSERT(state->charsBodyExpDigits == 0)
				r = TRUE;
			} else {
				r = FALSE; NBASSERT(FALSE) //undefined type?
			}
			//Autodetect subtype (if not specificated or number doesnt fit)
		    //NBASSERT(r)
			if(r){
				if(state->typeSub == ENNumericTypeSub_Count || state->type == ENNumericType_Integer){
					if(state->type == ENNumericType_Floating){
						//All floating points with no-sufix are 'double'
						state->typeSub = ENNumericTypeSub_Double;
					} else {
						if(state->format == ENNumericFormat_Hexadec || state->format == ENNumericFormat_Octal || state->format == ENNumericFormat_Binary || state->typeSub == ENNumericTypeSub_IntU || state->typeSub == ENNumericTypeSub_LongU){
							//Determine the best unsigned-size
							if(state->typeSub != ENNumericTypeSub_LongLongU){
								UI64 maxUnsigned = 0xFFull; SI32 bytesUsed = 1;
								while(bytesUsed < sizeof(unsigned int)){ maxUnsigned = (maxUnsigned << 8) + 0xFFull; bytesUsed++; }
								if(state->partInt <= maxUnsigned){
									if(state->typeSub != ENNumericTypeSub_LongU){
										state->typeSub = ENNumericTypeSub_IntU;
									}
								} else {
									while(bytesUsed < sizeof(unsigned long)){ maxUnsigned = (maxUnsigned << 8) + 0xFFull; bytesUsed++; }
									if(state->partInt <= maxUnsigned){
										state->typeSub = ENNumericTypeSub_LongU;
									} else {
										state->typeSub = ENNumericTypeSub_LongLongU;
									}
								}
							}
						} else {
							if(state->typeSub != ENNumericTypeSub_LongLong){
								//Determine the best signed-size
								UI64 maxUnsigned = 0xFFull; SI32 bytesUsed = 1;
								while(bytesUsed < sizeof(unsigned int)){ maxUnsigned = (maxUnsigned << 8) + 0xFFull; bytesUsed++; }
								if(state->partInt <= (maxUnsigned / 2ull)){
									if(state->typeSub != ENNumericTypeSub_Long){
										state->typeSub = ENNumericTypeSub_Int;
									}
								} else {
									while(bytesUsed < sizeof(unsigned long)){ maxUnsigned = (maxUnsigned << 8) + 0xFFull; bytesUsed++; }
									if(state->partInt <= (maxUnsigned / 2ull)){
										state->typeSub = ENNumericTypeSub_Long;
									} else {
										state->typeSub = ENNumericTypeSub_LongLong;
									}
								}
							}
						}
					}
				}
				//Set value
			    NBASSERT(state->typeSub != ENNumericTypeSub_Count)
				switch (state->typeSub) {
					case ENNumericTypeSub_Int: state->valInt = (int)state->partInt; break;
					case ENNumericTypeSub_IntU: state->valIntU = (unsigned int)state->partInt; break;
					case ENNumericTypeSub_Long: state->valLong = (long)state->partInt; break;
					case ENNumericTypeSub_LongU: state->valLongU = (unsigned long)state->partInt; break;
					case ENNumericTypeSub_LongLong: state->valLongLong = (long long)state->partInt; break;
					case ENNumericTypeSub_LongLongU: state->valLongLongU = (unsigned long long)state->partInt; break;
					case ENNumericTypeSub_Float:
					{
						SI32 negExp = (SI32)(state->isExpNeg ? -state->partExp : state->partExp) - (SI32)state->implicitExp;
						if(negExp == 0){
							state->valFloat = (float)state->partInt;
							//PRINTF_INFO("Usando parte entera intacta (%s%d).\n", (state->isExpNeg ? "-" : "+"), state->partExp);
						} else if(negExp > 0){
							float mExp = 1.0f; const float baseExp = (state->format == ENNumericFormat_Hexadec ? 2.f : 10.f);
							while(negExp-- != 0) mExp *= baseExp;
							state->valFloat = (float)state->partInt * mExp;
						} else {
						    NBASSERT(negExp < 0)
							float mExp = 1.0f; const float baseExp = (state->format == ENNumericFormat_Hexadec ? 2.f : 10.f);
							while(negExp++ != 0) mExp *= baseExp;
							state->valFloat = (float)state->partInt / mExp;
						}
					}
						break;
					case ENNumericTypeSub_Double:
					{
						SI32 negExp = (SI32)(state->isExpNeg ? -state->partExp : state->partExp) - (SI32)state->implicitExp;
						if(negExp == 0){
							state->valDouble = (double)state->partInt;
						} else if(negExp > 0){
							double mExp = 1.0f; const double baseExp = (state->format == ENNumericFormat_Hexadec ? 2.f : 10.f);
							while(negExp-- != 0) mExp *= baseExp;
							state->valDouble = (double)state->partInt * mExp;
						} else {
						    NBASSERT(negExp < 0)
							double mExp = 1.0f; const double baseExp = (state->format == ENNumericFormat_Hexadec ? 2.f : 10.f);
							while(negExp++ != 0) mExp *= baseExp;
							state->valDouble = (double)state->partInt / mExp;
						}
					}
						break;
					case ENNumericTypeSub_DoubleLong:
					{
						SI32 negExp = (SI32)(state->isExpNeg ? -state->partExp : state->partExp) - (SI32)state->implicitExp;
						if(negExp == 0){
							state->valDoubleLong = (long double)state->partInt;
						} else if(negExp > 0){
							long double mExp = 1.0f; const long double baseExp = (state->format == ENNumericFormat_Hexadec ? 2.f : 10.f);
							while(negExp-- != 0) mExp *= baseExp;
							state->valDoubleLong = (long double)state->partInt * mExp;
						} else {
						    NBASSERT(negExp < 0)
							long double mExp = 1.0f; const long double baseExp = (state->format == ENNumericFormat_Hexadec ? 2.f : 10.f);
							while(negExp++ != 0) mExp *= baseExp;
							state->valDoubleLong = (long double)state->partInt / mExp;
						}
					}
						break;
					default:
						r = FALSE; state->isErr = TRUE; NBASSERT(FALSE) //Inextpected subtype
						break;
				}
			}
		}
		//Set error flag
		if(!r) state->isErr = TRUE;
	}
	return r;
}

//Static methods

STNBNumParser NBNumParser_strParseUnsigned(const char* str){
	STNBNumParser r;
	NBNumParser_init(&r);
	if(!NBNumParser_feed(&r, str)){
		r.isErr = TRUE;
	} else if(!NBNumParser_end(&r)){
		r.isErr = TRUE;
	}
	NBNumParser_release(&r);
	return r;
}

STNBNumParser NBNumParser_strParseUnsignedBytes(const char* str, const UI32 strSz){
	STNBNumParser r;
	NBNumParser_init(&r);
	if(!NBNumParser_feedBytes(&r, str, strSz)){
		r.isErr = TRUE;
	} else if(!NBNumParser_end(&r)){
		r.isErr = TRUE;
	}
	NBNumParser_release(&r);
	return r;
}

SI32 NBNumParser_toSI32(const char* str, BOOL* dstSucces){
	SI32 r = 0; BOOL succ = FALSE;
	STNBNumParser p;
	NBNumParser_init(&p);
	if(!NBNumParser_feed(&p, str)){
		succ = FALSE;
	} else if(!NBNumParser_end(&p)){
		succ = FALSE;
	} else {
		succ = TRUE;
		switch (p.typeSub) {
			case ENNumericTypeSub_Int: r = (SI32)p.valInt; break;
			case ENNumericTypeSub_IntU: r = (SI32)p.valIntU; break;
			case ENNumericTypeSub_Long: r = (SI32)p.valLong; break;
			case ENNumericTypeSub_LongU: r = (SI32)p.valLongU; break;
			case ENNumericTypeSub_LongLong: r = (SI32)p.valLongLong; break;
			case ENNumericTypeSub_LongLongU: r = (SI32)p.valLongLongU; break;
			case ENNumericTypeSub_Float: r = (SI32)p.valFloat; break;
			case ENNumericTypeSub_Double: r = (SI32)p.valDouble; break;
			case ENNumericTypeSub_DoubleLong: r = (SI32)p.valDoubleLong; break;
			default:
				succ = FALSE;
				NBASSERT(FALSE) //Inextpected subtype
				break;
		}
	}
	NBNumParser_release(&p);
	if(dstSucces != NULL) *dstSucces = succ;
	return r;
}

SI64 NBNumParser_toSI64(const char* str, BOOL* dstSucces){
	SI64 r = 0; BOOL succ = FALSE;
	STNBNumParser p;
	NBNumParser_init(&p);
	if(!NBNumParser_feed(&p, str)){
		succ = FALSE;
	} else if(!NBNumParser_end(&p)){
		succ = FALSE;
	} else {
		succ = TRUE;
		switch (p.typeSub) {
			case ENNumericTypeSub_Int: r = (SI64)p.valInt; break;
			case ENNumericTypeSub_IntU: r = (SI64)p.valIntU; break;
			case ENNumericTypeSub_Long: r = (SI64)p.valLong; break;
			case ENNumericTypeSub_LongU: r = (SI64)p.valLongU; break;
			case ENNumericTypeSub_LongLong: r = (SI64)p.valLongLong; break;
			case ENNumericTypeSub_LongLongU: r = (SI64)p.valLongLongU; break;
			case ENNumericTypeSub_Float: r = (SI64)p.valFloat; break;
			case ENNumericTypeSub_Double: r = (SI64)p.valDouble; break;
			case ENNumericTypeSub_DoubleLong: r = (SI64)p.valDoubleLong; break;
			default:
				succ = FALSE;
				NBASSERT(FALSE) //Inextpected subtype
				break;
		}
	}
	NBNumParser_release(&p);
	if(dstSucces != NULL) *dstSucces = succ;
	return r;
}

UI32 NBNumParser_toUI32(const char* str, BOOL* dstSucces){
	UI32 r = 0; BOOL succ = FALSE;
	STNBNumParser p ;
	NBNumParser_init(&p);
	if(!NBNumParser_feed(&p, str)){
		succ = FALSE;
	} else if(!NBNumParser_end(&p)){
		succ = FALSE;
	} else {
		succ = TRUE;
		switch (p.typeSub) {
			case ENNumericTypeSub_Int: r = (UI32)p.valInt; break;
			case ENNumericTypeSub_IntU: r = (UI32)p.valIntU; break;
			case ENNumericTypeSub_Long: r = (UI32)p.valLong; break;
			case ENNumericTypeSub_LongU: r = (UI32)p.valLongU; break;
			case ENNumericTypeSub_LongLong: r = (UI32)p.valLongLong; break;
			case ENNumericTypeSub_LongLongU: r = (UI32)p.valLongLongU; break;
			case ENNumericTypeSub_Float: r = (UI32)p.valFloat; break;
			case ENNumericTypeSub_Double: r = (UI32)p.valDouble; break;
			case ENNumericTypeSub_DoubleLong: r = (UI32)p.valDoubleLong; break;
			default:
				succ = FALSE;
			    NBASSERT(FALSE) //Inextpected subtype
				break;
		}
	}
	NBNumParser_release(&p);
	if(dstSucces != NULL) *dstSucces = succ;
	return r;
}

UI64 NBNumParser_toUI64(const char* str, BOOL* dstSucces){
	UI64 r = 0; BOOL succ = FALSE;
	STNBNumParser p;
	NBNumParser_init(&p);
	if(!NBNumParser_feed(&p, str)){
		succ = FALSE;
	} else if(!NBNumParser_end(&p)){
		succ = FALSE;
	} else {
		succ = TRUE;
		switch (p.typeSub) {
			case ENNumericTypeSub_Int: r = (UI64)p.valInt; break;
			case ENNumericTypeSub_IntU: r = (UI64)p.valIntU; break;
			case ENNumericTypeSub_Long: r = (UI64)p.valLong; break;
			case ENNumericTypeSub_LongU: r = (UI64)p.valLongU; break;
			case ENNumericTypeSub_LongLong: r = (UI64)p.valLongLong; break;
			case ENNumericTypeSub_LongLongU: r = (UI64)p.valLongLongU; break;
			case ENNumericTypeSub_Float: r = (UI64)p.valFloat; break;
			case ENNumericTypeSub_Double: r = (UI64)p.valDouble; break;
			case ENNumericTypeSub_DoubleLong: r = (UI64)p.valDoubleLong; break;
			default:
				succ = FALSE;
			    NBASSERT(FALSE) //Inextpected subtype
				break;
		}
	}
	NBNumParser_release(&p);
	if(dstSucces != NULL) *dstSucces = succ;
	return r;
}

UI32 NBNumParser_hexToUI32(const char* str, BOOL* dstSucces){
	UI32 r = 0; BOOL succ = FALSE;
	if(str != 0){
		if(*str != '\0'){
			//First char
			const char c = *str;
			if((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')){
				r = (c < 'A' ? c - '0' : c < 'a' ? 10 + (c - 'A') : 10 + (c - 'a')); str++;
			} else {
				succ = TRUE;
				//Others
				while(*str != '\0'){
					const char c = *str;
					if((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')){
						r = (r << 4) | (c < 'A' ? c - '0' : c < 'a' ? 10 + (c - 'A') : 10 + (c - 'a')); str++;
					} else {
						succ = FALSE;
						break;
					}
				}
			}
		}
	}
	if(dstSucces != NULL) *dstSucces = succ;
	return r;
}

UI64 NBNumParser_hexToUI64(const char* str, BOOL* dstSucces){
	UI64 r = 0; BOOL succ = FALSE;
	if(str != 0){
		if(*str != '\0'){
			//First char
			const char c = *str;
			if((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')){
				r = (c < 'A' ? c - '0' : c < 'a' ? 10 + (c - 'A') : 10 + (c - 'a')); str++;
			} else {
				succ = TRUE;
				//Others
				while(*str != '\0'){
					const char c = *str;
					if((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')){
						r = (r << 4) | (c < 'A' ? c - '0' : c < 'a' ? 10 + (c - 'A') : 10 + (c - 'a')); str++;
					} else {
						succ = FALSE;
						break;
					}
				}
			}
		}
	}
	if(dstSucces != NULL) *dstSucces = succ;
	return r;
}

//

SI32 NBNumParser_toSI32Bytes(const char* str, const UI32 strSz, BOOL* dstSuccess){
	SI32 r = 0; BOOL succ = FALSE;
	STNBNumParser p ;
	NBNumParser_init(&p);
	if(!NBNumParser_feedBytes(&p, str, strSz)){
		succ = FALSE;
	} else if(!NBNumParser_end(&p)){
		succ = FALSE;
	} else {
		succ = TRUE;
		switch (p.typeSub) {
			case ENNumericTypeSub_Int: r = (SI32)p.valInt; break;
			case ENNumericTypeSub_IntU: r = (SI32)p.valIntU; break;
			case ENNumericTypeSub_Long: r = (SI32)p.valLong; break;
			case ENNumericTypeSub_LongU: r = (SI32)p.valLongU; break;
			case ENNumericTypeSub_LongLong: r = (SI32)p.valLongLong; break;
			case ENNumericTypeSub_LongLongU: r = (SI32)p.valLongLongU; break;
			case ENNumericTypeSub_Float: r = (SI32)p.valFloat; break;
			case ENNumericTypeSub_Double: r = (SI32)p.valDouble; break;
			case ENNumericTypeSub_DoubleLong: r = (SI32)p.valDoubleLong; break;
			default:
				succ = FALSE;
				NBASSERT(FALSE) //Inextpected subtype
				break;
		}
	}
	NBNumParser_release(&p);
	if(dstSuccess != NULL) *dstSuccess = succ;
	return r;
}

SI64 NBNumParser_toSI64Bytes(const char* str, const UI32 strSz, BOOL* dstSuccess){
	SI64 r = 0; BOOL succ = FALSE;
	STNBNumParser p ;
	NBNumParser_init(&p);
	if(!NBNumParser_feedBytes(&p, str, strSz)){
		succ = FALSE;
	} else if(!NBNumParser_end(&p)){
		succ = FALSE;
	} else {
		succ = TRUE;
		switch (p.typeSub) {
			case ENNumericTypeSub_Int: r = (SI64)p.valInt; break;
			case ENNumericTypeSub_IntU: r = (SI64)p.valIntU; break;
			case ENNumericTypeSub_Long: r = (SI64)p.valLong; break;
			case ENNumericTypeSub_LongU: r = (SI64)p.valLongU; break;
			case ENNumericTypeSub_LongLong: r = (SI64)p.valLongLong; break;
			case ENNumericTypeSub_LongLongU: r = (SI64)p.valLongLongU; break;
			case ENNumericTypeSub_Float: r = (SI64)p.valFloat; break;
			case ENNumericTypeSub_Double: r = (SI64)p.valDouble; break;
			case ENNumericTypeSub_DoubleLong: r = (SI64)p.valDoubleLong; break;
			default:
				succ = FALSE;
				NBASSERT(FALSE) //Inextpected subtype
				break;
		}
	}
	NBNumParser_release(&p);
	if(dstSuccess != NULL) *dstSuccess = succ;
	return r;
}

UI32 NBNumParser_toUI32Bytes(const char* str, const UI32 strSz, BOOL* dstSucces){
	UI32 r = 0; BOOL succ = FALSE;
	STNBNumParser p ;
	NBNumParser_init(&p);
	if(!NBNumParser_feedBytes(&p, str, strSz)){
		succ = FALSE;
	} else if(!NBNumParser_end(&p)){
		succ = FALSE;
	} else {
		succ = TRUE;
		switch (p.typeSub) {
			case ENNumericTypeSub_Int: r = (UI32)p.valInt; break;
			case ENNumericTypeSub_IntU: r = (UI32)p.valIntU; break;
			case ENNumericTypeSub_Long: r = (UI32)p.valLong; break;
			case ENNumericTypeSub_LongU: r = (UI32)p.valLongU; break;
			case ENNumericTypeSub_LongLong: r = (UI32)p.valLongLong; break;
			case ENNumericTypeSub_LongLongU: r = (UI32)p.valLongLongU; break;
			case ENNumericTypeSub_Float: r = (UI32)p.valFloat; break;
			case ENNumericTypeSub_Double: r = (UI32)p.valDouble; break;
			case ENNumericTypeSub_DoubleLong: r = (UI32)p.valDoubleLong; break;
			default:
				succ = FALSE;
				NBASSERT(FALSE) //Inextpected subtype
				break;
		}
	}
	NBNumParser_release(&p);
	if(dstSucces != NULL) *dstSucces = succ;
	return r;
}

UI64 NBNumParser_toUI64Bytes(const char* str, const UI32 strSz, BOOL* dstSucces){
	UI64 r = 0; BOOL succ = FALSE;
	STNBNumParser p;
	NBNumParser_init(&p);
	if(!NBNumParser_feedBytes(&p, str, strSz)){
		succ = FALSE;
	} else if(!NBNumParser_end(&p)){
		succ = FALSE;
	} else {
		succ = TRUE;
		switch (p.typeSub) {
			case ENNumericTypeSub_Int: r = (UI64)p.valInt; break;
			case ENNumericTypeSub_IntU: r = (UI64)p.valIntU; break;
			case ENNumericTypeSub_Long: r = (UI64)p.valLong; break;
			case ENNumericTypeSub_LongU: r = (UI64)p.valLongU; break;
			case ENNumericTypeSub_LongLong: r = (UI64)p.valLongLong; break;
			case ENNumericTypeSub_LongLongU: r = (UI64)p.valLongLongU; break;
			case ENNumericTypeSub_Float: r = (UI64)p.valFloat; break;
			case ENNumericTypeSub_Double: r = (UI64)p.valDouble; break;
			case ENNumericTypeSub_DoubleLong: r = (UI64)p.valDoubleLong; break;
			default:
				succ = FALSE;
				NBASSERT(FALSE) //Inextpected subtype
				break;
		}
	}
	NBNumParser_release(&p);
	if(dstSucces != NULL) *dstSucces = succ;
	return r;
}

UI32 NBNumParser_hexToUI32Bytes(const char* str, const UI32 strSz, BOOL* dstSucces){
	UI32 r = 0; BOOL succ = FALSE;
	if(strSz > 0){
		const char* afterEnd = str + strSz;
		//First char
		const char c = *str;
		if((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')){
			succ = TRUE;
			r = (c < 'A' ? c - '0' : c < 'a' ? 10 + (c - 'A') : 10 + (c - 'a')); str++;
			//Others
			while(str < afterEnd){
				const char c = *str;
				if((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')){
					r = (r << 4) | (c < 'A' ? c - '0' : c < 'a' ? 10 + (c - 'A') : 10 + (c - 'a')); str++;
				} else {
					succ = FALSE;
					break;
				}
			}
		}
	}
	if(dstSucces != NULL) *dstSucces = succ;
	return r;
}

UI64 NBNumParser_hexToUI64Bytes(const char* str, const UI32 strSz, BOOL* dstSucces){
	UI64 r = 0; BOOL succ = FALSE;
	if(strSz > 0){
		const char* afterEnd = str + strSz;
		//First char
		const char c = *str;
		if((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')){
			succ = TRUE;
			r = (c < 'A' ? c - '0' : c < 'a' ? 10 + (c - 'A') : 10 + (c - 'a')); str++;
			//Others
			while(str < afterEnd){
				const char c = *str;
				if((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')){
					r = (r << 4) | (c < 'A' ? c - '0' : c < 'a' ? 10 + (c - 'A') : 10 + (c - 'a')); str++;
				} else {
					succ = FALSE;
					break;
				}
			}
		}
	}
	if(dstSucces != NULL) *dstSucces = succ;
	return r;
}
