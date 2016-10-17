#pragma once
#ifdef TESTX_TEST
#ifndef _TESTX_WCHAR_TOOLS_HPP
#define _TESTX_WCHAR_TOOLS_HPP

#include "testx.hpp"

#define TESTX_AUTO_CH_TEST_CASE(_name)	\
			template<typename Ch>			\
			void _name##_impl();			\
			TESTX_AUTO_TEST_CASE(_name)		\
			{								\
				_name##_impl<char>();		\
				_name##_impl<wchar_t>();	\
			}								\
			template<typename Ch>			\
			void _name##_impl()


namespace std {
	inline std::ostream& operator<<(std::ostream& out, const std::wstring& value)
	{
		 out << std::string(value.begin(), value.end());
		 return out;
	}
} 
#endif
#endif