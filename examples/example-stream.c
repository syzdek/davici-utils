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
 *  This program is an example of using the davici library to submit a command
 *  which does uses event streams to return data.
 *
 *  To compile this program, run the following:
 *
 *        gcc -W -O2 -o example-stream example-stream.c -ldavici
 */
#define __EXAMPLES_EXAMPLE_STREAM_C 1


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

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <getopt.h>
#include <poll.h>

#include <davici.h>


///////////////////
//               //
//  Definitions  //
//               //
///////////////////
// MARK: - Definitions

#undef PROGRAM_NAME
#define PROGRAM_NAME "example-stream"


//////////////
//          //
//  Macros  //
//          //
//////////////
// MARK: - Macros


/////////////////
//             //
//  Datatypes  //
//             //
/////////////////
#pragma mark - Datatypes

typedef struct _my_data my_data_t;
struct _my_data
{  int                           versbose;
   int                           quiet;
   int *                         queued;
   struct pollfd *               pollfd;
};


//////////////////
//              //
//  Prototypes  //
//              //
//////////////////
// MARK: - Prototypes

extern int
main(
         int                           argc,
         char **                       argv );


static void
my_davici_cb_command(
         struct davici_conn *          conn,
         int                           err,
         const char *                  name,
         struct davici_response *      res,
         void *                        user );


static void
my_davici_cb_event(
         struct davici_conn *          conn,
         int                           err,
         const char *                  name,
         struct davici_response *      res,
         void *                        user );


static int
my_davici_fdcb(
         struct davici_conn *          conn,
         int                           fd,
         int                           ops,
         void *                        user );


static int
my_parse_response(
         const char *                  name,
         struct davici_response *      res,
         my_data_t *                   datp );


static void
my_verbose(
         my_data_t *                   datp,
         const char *                  fmt,
         ... );


/////////////////
//             //
//  Functions  //
//             //
/////////////////
// MARK: - Functions

int
main(
         int                           argc,
         char **                       argv )
{
   int                        rc;
   int                        queued;
   const char *               socket_path;
   struct davici_conn *       conn;
   struct davici_request *    req;
   my_data_t                  dat;
   struct pollfd              pollfd;

   if (argc != 2)
   {  fprintf(stderr, "Usage: %s [ <socket> ]\n", PROGRAM_NAME);
      return(1);
   };

   // initializes defaults
   memset(&dat,      0, sizeof(my_data_t));
   memset(&pollfd,   0, sizeof(struct pollfd));
   socket_path       = argv[1];
   queued            = 0;
   pollfd.fd         = -1;

   // connect to vici socket
   my_verbose(&dat, "connecting to vici socket ...\n");
   rc = davici_connect_unix(socket_path, my_davici_fdcb, &dat, &conn);
   if (rc < 0)
   {  fprintf(stderr, "%s: %s\n", PROGRAM_NAME, strerror(-rc));
      return(1);
   };

   // initialize user data
   dat.versbose      = 1;
   dat.quiet         = 0;
   dat.pollfd        = &pollfd;
   dat.queued        = &queued;

   // initialize new command
   my_verbose(&dat, "initializing vici command \"list-sas\" ...\n");
   rc = davici_new_cmd("list-sas", &req);
   if (rc < 0)
   {  fprintf(stderr, "%s: %s\n", PROGRAM_NAME, strerror(-rc));
      davici_disconnect(conn);
      return(1);
   };

   // queue command
   my_verbose(&dat, "queueing vici command \"list-sas\" with event \"list-sa\" ...\n");
   rc = davici_queue_streamed(conn, req, my_davici_cb_command, "list-sa", my_davici_cb_event, &dat);
   if (rc < 0)
   {  fprintf(stderr, "%s: list-sas: %s\n", PROGRAM_NAME, strerror(-rc));
      davici_cancel(req);
      davici_disconnect(conn);
      return(1);
   };
   queued++;

  // poll for responses
   while ( (queued > 0) && (pollfd.fd != -1) )
   {  if ((rc = poll(&pollfd, 1, 30000)) < 0)
      {  fprintf(stderr, "%s: poll(): %s\n", PROGRAM_NAME, strerror(errno));
         davici_disconnect(conn);
         return(1);
      };
      if (!(rc))
      {  fprintf(stderr, "%s: list-sas: operation timed out\n", PROGRAM_NAME);
         davici_disconnect(conn);
         return(1);
      };
      if ((pollfd.revents & POLLIN))
      {  if ((rc = davici_read(conn)) < 0)
         {  fprintf(stderr, "%s: davici_read(): %s\n", PROGRAM_NAME, strerror(-rc));
            davici_disconnect(conn);
            return(1);
         };
      };
      if ((pollfd.revents & POLLOUT))
      {  if ((rc = davici_write(conn)) < 0)
         {  fprintf(stderr, "%s: davici_write(): %s\n", PROGRAM_NAME, strerror(-rc));
            davici_disconnect(conn);
            return(1);
         };
      };
   };

   davici_disconnect(conn);

   return(0);
}


