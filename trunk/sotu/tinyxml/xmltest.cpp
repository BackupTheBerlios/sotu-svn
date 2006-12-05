#include "tinyxml.h"

//
// This file demonstrates some basic functionality of TinyXml.
// Note that the example is very contrived. It presumes you know
// what is in the XML file. But it does test the basic operations,
// and show how to add and remove nodes.
//

int level = 0;

char *getIndent(void)
{
    static char buf[128];
    int i;
    for( i=0; i<level; i++)
    {
	buf[i] = ' ';
    }
    buf[i] = '\0';
    return buf;
}

void TraverseMenu( TiXmlNode* child)
{
    while( child)
    {
	fprintf( stdout, "%s[%s] : ", getIndent(), child->Value().c_str());

	TiXmlElement* thisEntry = child->ToElement();
        if( thisEntry)
        {
	    TiXmlAttribute* attrib;
	    for ( attrib = thisEntry->FirstAttribute(); attrib; attrib = attrib->Next() )
	    {       
		fprintf( stdout, "[%s=%s] ", attrib->Name().c_str(), attrib->Value().c_str());
	    }
        }    
	fprintf( stdout, "\n");

        level++;
        TraverseMenu( child->FirstChild());
        level--;

	child = child->NextSibling();
    }
}

int main()
{

    TiXmlDocument doc( "menu.xml" );
    doc.LoadFile();

    TraverseMenu( doc.FirstChild("Menu"));

    return 0;
}

