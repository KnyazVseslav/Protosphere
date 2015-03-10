#include "Facilities_LIB.h"

namespace string_facilities
{

	WCHAR* pchar_to_pwchar(const char* pchar_str)
	{

		WCHAR* pwchar_str= 0;

				int chars= MultiByteToWideChar(CP_ACP, 0, pchar_str, -1, NULL, 0);

				pwchar_str= new WCHAR[chars];

				MultiByteToWideChar(CP_ACP, 0, pchar_str, -1, pwchar_str, chars);

		return pwchar_str;

	}

}