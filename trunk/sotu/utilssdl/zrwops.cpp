// Description:
//   SDL_RWops wrapper for compressed stream.
//
// Copyright (C) 2001 Frank Becker
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
#include <stdlib.h>
#include <Trace.hpp>

#include <zStream.hpp>
#include <SDL_rwops.h>

int ziStream_seek(struct SDL_RWops * /*context*/,int offset,int whence)
{
    LOG_WARNING << "ziStream_seek not implemented!" << endl;
    LOG_WARNING << "ziStream_seek attempt to seek to offset " <<  offset << " whence " << whence << endl;
    return -1;
}
int ziStream_read(struct SDL_RWops *context, void *ptr, int size, int maxnum)
{
    ziStream &zi = *((ziStream*)context->hidden.unknown.data1);
    zi.read( (char*)ptr, size*maxnum);
//    return( zi.gcount()/size);
    return( maxnum);
}
int ziStream_write(struct SDL_RWops *,const void *,int,int)
{
    LOG_ERROR << "ziStream_write not implemented!" << endl;
    return -1;
}
int ziStream_close(struct SDL_RWops *context)
{
    if( context) 
    {
	SDL_FreeRW( context);
    }
    return(0);
}

SDL_RWops *RWops_from_ziStream( ziStream &zi)
{
    SDL_RWops *rwops;

    rwops = SDL_AllocRW();
    if ( rwops != NULL ) {
	rwops->seek = ziStream_seek;
	rwops->read = ziStream_read;
	rwops->write = ziStream_write;
	rwops->close = ziStream_close;
	rwops->hidden.unknown.data1 = &zi;
    }
    return( rwops);
}

