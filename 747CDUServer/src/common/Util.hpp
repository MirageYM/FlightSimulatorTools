#if !defined( PMDG747V3_CONNECT_UTIL_INCLUDED__ )
#define PMDG747V3_CONNECT_UTIL_INCLUDED__

#pragma once

#include <vector>
#include <string>
#include <cctype>
#include <algorithm>

namespace MirageUtil{


	inline std::vector< std::string > tokenizeString( const std::string& str, const std::string& sep ){
		std::vector< std::string > retTokens;
		auto last = str.find_first_not_of( sep, 0 );
		auto pos = str.find_first_of( sep, last );

		while( last != std::string::npos || pos != std::string::npos ){
			retTokens.push_back( str.substr( last, pos - last ) );
			last = str.find_first_not_of( sep, pos );
			pos = str.find_first_of( sep, last );
		}

		return retTokens;
	};

	inline void toLower( std::string& str ){
		std::transform( str.begin(), str.end(), str.begin(), std::tolower );
	};
	
	static const int EVENT_ID_BASE =	0x00011000;
	inline std::string simConnectEnumToStr( unsigned int enumVal, unsigned int base = EVENT_ID_BASE ){
		return std::string( "#" ) + std::to_string( base + enumVal );
	}


}//namespace Util


#endif
