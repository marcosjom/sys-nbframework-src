
#ifndef NB_FRAMEWORK_CFG_H
#define NB_FRAMEWORK_CFG_H

//-------------------------------
//-- Definiciones de compilacion final
//-------------------------------
#define NB_COMPILE_DEVELOPMENT_VERSION		//full runtime validations
//#define NB_COMPILE_DEVELOPMENT_VERSION_FAST	//minimal runtime validations
//#define NB_COMPILE_RELEASE_VERSION			//no runtime validations

#define NB_ENABLE_OPENSSL
#define NB_LIB_SSL
#define NB_LIB_TESSERACT
#define NB_LIB_LEPTONICA
#define NB_LIB_FREETYPE
#ifdef _WIN32
//#	define NB_LIB_TESSERACT_SYSTEM
//#	define NB_LIB_LEPTONICA_SYSTEM
//# define NB_LIB_FREETYPE_SYSTEM
#endif

#define NB_MEM_LEAK_DETECT_ENABLED			//if defined, OS-specific memory leak detection is activated

#if (defined(NB_COMPILE_DEVELOPMENT_VERSION) && defined(NB_COMPILE_RELEASE_VERSION)) || (!defined(NB_COMPILE_DEVELOPMENT_VERSION) && !defined(NB_COMPILE_RELEASE_VERSION))
#	error eaxtly one must be defined: NB_COMPILE_DEVELOPMENT_VERSION or NB_COMPILE_RELEASE_VERSION
#endif

//#if defined(_KERNEL_MODE) //Visual Studio: /kernel (Create kernel mode binary)
//#	define NB_COMPILE_DRIVER_MODE				//when defined memory allocation and printf are adapted to driver environment
//#endif

#ifdef NB_COMPILE_DRIVER_MODE
#	define NB_COMPILE_DRIVER_MODE_ALLOC_TAG		'NBFR' //secure memory allocation tag
#endif

// +++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++
// ++ DEFINICIONES RECOMENDADAS PARA RELEASE
// +++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++

//Si defined, entonces todas las instrucciones "RINTF*(...)" seran reemplazadas por "(0)" en tiempo de compilacion.
//Esto reduce el tamano del ejecutable y los bloques de instrucciones/datos en memoria porque se excluyen los
//textos que se incrustran en el binario.
#ifdef NB_COMPILE_RELEASE_VERSION
#	define CONFIG_NB_DESHABILITAR_IMPRESIONES_PRINTF
#endif

//Si defined, entonces se omite el codigo integrado de OGG y VORBIS.
//#define CONFIG_NB_UNSUPPORT_OGG_FORMAT

//Si defined, entonces se omite el codigo integrado de LIBJPEG.
//#define CONFIG_NB_UNSUPPORT_INTERNAL_LIBJPEG

//Si defined, entonces se incluyen metodos heredados que permiten
//identificar si una clase es de un tipo especifico, u obtener el nombre de la ultima clase.
#define CONFIG_NB_INCLUIR_METODOS_IDENTIFICAN_CLASES_POR_ID
#define CONFIG_NB_INCLUIR_METODOS_IDENTIFICAN_CLASES_POR_NOMBRES

//Si defined, entonces se habilitan las llamadas a clock mediante la macro "CICLOS_CPU"
//Advertencia, medir los ciclos cpu aparentemente provoca que el proceso ceda el procesador, impactando significativamente en el rendimiento del programa.
#define CONFIG_NB_INCLUIR_MEDICIONES_CICLOS_CPU

//Si defined, entonces se implementan mutexes en el NBGestorMemoria y en NBGestorPilaLLamadas para evitar
//corrupcion de datos a causa de llamadas simultaneas (multihilos).
//Nota de rendimiento: los mutexes generan una carga adicional, si no se implementan multihilos es mejor no definir esta
#define CONFIG_NB_GESTION_MEMORIA_IMPLEMENTAR_MUTEX

