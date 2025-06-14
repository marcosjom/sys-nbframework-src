//
//  XUBibCadenas.c
//  MetricaMobilFramework
//
//  Created by Marcos Ortega on 10/09/13.
//  Copyright (c) 2013 logicoim@gmail.com. All rights reserved.
//

#include "nb/NBFrameworkPch.h"
#include "nb/core/NBStringsLib.h"
#include "nb/core/NBMemory.h"

//---------------------------
//Metodos de alimentacion de biblioteca
//---------------------------

void NBStringsLib_init(struct NBStringsLib* objLibrary){
	objLibrary->sizeChars	= NBLIBSTR_CHARS_BUFFER_SIZE_START;
	objLibrary->chars		= (char*) NBMemory_alloc(sizeof(char) * NBLIBSTR_CHARS_BUFFER_SIZE_START);
	objLibrary->useChars	= 1;
	objLibrary->chars[0]	= '\0'; //Por definicion, la cadena en el indice-cero es cadena-vacia.
	//
	objLibrary->sizeIndexes		= NBLIBSTR_INDEX_BUFFER_SIZE_START;
	objLibrary->indexes			= (UI32*) NBMemory_alloc(sizeof(UI32) * NBLIBSTR_INDEX_BUFFER_SIZE_START);
	objLibrary->useIndexes		= 1;
	objLibrary->indexes[0]		= 0; //Por definicion, la cadena en el indice-cero es cadena-vacia.
}

void NBStringsLib_release(struct NBStringsLib* objLibrary){
	objLibrary->sizeChars	= 0;
	objLibrary->useChars	= 0;
    NBMemory_free(objLibrary->chars);
	objLibrary->chars		= NULL;
	//
	objLibrary->sizeIndexes		= 0;
	objLibrary->useIndexes		= 0;
    NBMemory_free(objLibrary->indexes);
	objLibrary->indexes			= NULL;
}

void NBStringsLib_empty(struct NBStringsLib* objLibrary){
	objLibrary->useChars	= 1;
	objLibrary->chars[0]	= '\0'; //Por definicion, la cadena en el indice-cero es cadena-vacia.
	objLibrary->useIndexes		= 1;
	objLibrary->indexes[0]		= 0; //Por definicion, la cadena en el indice-cero es cadena-vacia.
}

UI32 NBStringsLib_indexOfStringSearchOnly(const struct NBStringsLib* objLibrary, const char* valueString, UI8* saveFoundAt){
	UI32 i; const UI32 useIndexes = objLibrary->useIndexes; NBASSERT(valueString != NULL)
	//Buscar cadena actual
	for(i=0; i<useIndexes; i++){
		//PRINTF_INFO("Valor de indice #%u de %u: %d\n", (i+1), usoIndices, objBiblioteca->indices[i]);
		if(NBStringsLib_compareStrings(&objLibrary->chars[objLibrary->indexes[i]], valueString)){
			//PRINTF_INFO("Cadena '%s' encontrada en indice %d.\n", valorCadena, i);
			*saveFoundAt = 1;
			return i;
		}
	}
	*saveFoundAt = 0;
	return 0;
}

UI32 NBStringsLib_indexOfString(struct NBStringsLib* objLibrary, const char* valueString){
	UI32 i; const UI32 useIndexes = objLibrary->useIndexes; NBASSERT(valueString != NULL)
	//Buscar cadena actual
	for(i=0; i<useIndexes; i++){
		//PRINTF_INFO("Valor de indice #%u de %u: %d\n", (i+1), usoIndices, objBiblioteca->indices[i]);
		if(NBStringsLib_compareStrings(&objLibrary->chars[objLibrary->indexes[i]], valueString)){
			//PRINTF_INFO("Cadena '%s' encontrada en indice %d.\n", valorCadena, i);
			return i;
		}
	}
	//Registrar indice
	if(objLibrary->useIndexes>=objLibrary->sizeIndexes){
		UI32* newBuff; UI32* curBuff; UI32 i;
		const UI32 useBuff = objLibrary->useIndexes;
		objLibrary->sizeIndexes += NBLIBSTR_INDEX_BUFFER_GROWTH_SIZE;
		newBuff = (UI32*) NBMemory_alloc(sizeof(UI32) * objLibrary->sizeIndexes);
		curBuff = objLibrary->indexes;
		for(i=0; i<useBuff; i++) newBuff[i] = curBuff[i];
	    NBMemory_free(objLibrary->indexes); objLibrary->indexes = newBuff;
	}
	objLibrary->indexes[objLibrary->useIndexes++] = objLibrary->useChars;
	//PRINTF_INFO("Cadena '%s' agregada en indice %d.\n", valorCadena, (objBiblioteca->usoIndices-1));
	//Registrar cadena y retornar indice
	do {
		if(objLibrary->useChars>=objLibrary->sizeChars) NBStringsLib_growCharBuffer(objLibrary);
		objLibrary->chars[objLibrary->useChars++] = *valueString; if(*valueString=='\0') break;
		valueString++;
	} while(1);
	//PRINTF_INFO("Despues de agregar: '%s'.\n", &objBiblioteca->caracteres[objBiblioteca->indices[objBiblioteca->usoIndices-1]]);
	//
	return (objLibrary->useIndexes-1);
}

