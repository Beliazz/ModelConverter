#ifndef ModelElement_H__
#define ModelElement_H__

#include "main.h"

class ModelElement
{
	public:
		ModelElement(){};
		~ModelElement();

		virtual bool VSave( TiXmlElement* );
		virtual bool VSave( IOHelper* );
		virtual bool VLoad( TiXmlElement* ) { return true; }
		virtual string GetType()=0;
		
		void AddElement( ModelElement* pElement) { return m_pChildren.push_back(pElement); }
		DWORD GetChildCount() { return m_pChildren.size(); }
		ModelElement* GetChild( DWORD index ) { return m_pChildren[index]; }

		virtual string str() {return "Unsupported"; }
		virtual TiXmlElement* xml() 
		{ 
			TiXmlElement* var = new TiXmlElement( GetType().c_str() );
			var->SetAttribute( "Info", "Unsupported" );

			return var; 
		}
		virtual void bin( IOHelper* pWriter ) 
		{ 
		}

		virtual void Delete(){};

		vector<ModelElement*> m_pChildren;
		KFbxMatrix m_GlobalMatrix;
};

class RootElement : public ModelElement
{
public:
	RootElement();

	virtual bool VSave( TiXmlElement* );
	virtual bool VSave( IOHelper* );
	virtual string GetType() { return "Root"; };

private:
	string m_sName;
	virtual void Delete();
};

#endif