//Define cual es el multiplo que debe respetarse para los punteros.
//Todos los punteros devueltos por malloc y NBGestorMemoria deben ser multiplo de este valor.
#if defined(__ANDROID__)
#	define CONFIG_NB_GESTION_MEMORIA_MULTIPLO_PUNTEROS 8
#else
#	define CONFIG_NB_GESTION_MEMORIA_MULTIPLO_PUNTEROS 16 //Segun documentacion de Intel, SSE2 requiere direcciones de memoria multiplo de 16
#endif

//Si defined, entonces sobrecarga los operadores "new" y "delete" de los "AUObjeto"
//para que la memoria sea reservada y liberada usando el NBGestorMemoria.
//Necesario para el seguimiento y depuracion de punteros y memoria reservada/liberada.
#define CONFIG_AUOBJETO_SOBRECARGAR_OPERADORES_NEW_DELETE

//Si defined, entonces formatea con ceros todos los bytes
//de memoria reservada mediante NBGestorMemoria::reservarMemoria
//(consume tiempo de procesador, sobre todo para bloques grandes)
#define CONFIG_NB_GESTOR_MEMORIA_VALOR_POR_DEFECTO_DE_FORMATEAR_TODOS_LOS_BLOQUES_AL_RESERVAR false

// +++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++
// ++ DEFINICIONES RECOMENDADAS PARA DEBUG GENERAL
// +++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++

//Si defined, entonces todas las validaciones NBASSERT(...) se incluyen en el codigo a compilar
#ifdef NB_COMPILE_DEVELOPMENT_VERSION
#	define CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
#	define CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT_ACCESO_MEMORIA	//Valida el acceso no simultaneo al NBGestorMemoria y NBGestorPilaLLamadas
#endif

#ifdef NB_COMPILE_DEVELOPMENT_VERSION
	//If defined, MUTEX-permanent-lock validation is implemented.
	//Note: when enabled, multithreading can be heavely impacted.
#	define CONFIG_NB_INCLUDE_THREADS_LOCKS_VALIDATION
    //If defined, per-thread-storage variables (STNBThreadStorage)
    //are tracked by the NBThreadsMngr to be manually cleaned.
    //This provides a cleaner memory-leak detection at the end of the program.
#   define CONFIG_NB_INCLUDE_THREADS_STORAGE_CLEANUP
    //Slow options
#	ifndef NB_COMPILE_DEVELOPMENT_VERSION_FAST
		//If defined, posible MUTEX-permanent-lock validation is implemented.
		//Note: when enabled, multithreading can be heavely impacted.
#		define CONFIG_NB_INCLUDE_THREADS_LOCKS_RISK_VALIDATION
		//If defined, locks/unlocks/waits/signals/broadcasts are counted.
#		define CONFIG_NB_INCLUDE_THREADS_METRICS
#	endif
#endif


// +++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++
// ++ DEFINICIONES RECOMENDADAS PARA DEBUG AVANZADO
// +++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++

//Si defined, entonces la memoria se reserva en grandes bloques clasificados en grupos.
//Las reservas individuales se realizan en base al grupo deseado.
//Esto permite asegurar cierta proximidad entre los datos, probablemente reduciendo significativamente los
//intercambios entre la memoria cache del procesador y la memoria principal.
#if !defined(__ANDROID__)
//#define CONFIG_NB_GESTOR_MEMORIA_IMPLEMENTAR_GRUPOS_ZONAS_MEMORIA
#endif

//Si defined, entonces se habilita todo el codigo referente al NBGestorPilaLlamadas
//#define CONFIG_NB_IMPLEMETAR_GESTOR_PILA_LLAMADAS

//Si defined entonces las operaciones con las estructuras de datos (como STNBPoint, NBMatriz, NBColor)
//se incrustan en las funciones que los utilizan. Esto produce binarios mas pesados incrementando
//la necesidad de cambios de cache de instrucciones del procesador. Pero a la vez incrementa los saltos
//por las llamadas a funciones.
//#define CONFIG_NB_INCRUSTAR_CODIGO_OPERACIONES_ESTRUCTURAS_DATOS //AUN NO IMPLEMETADO