void
my_davici_cb_command(
         struct davici_conn *          conn,
         int                           err,
         const char *                  name,
         struct davici_response *      res,
         void *                        user )
{
   int               rc;
   my_data_t *       datp;

   if (!(conn))
      return;

   datp = (my_data_t *)user;

   my_verbose(datp, "processing results of \"%s\" command ...\n", name);

   (*datp->queued)--;

   if (err < 0)
   {  fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, name, strerror(-err));
      return;
   };

   if (!(res))
      return;

   rc = my_parse_response(name, res, datp);
   if (rc < 0)
   {  fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, name, strerror(-rc));
      return;
   };

   return;
}


void
my_davici_cb_event(
         struct davici_conn *          conn,
         int                           err,
         const char *                  name,
         struct davici_response *      res,
         void *                        user )
{
   int               rc;
   my_data_t *       datp;

   if (!(conn))
      return;

   datp = (my_data_t *)user;

   my_verbose(datp, "processing results of \"%s\" event ...\n", name);

   if (err < 0)
   {  fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, name, strerror(-err));
      return;
   };

   if (!(res))
      return;

   rc = my_parse_response(name, res, datp);
   if (rc < 0)
   {  fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, name, strerror(-rc));
      return;
   };

   return;
}


int
my_davici_fdcb(
         struct davici_conn *          conn,
         int                           fd,
         int                           ops,
         void *                        user )
{
   my_data_t *    datp;

   if (!(conn))
      return(0);

   datp                  = (my_data_t *)user;

   my_verbose(datp, "configuring polling events ...\n");

   datp->pollfd->events  = ((ops & DAVICI_READ))      ? POLLIN    :  0;
   datp->pollfd->events |= ((ops & DAVICI_WRITE))     ? POLLOUT   :  0;
   datp->pollfd->fd      = ((datp->pollfd->events))   ? fd        : -1;

   return(0);
}


int
my_parse_response(
         const char *                  name,
         struct davici_response *      res,
         my_data_t *                   datp )
{
   int               rc;
   char              str[4096];
   const char *      key;

   my_verbose(datp, "parsing response of \"%s\" ...\n", name);

   while((rc = davici_parse(res)) >= 0)
   {  switch(rc)
      {  case DAVICI_END:
            printf("DAVICI_END\n");
            return(0);

         case DAVICI_SECTION_START:
            key = davici_get_name(res);
            printf("DAVICI_SECTION_START: %s\n", key);
            break;

         case DAVICI_SECTION_END:
            printf("DAVICI_SECTION_END\n");
            break;

         case DAVICI_KEY_VALUE:
            rc = davici_get_value_str(res, str, sizeof(str));
            if (rc < 0)
            {  fprintf(stderr, "%s: davici_get_value_str(): %s\n", PROGRAM_NAME, strerror(-rc));
               return(0);
            };
            key = davici_get_name(res);
            printf("DAVICI_KEY_VALUE: %s = \"%s\"\n", key, str);
            break;

         case DAVICI_LIST_START:
            key = davici_get_name(res);
            printf("DAVICI_LIST_START: %s\n", key);
            break;

         case DAVICI_LIST_ITEM:
            rc = davici_get_value_str(res, str, sizeof(str));
            if (rc < 0)
            {  fprintf(stderr, "%s: davici_get_value_str(): %s\n", PROGRAM_NAME, strerror(-rc));
               return(0);
            };
            printf("DAVICI_LIST_ITEM: %s\n", str);
            break;

         case DAVICI_LIST_END:
            printf("DAVICI_LIST_END\n");
            break;

         default:
            printf("UNKNOWN\n");
            break;
      };
   };

   return(rc);
}


void
my_verbose(
         my_data_t *                   datp,
         const char *                  fmt,
         ... )
{
   va_list     args;
   if (!(datp->versbose))
      return;
   va_start(args, fmt);
   vprintf(fmt, args);
   va_end(args);
   return;
}


/* end of source */
