#ifndef IOHelper__H_
#define IOHelper__H_

#include "main.h"

class IOHelper
{
public:
	IOHelper()
	{

	}
	~IOHelper()
	{
		close();
	}

	bool open( string filename, const char* mode )
	{
		m_pFile = fopen( filename.c_str(), mode );

		if ( m_pFile == NULL)
			return false;

		return true;
	}
	void close()
	{
		fclose(m_pFile);
	}

	void write( bool value )		
	{
		write( (char*)&value, sizeof(bool) );
	}
	void write( int value )		
	{
		write( (char*)&value, sizeof(int) );
	}
	void write( WORD value )
	{
		write( (char*)&value, sizeof(WORD) );
	}
	void write( DWORD value )
	{
		write( (char*)&value, sizeof(DWORD) );
	}
	void write( double value )
	{
		write( (char*)&value, sizeof(double) );
	}
	void write( float value )
	{
		write( (char*)&value, sizeof(float) );
	}
	void write( Color value )
	{
		write( (char*)&value, sizeof(float), 4 );
	}
	void write( Vector3 value )
	{
		write( (char*)&value, sizeof(float), 3 );
	}

	void write( KFbxMatrix matrix )
	{
		const DOUBLE* pDoubles = (const DOUBLE*)matrix.mData;
		float data[16];

		for (unsigned int i = 0; i < 16 ; i++)
		{
			data[i] = (float)pDoubles[i];
		}

		write( data, sizeof(float), 16 );
	}

	void read( bool &value )		
	{
		read( (char*)&value, sizeof(bool) );
	}
	void read( int& value )		
	{
		read( (char*)&value, sizeof(int) );
	}
	void read( WORD& value )
	{
		read( (char*)&value, sizeof(WORD) );
	}
	void read( DWORD& value )
	{
		read( (char*)&value, sizeof(DWORD) );
	}
	void read( double& value )
	{
		read( (char*)&value, sizeof(double) );
	}
	void read( float& value )
	{
		read( (char*)&value, sizeof(float) );
	}
	void read( Color value )
	{
		read( (char*)&value, sizeof(float), 4 );
	}
	void read( Vector3 value )
	{
		read( (char*)&value, sizeof(float), 3 );
	}

	void write( string value, DWORD size)
	{
		strcpy(buffer,value.c_str());

		write( buffer, size);
	}

	void write(const void* data, DWORD size, DWORD count = 1 )
	{
		fwrite( data, size, count, m_pFile );
	}
	void read( void* data, DWORD size, DWORD count = 1 )
	{
		fread( data, size, count, m_pFile );
	}

private:
	char buffer[256*4];
	FILE* m_pFile;

};

#endif