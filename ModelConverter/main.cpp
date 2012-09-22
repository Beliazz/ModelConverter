#include "main.h"

float dwStartTickCount;
float dwEleapsedTicks;

CModel* model = NULL;

#define DEFAULT_FILE "C:\\Users\\Mathias\\Documents\\Assets\\floor_wood.fbx"


int main(int argc, char* args[])
{
	string filepath;
	string sDirectory = string(getenv( "BLOCO_HOME" )) + "\\Editor\\Win32\\Debug\\data\\model\\";

	if (argc < 2)
	{
		printf("No start argument given try to load a hard coded filename: %s \n", DEFAULT_FILE );
		filepath = DEFAULT_FILE;
	}
	else
	{
		filepath = args[1];
	}

	if (argc < 3)
	{
		printf("No start argument given try to load a hard coded filename: %s \n", DEFAULT_FILE );
	}
	else
	{
		filepath = args[2];
	}


	model = new CModel();

	dwStartTickCount = GetTickCount();

	model->Load( filepath );

	dwEleapsedTicks = GetTickCount() - dwStartTickCount;
	printf( "Loading time: %.3f s\n", (float)dwEleapsedTicks/1000.0f );


	string saveName = filepath;
	saveName.erase( saveName.end()-3, saveName.end() );
	saveName += "bcb";

	string saveNameXML = filepath;
	saveNameXML.erase( saveNameXML.end()-3, saveNameXML.end() );
	saveNameXML += "bcx";

	dwStartTickCount = GetTickCount();

	model->Save( sDirectory,saveName, BINARY );
	printf( "Saving path: %s\n", saveName.c_str() );


	model->Save( sDirectory,saveNameXML, XML );
	printf( "Saving path: %s\n", saveNameXML.c_str() );

	//model->Save( "C:\\Users\\Mathias\\Documents\\Visual Studio 2010\\Projects\\bloco\\trunk\\bin\\bloco\\debug\\data\\model\\Test.bcx", XML );


	dwEleapsedTicks = GetTickCount() - dwStartTickCount;
	printf( "Saving time:  %.3f s\n", (float)dwEleapsedTicks/1000.0f );

	system("PAUSE");
	

	delete model;

	return 0;
}

std::string FilenameTitle( string filename )
{
	std::string title = filename;

	for (int i = filename.length() - 1; i >= 0 ; i--)
	{
		if (filename[i] == '\\')
		{
			title.erase(title.begin(), title.begin() + i + 1);
			break;
		}
	}

	return title;
}

std::string FilenameExtension( string filename )
{
	std::string extension = filename;

	for (int i = filename.length() - 1; i >= 0 ; i--)
	{
		if (filename[i] == '.')
		{
			extension.erase(extension.begin(), extension.begin() + i + 1);
			break;
		}
	}

	return extension;
}

std::string GetNameFromFilepath( string filename )
{
	string name   = FilenameTitle( filename );
	string suffix = FilenameExtension(filename);

	name.erase( name.end() - suffix.length() - 1, name.end());

	return name;
}

extern string FixFilePath( string filename )
{
	string result;

	for (unsigned int i = 0; i < filename.length() ; i++)
	{
		if ( filename[i] == '/' )
		{
			result += '\\';
		}
		else
		{
			result += filename[i];
		}
	}

	return result;
}
