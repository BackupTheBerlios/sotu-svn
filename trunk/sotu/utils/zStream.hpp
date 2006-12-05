// Description:
//   A compressed (file) input/output stream.
//
// Copyright (C) 2005 Frank Becker
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation;  either version 2 of the License,  or (at your option) any  later
// version.
//
// This program is distributed in the hope that it will be useful,  but  WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details
//
#ifndef _zStream_hpp_
#define _zStream_hpp_

#include <fstream>
#include <iostream>

#include <zStreamBufferImpl.hpp>

inline void swap( void *buf, int len)
{
    char *b=(char*)buf;
    for( int i=0; i<(len>>1); i++)
    {
	char t = b[ i];
	b[ i] = b[len-i-1]; 
	b[ len-i-1] = t;
    }
}

enum CompressionType
{
    eNoCompression,
    eZLibCompression,
    eLZMACompression
};

class ziStreamBuffer: public std::streambuf
{
    typedef std::streambuf super;
    typedef int int_type;
    typedef std::streamsize streamsize;
/*
    typedef super::int_type int_type;
    typedef std::char_traits<char> traits;
*/
public:
    ziStreamBuffer( std::ifstream &fd);
    virtual ~ziStreamBuffer();
    bool isOK( void){ return _isOK;}

protected:
    virtual super* setbuf(char*, streamsize)
    {
        return this;
    }
    virtual int_type underflow( void)
    {
        unsigned char c;
        streamsize n = xsgetn( (char*)&c, 1);

        if( n != 1) return EOF;
        return c;
    }
    virtual int_type uflow( void)
    {
#if (__GNUC__ == 3) && (__GNUC_MINOR__ < 4)
        return underflow();
#elif (__GNUC__ == 2) || defined(VCPP)
        return underflow();
#else
        return 0;
#endif
    }
    virtual streamsize xsgetn(char* s, streamsize n);

private:
    ziStreamBuffer( void);
    ziStreamBuffer( const ziStreamBuffer&);
    ziStreamBuffer &operator=(const ziStreamBuffer&);

    void _init( void);
    
    std::ifstream &_fstream;
    bool _isOK;
    ziStreamBufferImpl *_impl;
};

class zoStreamBufferImpl;
class zoStreamBuffer: public std::streambuf
{
    typedef std::streambuf super;
    typedef int int_type;
    typedef std::streamsize streamsize;
/*
    typedef super::int_type int_type;
    typedef std::char_traits<char> traits;
*/
public:
    zoStreamBuffer( std::ofstream &fd, CompressionType compressionType);
    virtual ~zoStreamBuffer();
    bool isOK( void){ return _isOK;}

protected:
    virtual super* setbuf(char*, streamsize)
    {
        return this;
    }
    virtual int_type overflow( int_type c=EOF)
    {
        if( c != EOF)
        {
            char ch = c;
            streamsize n = xsputn( &ch, 1);
            if( n != 1) return EOF;
        }
        return c;
    }

    virtual streamsize xsputn(const char* s, streamsize n);

private:
    zoStreamBuffer( void);
    zoStreamBuffer( const zoStreamBuffer&);
    zoStreamBuffer &operator=(const zoStreamBuffer&);

    void _flush( void);
    void _init( void);
    
    std::ofstream &_fstream;
    bool _isOK;
    zoStreamBufferImpl *_impl;
};

class ziStream: public std::istream
{
    typedef std::istream super;

public:
    ziStream( std::ifstream &inf);
    virtual ~ziStream();

    bool isOK( void){ return _streambuf.isOK();}

private:
    ziStream( void);
    ziStream( const ziStream&);
    ziStream &operator=(const ziStream&);

    ziStreamBuffer &_streambuf;
};

class zoStream: public std::ostream
{
    typedef std::ostream super;
    
public:
    zoStream( std::ofstream &outf, CompressionType compressionType = eZLibCompression);
    virtual ~zoStream();

    bool isOK( void){ return _streambuf.isOK();}
    
private:
    zoStream( void);
    zoStream( const zoStream&);
    zoStream &operator=(const zoStream&);

    zoStreamBuffer &_streambuf;
};  

#endif
