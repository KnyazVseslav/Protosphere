#include "IncludeHLSL.h"

#include <fstream>
#include <vector>


using namespace std;

wstring path_trunc(const wstring& path, const short& extract_level)
{
	
	wstring r= path;

	if((wchar_t)92 == r[r.size()-1]) // (wchar_t)92 ��� ������� \ � �������
		r.resize(r.size()-1);

		for(UINT32 i= 0; i < extract_level; i++)
			r.resize(r.find_last_of(L"\\/"));


	return r+= L"\\";

}

wstring get_current_exe_dir()
{

	WCHAR exe_dir[MAX_PATH+1]= L"";

	GetModuleFileNameW(NULL, exe_dir, MAX_PATH);

	return path_trunc(exe_dir, 1); // ������ ��� exe-�����; ��������� ����� ��������� ������� �������� ����� �����

}


wstring& get_root(wstring& path)
{
	if((path.find(L"Debug") != wstring::npos) || (path.find(L"Release") != wstring::npos))
		path= path_trunc(path, 1);

	return path;
}


wstring get_root(void)
{
	wstring path= get_current_exe_dir();

	if((path.find(L"Debug") != wstring::npos) || (path.find(L"Release") != wstring::npos))
		path= path_trunc(path, 1);

	return path;
}


WCHAR* pchar_to_pwchar(const char* pchar_str)
{

	WCHAR* pwchar_str= 0;

			int chars= MultiByteToWideChar(CP_ACP, 0, pchar_str, -1, NULL, 0);

			pwchar_str= new WCHAR[chars];

			MultiByteToWideChar(CP_ACP, 0, pchar_str, -1, pwchar_str, chars);

	return pwchar_str;

}



bool path_is_absolute(const wstring& path)
{
	// ���� � ������ ���� ���� �������  : � (\ ��� /), �� ���� �������� ����������, ����� �������� ���������, �.�. ���� ����� ��������� : ��� / ��� \ � ���� ��� ���� ���������
	return ( path.find((wchar_t)58) != wstring::npos && (path.find((wchar_t)92) != wstring::npos || path.find((wchar_t)47) != wstring::npos) ); // (wchar_t)47, 58,  92 ���� �������� /, :, \ : � �������
}	



IncludeHLSL::IncludeHLSL(const char* system_path)
{

	// ����������� (���������) ����� ������ ������������� ������
	// ����� ����������� � ��������, ������������ � ��������� system_path, ���� �� �� �������� ������ �������,
	// ����� ����� ������ �������� ������� ��� ��� system_path, ��� � ��� m_root_path;
	// ��� ����, ���� ���� �������������, �� �� ����� �������������� � �������� ����,
	// ���� �� �� ����������, �� ����� ����������� ��� �������������� �������� ��� ���


	if(system_path != "")
	{
		m_root_path= get_root();
		m_system_path= pchar_to_pwchar(system_path);

		if(!path_is_absolute(m_system_path)) m_system_path = m_root_path + m_system_path;

	}
		else m_system_path = m_root_path = get_root();

}


HRESULT __stdcall IncludeHLSL::Open(D3D_INCLUDE_TYPE IncludeType,
									LPCSTR pFileName,
									LPCVOID pParentData,
									LPCVOID* pData,
									UINT* pBytes)
{
	
	//if(NULL == pFileName || "" == pFileName) throw ios::failure e;

	bool is_absolute_path;

	  
	
	wstring file_name= pchar_to_pwchar(pFileName);


		 is_absolute_path= path_is_absolute(file_name);


	vector<wstring> search_paths;


	try
	{
	
		switch(IncludeType)
		{
									// �������� �� ���� �������������?
			case D3D_INCLUDE_LOCAL: if(is_absolute_path) search_paths.push_back(file_name); // ���� ���, �� ������ search_paths ����� ����� ������ ���� �������, ������ ����� ����
										else // ���� ��, �� ������ search_paths ����� �������� �� ���� ���������
										{
											search_paths.push_back(m_root_path+file_name); // ������: �������� ���� + ������������� ����
											search_paths.push_back(m_system_path+file_name); // ������: ��������� ���� (������� ����� ���� ��������, ���� � �-� ���� �������� ������ ������) + ������������� ����
										}
			 break;
				// ��� ���������� �����������  ������, �� ��� ��, 
		   case D3D_INCLUDE_SYSTEM: if(is_absolute_path) search_paths.push_back(file_name);
										else
										{
											// ������ ������ � ������ ���� �������� �������, �.�.
											search_paths.push_back(m_system_path+file_name);      // ����� ����� ���������� � ����: ��������� ���� (������� ����� ���� ��������, ���� � �-� ���� �������� ������ ������) + ������������� ���� ,
											search_paths.push_back(m_root_path+file_name);		  // ���� ���� �� ��� ������ (������), �� ��������� ���� ������ ������ ����: �������� ���� + ������������� ����
										}
			break;
		}


			fstream f;
			int fsize= -1;
	
			f.exceptions(ios::failbit | ios::badbit); // ���������� ������ ���������� �� ��������� ������, �����������������
													  // ������� ios::failbit � ios::badbit (����� ���������� ������ ������� ��������
													  // ios::failbit | ios::badbit)

													  // ���������� ����� ���������� (stream exception mask) �� ��������� ���� ������,
													  // ��� ����� ��������� ��������������� ������ �������� (��� ���� ���������� �� �����
													  // �������������)

													  // ���� ios::failbit ��������������� ��� ������� ������ � ������ ������������� 
													  // ������ �� ����� �������� �����-������ � ������, ����. open() (��������� ���������� ��
													  // ������ ��������� ������ - ��. error state flags �
													  // ios::rdstate() - �������, ������������ ����� ��������� ������, ����������
													  // ������� ����������� ������: goodbit, eofbit, failbit, badbit, ������������ ������� ��������� OR (|)

													  // �.�., ���� �������� ����� - ����� open() - �� ���������, � ��������� ������
													  // ����� �������� ���� failbit � ����� ������� ���������� ios::failure
		
		bool success_flag= false;

		 short i= 0;

			for (i = 0; !success_flag && i < search_paths.size(); i++ ) // �������� ������� ���� �� ��� ��� ���� success_flag == false � �� ��������� ����� ������� search_paths
			{

				try
				{
			
					f.open(search_paths[i], ios::in); // ������� ������ ��� ������
			
					success_flag= true;

					//read and output file content
			
							f.seekg(0, f.end);

							fsize= f.tellg();

								f.seekg(0, f.beg);

							char* buf = new char[fsize]();


								f.read(buf, fsize);

				}			
				catch(ios::failure& e)
				{
					MessageBoxA(0, e.what(), "Exception occurred", 0);
				}	
							
				
			}// for



		return S_OK;

	}
	catch(ios::failure& e)
	{
	
	}
	

}


HRESULT __stdcall IncludeHLSL::Close(LPCVOID pData)
{

	char* buf= (char*)pData;

	delete [] buf;

	return S_OK;

}