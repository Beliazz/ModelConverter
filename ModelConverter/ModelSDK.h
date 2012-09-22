#ifndef Model_H__
#define Model_H__

#include<string>
using namespace std;


class	RootElement;

enum SaveType
{
	ASCII,
	XML,
	BINARY,
};

class CModel
{
public:
	CModel(void);
	~CModel(void);

	 bool Load( std::string filename );
	 bool Save( std::string directory, std::string filename, SaveType mode );

private:
	void Delete();
	bool LoadFBX(	std::string filename );
	bool LoadSTL(	std::string filename );
	bool LoadBLEND( std::string filename );


	RootElement* m_pRootElement;
};

#endif
