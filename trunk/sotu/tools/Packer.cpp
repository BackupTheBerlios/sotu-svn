#include "SDL.h" //needed for SDL_main

#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include <list>
#include <fstream>

#include <zlib.h>

#include <Trace.hpp>
#include <Tokenizer.hpp>
#include <ResourceManager.hpp>

const int MAX_LEVEL = 5;
const char RPATH_SEPERATOR = '/';
//OS dependent seperator
const char PATH_SEPERATOR = '/';

bool operator<( DirectoryEntry &de1, DirectoryEntry &de2)
{
    return( de1.resourceName < de2.resourceName);
}

class Packer
{
public:
    Packer( const string &packfileDescription, const string &packfile):
        _level( 0),
        _infile( packfileDescription.c_str()),
        _outfile( packfile.c_str(), ios::out | ios::trunc | ios::binary)
    {
    }

    bool Process( void);

private:
    void addFile( 
	const string &fileName, 
	const string &relativePath,
	const string &fName);

    void addDirectory( 
	const string &dirName,
	const string &resourceName, 
	const string &relativePath);

    void addEntry( 
	const string &fileName,
	const string &resourceName,
	const string &relativePath,
	const string &dName);

    int _level;
    ifstream _infile;
    ofstream _outfile;

    list<DirectoryEntry> dirEntryList;
};

bool Packer::Process( void)
{
    string line;

    while( !getline( _infile, line).eof())
    {
        if( line[0] == '#') continue;
        Tokenizer  t( line);
        string fileName = t.next();
        string resourceName = t.next();
        if( fileName == "") continue;

	char fullPath[PATH_MAX];
#ifdef WIN32
	if( _fullpath(fullPath, fileName.c_str(), PATH_MAX) == 0)
#else
	if( realpath(fileName.c_str(), fullPath) == 0)
#endif
        {
	    LOG_WARNING << "Problem getting full path to file: " 
                        << fileName << endl;
	    LOG_WARNING << "Skipping..." << endl;
            continue;
        }

        if( resourceName != "")
        {
            fileName = resourceName;
        }
        string fullFileName = fullPath;
        string relativePath = "";
        addEntry( fullFileName, resourceName, relativePath, fileName);
    }
    _infile.close();

    LOG_INFO << "Starting sort of " << dirEntryList.size() << " elements" << endl;
    dirEntryList.sort();

    Uint32 dirStart = _outfile.tellp();
    _outfile << (Uint32) dirStart;

    list<DirectoryEntry>::iterator i;
    for( i=dirEntryList.begin(); i!=dirEntryList.end(); i++)
    {
        const string &rName = (*i).resourceName;
        Uint32 offset = (*i).offset;
        Uint32 origSize = (*i).origSize;
        Uint32 compSize = (*i).compSize;
	LOG_INFO << "Writing: " << rName << " [" << offset << "]" << endl;

        _outfile.write( rName.c_str(), rName.length()+1);
	_outfile << (Uint32) offset;
	_outfile << (Uint32) origSize;
	_outfile << (Uint32) compSize;
    }

    Uint32 dirLen = (Uint32)_outfile.tellp() - dirStart;

    _outfile << (Uint32) dirLen;
    _outfile << (Uint32) RESOURCE_MAGIC;

    _outfile.close();

    return true;
}

void Packer::addFile( 
    const string &fileName, 
    const string &relativePath,
    const string &fName)
{
    string rName = relativePath + PATH_SEPERATOR + fName;

    if( relativePath == "")
    {
        rName = fName;
    }

    for( unsigned int i=0; i<rName.length(); i++)
    {
        if( rName[i] == PATH_SEPERATOR)
        {
            rName[i] = RPATH_SEPERATOR;
        }
    }

    ifstream theFile( fileName.c_str(), ios::in | ios::binary);
    ziStream inFile( theFile);
    if( !inFile.isOK())
    {
	LOG_WARNING << "Unable to open " << fileName << endl;
        return;
    }

    DirectoryEntry de;
    de.resourceName = rName;
    de.offset = _outfile.tellp();

    {
	zoStream outFile( _outfile/*, eLZMACompression*/);
	if( !outFile.isOK())
	{
	    LOG_ERROR << "Unable to open packfile!" << endl;
	    return;
	}

	LOG_INFO << "Processing [" << fileName << "] ";
	char inBuf[8192];

	int inCount = 0;
	inFile.read( inBuf, 8192);
	while( inFile.gcount() > 0)
	{ 
	    inCount+=inFile.gcount();
	    outFile.write( inBuf, inFile.gcount());
	    inFile.read( inBuf, 8192);
	}

	de.origSize = inCount;
    }
    _outfile.flush();
    de.compSize = (Uint32)_outfile.tellp()-de.offset;

    int percent = de.origSize ? (100*de.compSize)/de.origSize : 0;

    LOG_INFO << " OK (" << de.origSize << "/" 
             << de.compSize << ") " << percent << "%" << endl;

    dirEntryList.insert( dirEntryList.end(), de);
}