//Si defined, entonces se recopilaran las estadisticas del NBGestorMemoria
//tales como: punteros reservados/liberados en tick, memoria total reservada, et...
//#define CONFIG_NB_RECOPILAR_ESTADISTICAS_DE_GESTION_MEMORIA

//Si defined, entonces se valida la estructura del indice de cada zona de memoria
//cada vez que se realiza una operacion.
//ADVERTENCIA: es extremadamente costoso en rendimiento.
//#define CONFIG_NB_GESTOR_MEMORIA_VALIDAR_INDICE_GRUPOS_ZONAS_MEMORIA_SIEMPRE

//Si defined, entonces lleva un registro de
//todos los bloques de memoria reservados mediante NBGestorMemoria::reservarMemoria
//(consume tiempo de procesador)
//#define CONFIG_NB_GESTOR_MEMORIA_REGISTRAR_BLOQUES

//Si defined, entonces reserva bytes adicionales por puntero,
//y almacena informacion para diagnostico (el nombre de cada puntero).
//#define CONFIG_NB_GESTOR_MEMORIA_GUARDAR_NOMBRES_PUNTEROS

//Si defined, entonces lleva registro de todos los AUObjetos creados.
//Permite la consultas a lista de objetos pero consume recursos de procesamiento y memoria.
//#define CONFIG_NB_GESTOR_AUOBJETOS_REGISTRAR_TODOS

//Si defined, entonces reserva bytes adicionales por puntero,
//y almacena informacion para diagnostico (los punteros de quienes lo han retenido).
//#define CONFIG_NB_GESTOR_MEMORIA_GUARDAR_PUNTEROS_RETENCIONES

//Obsoleto: Si defined, entonces cada AUObjeto reservara bytes adicionales
//para almacenar las llamadas en pila previas a cada retencion, liberacion y autoliberacion.
//NOTA: esta informacion de diagnostico se considera obsoleta, salvo casos especiales.
//NOTA: el uso de esta fue reemplazado por "CONFIG_NB_GESTOR_MEMORIA_GUARDAR_PUNTEROS_RETENCIONES"
//#define CONFIG_AU_OBJETO_GUARDAR_REFERENCIAS_DE_LLAMADAS_EN_PILA

//Si defined, entonces advierte cuando una cadena ha sido redimensionada igual o mas veces
//que la especificada en el valor de esta macro
//#define CONFIG_AU_ADVIERTE_MULTIREDIMENSIONAMIENTO_CADENAS_VECES	5

//Si defined, entonces advierte cuando un arreglo ha sido redimensionado igual o mas veces
//que la especificada en el valor de esta macro
//#define CONFIG_AU_ADVIERTE_MULTIREDIMENCIONAMIENTOS_ARREGLOS_VECES	3

//Si defined, entonces monitorea las operaciones de inserciones, eliminacion y busqueda de elementos,
//con el objetivo de recomendar cambiar entre arreglos ORDENADOS y NO-ORDENADOS
//#define CONFIG_AU_SUGERENCIAS_ARREGLOS_EN_BASE_A_USO
//#define CONFIG_AU_SUGERENCIAS_ARREGLOS_MINIMA_RELACION		100.0 //veces operaciones que la comparada

//Si defined, entonces se habilitan macros que permiten realizar la consulta de datos XML
//mediante dos nombres. Necesario para cuando se tienen dosposibles nombres para un tipo de dato (soporte a formatos anteriores).
//Nota: reduce el rendimiento de la intepretacion de etiquetas XML, debido a la doble busqueda de nodos,recomendado solamente durante la transcion de un formato a otro.
//#define CONFIG_NB_DOBLE_CONSULTA_DE_ETIQUETAS_XML

#endif
