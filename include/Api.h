// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: Api.h - Header containing necessary macro needed for implicit linking
// author: Karl-Mihkel Ott

#ifndef API_H
#define API_H

#ifdef _MSC_BUILD
	#ifdef LIBDAS_EXPORT_LIBRARY
		#define LIBDAS_API _declspec(dllexport)
	#else
		#define LIBDAS_API _declspec(dllimport)
	#endif
#else
	#define LIBDAS_API
#endif

#endif