void NBStringsLib_growCharBuffer(struct NBStringsLib* objLibrary){
	char* newBuff; char* curBuff; UI32 i;
	const UI32 useBuff = objLibrary->useChars;
	objLibrary->sizeChars += NBLIBSTR_CHARS_BUFFER_GROWTH_SIZE;
	newBuff = (char*) NBMemory_alloc(sizeof(char) * objLibrary->sizeChars);
	curBuff = objLibrary->chars;
	for(i=0; i<useBuff; i++) newBuff[i] = curBuff[i];
    NBMemory_free(objLibrary->chars); objLibrary->chars = newBuff;
}

const char* NBStringsLib_stringAtIndex(const struct NBStringsLib* objLibrary, const UI32 index){
    NBASSERT(index>=0 && index<objLibrary->useIndexes);
	if(index<objLibrary->useIndexes){
		return &objLibrary->chars[objLibrary->indexes[index]];
	}
	return "";
}

UI8 NBStringsLib_stringAtIndexBool(const struct NBStringsLib* objLibrary, const UI32 index){
    NBASSERT(index>=0 && index<objLibrary->useIndexes);
	if(index<objLibrary->useIndexes){
		const char* strValue = &objLibrary->chars[objLibrary->indexes[index]];
		if(strValue[0]=='0'){
			if(strValue[1]=='\0') return 0;
		} else if((strValue[0]=='n' || strValue[0]=='N') && (strValue[1]=='o' || strValue[1]=='O')){
			if(strValue[2]=='\0') return 0;
		}
		return 1;
	}
	return 0;
}

//---------------------------
//Metodos auxiliares
//---------------------------
UI32 NBStringsLib_stringLength(const char* string){
	UI32 size = 0; NBASSERT(string != NULL)
	while(string[size]!='\0') size++;
	return size;
}

UI32 NBStringsLib_copyString(char* destination, const char* string){
	UI32 i = 0;
    NBASSERT(destination != NULL)
    NBASSERT(string != NULL)
	do { destination[i] = string[i]; } while(string[i++]!='\0');
	return (i - 1);
}

UI32 NBStringsLib_compareStrings(const char* string1, const char* string2){
	UI32 result = 1;
	if(string1 == NULL || string2 == NULL) return 0; //Validar nulos
	//PRINTF_INFO("Comparando cadenas: '%s' vs '%s'\n", cadena1, cadena2);
	//comparas caracteres
	//comparas caracteres
	while(result && (*string1)!=0 && (*string2)!=0){
		if((*string1)!=(*string2)){
			result = 0;
		}
		string1++;	//mover puntero al siguiente caracter
		string2++;	//mover puntero al siguiente caracter
	}
	//validar si no se realizo la compracion de todos los caracteres
	//(una cadena es mas corta que la otra)
	if(result){
		result = ((*string1)==(*string2)); //la condicional es falso si uno es '\0' y el otro es diferente
	}
	return result;
}

char* NBStringsLib_createCopyString(const char* string){
	UI32 sizeString; char* stringD;
    NBASSERT(string != NULL)
	sizeString	= NBStringsLib_stringLength(string);
	stringD		= (char*) NBMemory_alloc(sizeof(char) * (sizeString + 1));
    NBStringsLib_copyString(stringD, string);
	return stringD;
}

char* NBStringsLib_loadString(FILE* objFile){
    NBASSERT(objFile != NULL)
	if(objFile != NULL){
		UI32 sizeString; char* string;
		sizeString = 0; fread(&sizeString, sizeof(sizeString), 1, objFile); NBASSERT(sizeString<2048)
		string = (char*) NBMemory_alloc(sizeof(char) * (sizeString + 1));
		if(sizeString!=0) fread(string, sizeof(char), sizeString, objFile);
		string[sizeString] = '\0';
		return string;
	}
	return NULL;
}

