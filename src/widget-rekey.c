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
#define __SRC_WIDGET_REKEY_C 1


///////////////
//           //
//  Headers  //
//           //
///////////////
// MARK: - Headers

#include "davicictl.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>

#include <davici.h>


///////////////////
//               //
//  Definitions  //
//               //
///////////////////
// MARK: - Definitions


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


//////////////////
//              //
//  Prototypes  //
//              //
//////////////////
// MARK: - Prototypes


/////////////////
//             //
//  Variables  //
//             //
/////////////////
// MARK: - Variables


/////////////////
//             //
//  Functions  //
//             //
/////////////////
// MARK: - Functions


int
my_widget_rekey(
         my_config_t *                 cnf )
{
   int                     rc;
   int                     c;
   const char *            event;
   const my_widget_t *     widget;

   if (!(cnf))
      return(1);
   widget   = cnf->widget;

   c  =  0;
   c  += ((cnf->child_sa))    ? 1 : 0;
   c  += ((cnf->child_sa_id)) ? 1 : 0;
   c  += ((cnf->ike_sa))      ? 1 : 0;
   c  += ((cnf->ike_sa_id))   ? 1 : 0;
   if ( c > 1 )
   {  fprintf(stderr, "%s: incompatible options `-C', `-c', `-I', and `-i'\n", my_prog_name(cnf));
      fprintf(stderr, "Try `%s --help' for more information.\n",  my_prog_name(cnf));
      return(1);
   };
   if ( c != 1 )
   {  fprintf(stderr, "%s: missing required option `-C', `-c', `-I', or `-i'\n", my_prog_name(cnf));
      fprintf(stderr, "Try `%s --help' for more information.\n",  my_prog_name(cnf));
      return(1);
   };

   // initialize new command
   my_verbose(cnf, "initializing vici command \"%s\" ...\n", widget->davici_cmd);
   rc = davici_new_cmd(widget->davici_cmd, &cnf->davici_req);
   if (rc < 0)
   {  fprintf(stderr, "%s: %s\n", my_prog_name(cnf), strerror(-rc));
      return(1);
   };

   // add arguments
   event = NULL;
   if ((cnf->child_sa))
   {  davici_kv(cnf->davici_req, "child", cnf->child_sa, (unsigned)strlen(cnf->child_sa));
      event = "child-rekey";
   } else if ((cnf->child_sa_id))
   {  davici_kv(cnf->davici_req, "child-id", cnf->child_sa_id, (unsigned)strlen(cnf->child_sa_id));
      event = "child-rekey";
   } else if ((cnf->ike_sa))
   {  davici_kv(cnf->davici_req, "ike", cnf->ike_sa, (unsigned)strlen(cnf->ike_sa));
      event = "ike-rekey";
   } else if ((cnf->ike_sa_id))
   {  davici_kv(cnf->davici_req, "ike-id", cnf->ike_sa_id, (unsigned)strlen(cnf->ike_sa_id));
      event = "ike-rekey";
   };
   if ((cnf->flags & MY_FLG_REAUTH))
      davici_kv(cnf->davici_req, "reauth", "yes", (unsigned)strlen("yes"));

   // queue command
   my_verbose(cnf, "queueing vici command \"%s\" with event \"%s\" ...\n", widget->davici_cmd, widget->davici_event);
   rc = davici_queue_streamed(cnf->davici_conn, cnf->davici_req, my_davici_cb_command, event, my_davici_cb_event, cnf);
   if (rc < 0)
   {  fprintf(stderr, "%s: %s\n", my_prog_name(cnf), strerror(-rc));
      my_verbose(cnf, "canceling vici command \"%s\" ...\n", widget->davici_cmd);
      davici_cancel(cnf->davici_req);
      return(1);
   };
   cnf->queued++;

   if ((my_poll(cnf)))
      return(1);

   return(0);
}

/* end of source */
