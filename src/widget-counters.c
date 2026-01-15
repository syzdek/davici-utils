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
#define __SRC_WIDGET_GET_COUTNERS_C 1


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
my_widget_counters(
         my_config_t *                 cnf )
{
   int               rc;
   const my_widget_t *     widget;

   if (!(cnf))
      return(1);
   widget   = cnf->widget;

   if ( ((cnf->ike_sa)) && ((cnf->flags & MY_FLG_ALL_IKE)) )
   {  fprintf(stderr, "%s: incompatible options `-a' and `-i'\n", my_prog_name(cnf));
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
   if ((cnf->ike_sa))
      davici_kv(cnf->davici_req, "name", cnf->ike_sa, (unsigned)strlen(cnf->ike_sa));
   if ((cnf->flags & MY_FLG_ALL_IKE))
      davici_kv(cnf->davici_req, "all", "yes", (unsigned)strlen("yes"));

   // queue command
   my_verbose(cnf, "queueing vici command \"%s\" ...\n", widget->davici_cmd);
   rc = davici_queue(cnf->davici_conn, cnf->davici_req, my_davici_cb_command, cnf);
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
