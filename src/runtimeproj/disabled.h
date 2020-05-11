#pragma once

#define FILE $
#define _open $
#if defined(fopen)
#undef fopen
#else
#define fopen $
#endif

#define _wopen $
#define _fsopen $
#undef _tfsopen
#define _wfsopen $

#if defined(_open)
#undef _open
#else
#define _open
#endif

#define _read
