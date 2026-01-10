/*
 *  Davici Utilities for Strongswan
 *  Copyright (C) 2026 David M. Syzdek <david@syzdek.net>.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *     1. Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *
 *     2. Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *
 *     3. Neither the name of the copyright holder nor the names of its
 *        contributors may be used to endorse or promote products derived from
 *        this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 *  src/davici-utils.h - common includes and prototypes
 */
#ifndef __SRC_DAVICI_UTILS_H
#define __SRC_DAVICI_UTILS_H 1

///////////////
//           //
//  Headers  //
//           //
///////////////
// MARK: - Headers

// defined in the Single UNIX Specification
#ifndef _XOPEN_SOURCE
#   define _XOPEN_SOURCE 600
#endif

#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif

#include <davici.h>
#include <poll.h>


//////////////
//          //
//  Macros  //
//          //
//////////////
// MARK: - Macros


///////////////////
//               //
//  Definitions  //
//               //
///////////////////
// MARK: - Definitions

#ifndef PROGRAM_NAME
#   define PROGRAM_NAME "davicictl"
#endif
#ifndef PACKAGE_BUGREPORT
#   define PACKAGE_BUGREPORT ""
#endif
#ifndef PACKAGE_COPYRIGHT
#   define PACKAGE_COPYRIGHT "Copyright (C) 2026 David M. Syzdek <david@syzdek.net>."
#endif
#ifndef PACKAGE_NAME
#   define PACKAGE_NAME "Davici Utilities for Strongswan"
#endif
#ifndef PACKAGE_TARNAME
#   define PACKAGE_TARNAME "davici-utils"
#endif
#ifndef PACKAGE_VERSION
#   define PACKAGE_VERSION ""
#endif

#undef MY_SOCK_PATH
#define MY_SOCK_PATH "/var/run/charon.vici"

#define MY_FLG_NOBLOCK     0x00000001


//////////////////
//              //
//  Data Types  //
//              //
//////////////////
// MARK: - Data Types

typedef struct _my_config     my_config_t;
typedef struct _my_widget     my_widget_t;


struct _my_config
{  int                           verbose;
   int                           quiet;
   int                           symlinked;
   int                           queued;
   int                           argc;
   int                           flags;
   struct pollfd                 pollfd;
   char * const *                argv;
   const char *                  prog_name;
   const char *                  vici_sockpath;
   const char *                  ike_sa;
   const char *                  ike_sa_id;
   const char *                  child_sa;
   const char *                  child_sa_id;
   const my_widget_t *           widget;
   struct davici_conn *          davici_conn;
   struct davici_request *       davici_req;
};


struct _my_widget
{  const char *               name;
   const char *               desc;
   const char *               davici_cmd;
   const char *               davici_event;
   const char * const *       aliases;
   const char *               usage;
   const char *               short_opt;
   const struct option *      long_opt;
   int                        arg_min;
   int                        arg_max;
   int                        alias_idx;
   int                        padint;
   int  (*func_exec)(my_config_t * cnf);
   int  (*func_usage)(my_config_t * cnf);
};


/////////////////
//             //
//  Variables  //
//             //
/////////////////
// MARK: - Variables


//////////////////
//              //
//  Prototypes  //
//              //
//////////////////
// MARK: - Prototypes

extern int
my_poll(
         my_config_t *                 cnf );


extern char *
my_prog_name(
         my_config_t *                 cnf );


extern void
my_verbose(
         my_config_t *                 cnf,
         const char *                  fmt,
         ... );


//-------------------//
// davici prototypes //
//-------------------//
#pragma mark davici prototypes

extern void
my_davici_cb_command(
         struct davici_conn *          conn,
         int                           err,
         const char *                  name,
         struct davici_response *      res,
         void *                        user );


extern void
my_davici_cb_event(
         struct davici_conn *          conn,
         int                           err,
         const char *                  name,
         struct davici_response *      res,
         void *                        user );


//--------------------//
// widgets prototypes //
//--------------------//
#pragma mark widgets prototypes

extern int
my_widget_list_sas(
         my_config_t *                 cnf );

#endif /* end of header */
