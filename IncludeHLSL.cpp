#include "IncludeHLSL.h"

#include <fstream>
#include <vector>


using namespace std;

wstring path_trunc(const wstring& path, const short& extract_level)
{
	
	wstring r= path;

	if((wchar_t)92 == r[r.size()-1]) // (wchar_t)92 код символа \ в уникоде
		r.resize(r.size()-1);

		for(UINT32 i= 0; i < extract_level; i++)
			r.resize(r.find_last_of(L"\\/"));


	return r+= L"\\";

}

wstring get_current_exe_dir()
{

	WCHAR exe_dir[MAX_PATH+1]= L"";

	GetModuleFileNameW(NULL, exe_dir, MAX_PATH);

	return path_trunc(exe_dir, 1); // отсечь имя exe-файла; результат будет содержать ведущую обратную косую черту

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
	// если в строке пути есть символы  : и (\ или /), то путь является глобальным, такая проверка актуальна, т.к. путь может содержать : без / или \ и быть при этом локальным
	return ( path.find((wchar_t)58) != wstring::npos && (path.find((wchar_t)92) != wstring::npos || path.find((wchar_t)47) != wstring::npos) ); // (wchar_t)47, 58,  92 коды символов /, :, \ : в уникоде
}	



IncludeHLSL::IncludeHLSL(const char* system_path)
{

	// Стандартная (системная) папка поиска подключаемого модуля
	// будет установлена в значение, передаваемое в параметре system_path, если он не является пустой строкой,
	// иначе будет выбран корневой каталог как для system_path, так и для m_root_path;
	// при этом, если путь относительный, то он будет конкатенирован с корневым путём,
	// если же он абсолютный, то будет использован без дополнительных операций над ним


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
									// является ли путь относительным?
			case D3D_INCLUDE_LOCAL: if(is_absolute_path) search_paths.push_back(file_name); // если нет, то массив search_paths будет иметь только один элемент, равный этому пути
										else // если да, то массив search_paths будет состоять из двух элементов
										{
											search_paths.push_back(m_root_path+file_name); // первый: корневой путь + относительный путь
											search_paths.push_back(m_system_path+file_name); // второй: системный путь (который может быть корневым, если в к-р была передана пустая строка) + относительный путь
										}
			 break;
				// для системного подключения  модуля, всё так же, 
		   case D3D_INCLUDE_SYSTEM: if(is_absolute_path) search_paths.push_back(file_name);
										else
										{
											// только первый и второй пути поменяны местами, т.е.
											search_paths.push_back(m_system_path+file_name);      // поиск будет начинаться с пути: системный путь (который может быть корневым, если в к-р была передана пустая строка) + относительный путь ,
											search_paths.push_back(m_root_path+file_name);		  // если файл не был найден (открыт), то следующим путём поиска станет путь: корневой путь + относительный путь
										}
			break;
		}


			fstream f;
			int fsize= -1;
	
			f.exceptions(ios::failbit | ios::badbit); // установить бросок исключения на состояния потока, характеризующиеся
													  // флагами ios::failbit и ios::badbit (маска исключений потока приймет значение
													  // ios::failbit | ios::badbit)

													  // изначально маска исключений (stream exception mask) не содержала этих флагов,
													  // что можно проверить закомментировав данный оператор (при этом исключение не будет
													  // сгенерировано)

													  // флаг ios::failbit устанавливается для объекта потока в случае возникновения 
													  // ошибки во время операции ввода-вывода в потоке, напр. open() (детальная информация по
													  // флагам состояния потока - см. error state flags и
													  // ios::rdstate() - функция, возвращающая маску состояния потока, являющуюся
													  // битовой комбинацией флагов: goodbit, eofbit, failbit, badbit, объединяемых битовой операцией OR (|)

													  // Т.о., если открытие файла - метод open() - не получится, в состояние потока
													  // будет добавлен флаг failbit и будет брошено исключение ios::failure
		
		bool success_flag= false;

		 short i= 0;

			for (i = 0; !success_flag && i < search_paths.size(); i++ ) // пытаться открыть файл до тех пор пока success_flag == false и не достигнут конец массива search_paths
			{

				try
				{
			
					f.open(search_paths[i], ios::in); // открыть только для чтения
			
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