void NBStringsLib_saveString(const char* string, FILE* objFile){
    NBASSERT(objFile != NULL)
	if(objFile != NULL){
		UI32 sizeString = NBStringsLib_stringLength(string); NBASSERT(sizeString<(1024*1024))
		fwrite(&sizeString, sizeof(sizeString), 1, objFile);
		if(sizeString!=0) fwrite(string, sizeof(char), sizeString, objFile);
	}
}

void NBStringsLib_saveToFile(struct NBStringsLib* objLibrary, FILE* objFile){
    NBASSERT(objFile != NULL)
	if(objFile != NULL){
		UI32 valVerifStart	= NBLIBSTR_FILE_ID_START;
		UI32 valVerifEnd	= NBLIBSTR_FILE_ID_END;
		UI32 useIndexes		= objLibrary->useIndexes;
		UI32 useChars	= objLibrary->useChars;
		fwrite(&valVerifStart, sizeof(valVerifStart), 1, objFile);
		fwrite(&useIndexes, sizeof(useIndexes), 1, objFile);
		fwrite(&useChars, sizeof(useChars), 1, objFile);
		fwrite(objLibrary->indexes, sizeof(UI32), useIndexes, objFile);
		fwrite(objLibrary->chars, sizeof(char), useChars, objFile);
		fwrite(&valVerifEnd, sizeof(valVerifEnd), 1, objFile);
	}
}

UI32 NBStringsLib_loadFromFile(struct NBStringsLib* objLibrary, FILE* objFile){
    NBASSERT(objFile != NULL)
	if(objFile != NULL){
		UI32 valVerif; fread(&valVerif, sizeof(valVerif), 1, objFile);
		if(valVerif == NBLIBSTR_FILE_ID_START){
			UI32 useIndexes, useChars; 
			fread(&useIndexes, sizeof(useIndexes), 1, objFile);
			fread(&useChars, sizeof(useChars), 1, objFile);
			//PRINTF_INFO("Cargando %d indices, %d caracteres ...\n", usoIndices, usoCaracteres);
			if(useIndexes < NBBIBSTR_INDEXES_BUFFER_GROWTH_MAX && useChars < NBBIBSTR_CHARS_BUFFER_GROWTH_MAX) {
				UI32 i, indexesValids;
				if(objLibrary->sizeIndexes<useIndexes){
					objLibrary->sizeIndexes = useIndexes + NBLIBSTR_INDEX_BUFFER_GROWTH_SIZE;
				    NBMemory_free(objLibrary->indexes); objLibrary->indexes = (UI32*) NBMemory_alloc(sizeof(UI32) * objLibrary->sizeIndexes);
				}
				fread(objLibrary->indexes, sizeof(UI32), useIndexes, objFile);
				//Verificar valores de indices
				indexesValids = 1;
				for(i=0; i<useIndexes; i++){
					if(objLibrary->indexes[i]>=useChars){
						PRINTF_ERROR("No se pudo cargar indices, #%u de %u esta fuera de rango (%u de maximo %u)\n", (i+1), useIndexes, objLibrary->indexes[i], useChars);
						indexesValids = 0; break;
					}
				}
				//Por definicion, el primer indice debe ser cero
				if(indexesValids && objLibrary->indexes[0]==0){
					if(objLibrary->sizeChars<useChars){
						objLibrary->sizeChars = useChars + NBLIBSTR_CHARS_BUFFER_GROWTH_SIZE;
					    NBMemory_free(objLibrary->chars); objLibrary->chars = (char*) NBMemory_alloc(sizeof(char) * objLibrary->sizeChars);
					}
					fread(objLibrary->chars, sizeof(char), useChars, objFile);
					//Por definicion, la primer cadena es cadena-vacia
					if(objLibrary->chars[0]=='\0'){
						fread(&valVerif, sizeof(valVerif), 1, objFile);
						if(valVerif == NBLIBSTR_FILE_ID_END){
							objLibrary->useIndexes = useIndexes;
							objLibrary->useChars = useChars;
							//PRINTF_INFO("... %d indices, %d caracteres ... cargados\n", usoIndices, usoCaracteres);
							return 1;
						}
					}
				}
			}
		}
	}
    NBStringsLib_empty(objLibrary);
	return 0;
}