void Packer::addDirectory( 
    const string &dirName,
    const string &resourceName, 
    const string &relativePath)
{
    DIR *dir = opendir( dirName.c_str());
    if( !dir)
    {
	LOG_WARNING << "1Problem accessing: " << dirName << endl;
	LOG_WARNING << "Skipping..." << endl;
        return;
    }

    dirent *dirEntry;
    while( (dirEntry = readdir( dir)) != 0)
    {
        string fileName = dirEntry->d_name;
        if( (fileName == ".") || (fileName == "..")) continue;

        string fullName = dirName +PATH_SEPERATOR+ fileName;
        addEntry( fullName, resourceName, relativePath, fileName);
    }
}

void Packer::addEntry( 
    const string &fileName,
    const string &resourceName,
    const string &relativePath,
    const string &dName)
{
    if( _level > MAX_LEVEL)
    {
	LOG_WARNING << "Max directory depth (" 
                    << MAX_LEVEL << ") exceeded." << endl;
	LOG_WARNING << "Skipping: " << fileName << endl;
        return;
    }

    struct stat statInfo;
    if( stat( fileName.c_str(), &statInfo) == -1)
    {
	LOG_WARNING << "2Problem accessing: " << fileName << endl;
	LOG_WARNING << "Skipping..." << endl;
	return;
    }

    //skip .svn directories
    if( dName == ".svn")
    {
	LOG_WARNING << "Skipping Subversion directory." << endl;
	return;
    }

    //skip CVS directories
    if( dName == "CVS")
    {
	LOG_WARNING << "Skipping CVS directory." << endl;
	return;
    }

    if( S_ISDIR( statInfo.st_mode))
    {
        string newRelativePath = relativePath;
        if( _level > 0)
        {
            newRelativePath += PATH_SEPERATOR+ dName;
        }
        else
        {
            newRelativePath += dName;
        }
        _level++;
	addDirectory( fileName, resourceName, newRelativePath);
        _level--;
    }
    else if( S_ISREG( statInfo.st_mode))
    {
	addFile( fileName, relativePath, dName);
    }
    else
    {
	LOG_WARNING << "Unsupported file type: " << fileName << endl;
	LOG_WARNING << "Skipping..." << endl;
	return;
    }
}

int main( int argc, char *argv[])
{
    if( argc != 3)
    {
	LOG_ERROR << "Packer description_file pack_file" << endl;
        return -1;
    }

    string description_file( argv[1]);
    string pack_file( argv[2]);
    Packer p( description_file, pack_file);
    p.Process();

//-----

    ResourceManager &rm = *ResourceManagerS::instance();
    if( rm.addResourcePack( string( argv[2])))
    {
	LOG_INFO << "Verified packfile OK." << endl;
    }
#if 0
    rm.dump();
{
    string resName("models/Bonus2.model");
    if( rm.selectResource( resName))
    {
	ziStream &infile = rm.getInputStream();
	string line;
	while( !getline( infile, line).eof())
        {
            cout << line << endl;
        }
    }
    else
    {
        cout << resName << " not found" << endl;
    }
}
{
    string resName("models/P1.model");
    if( rm.selectResource( resName))
    {
	ziStream &infile = rm.getInputStream();
	string line;
	while( !getline( infile, line).eof())
        {
            cout << line << endl;
        }
    }
    else
    {
        cout << resName << " not found" << endl;
    }
}
{
    string resName("bitmaps/menuShadow.font");
    if( rm.selectResource( resName))
    {
	ziStream &infile = rm.getInputStream();
	char buffer[100000];
	infile.read( buffer, 100000);
	streamsize numBytes = infile.gcount();
        ofstream outfile( "test.png", ios::out | ios::trunc | ios::binary);
	outfile.write( buffer, numBytes); 
    }
    else
    {
        cout << resName << " not found" << endl;
    }
}
#endif

    return 0;
}
