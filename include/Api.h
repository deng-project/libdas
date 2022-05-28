// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: Api.h - Header containing necessary macro needed for implicit linking
// author: Karl-Mihkel Ott

#ifndef LIBDAS_API_H
#define LIBDAS_API_H

#if !defined(LIBDAS_STATIC) && defined(_MSC_BUILD)
	#ifdef _MSC_BUILD
		#ifdef LIBDAS_EXPORT_LIBRARY
			#define LIBDAS_API _declspec(dllexport)
		#else
			#define LIBDAS_API _declspec(dllimport)
		#endif
	#else 
		#define LIBDAS_API
	#endif
#else
	#define LIBDAS_API
#endif


// make sure that carriage return is respected in windows
#ifdef _WIN32
	#define LIBDAS_DAS_NEWLINE "\r\n"
#else
	#define LIBDAS_DAS_NEWLINE "\n"
#endif

#endif
