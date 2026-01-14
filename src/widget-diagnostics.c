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
#define __SRC_WIDGET_GET_DIAGNOSTICS_C 1


///////////////
//           //
//  Headers  //
//           //
///////////////
// MARK: - Headers

#include "davici-utils.h"

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


//////////////////
//              //
//  Prototypes  //
//              //
//////////////////
// MARK: - Prototypes

static int
my_widget_diagnostics_get_pools(
         my_config_t *                 cnf );


static int
my_widget_diagnostics_list_conns(
         my_config_t *                 cnf );


static int
my_widget_diagnostics_list_sas(
         my_config_t *                 cnf );


static int
my_widget_diagnostics_stats(
         my_config_t *                 cnf );


static int
my_widget_diagnostics_version(
         my_config_t *                 cnf );


/////////////////
//             //
//  Functions  //
//             //
/////////////////
// MARK: - Functions

int
my_widget_diagnostics(
         my_config_t *                 cnf )
{
   if (!(cnf))
      return(1);

   if ((my_widget_diagnostics_version(cnf)))
      return(1);
   if ((my_widget_diagnostics_stats(cnf)))
      return(1);
   if ((my_widget_diagnostics_get_pools(cnf)))
      return(1);
   if ((my_widget_diagnostics_list_conns(cnf)))
      return(1);
   if ((my_widget_diagnostics_list_sas(cnf)))
      return(1);

   if ((my_poll(cnf)))
      return(1);

   return(0);
}


int
my_widget_diagnostics_get_pools(
         my_config_t *                 cnf )
{
   int                     rc;
   const char *            command;

   if (!(cnf))
      return(1);
   command  = "get-pools";

   // initialize new command
   my_verbose(cnf, "initializing vici command \"%s\" ...\n", command);
   rc = davici_new_cmd(command, &cnf->davici_req);
   if (rc < 0)
   {  fprintf(stderr, "%s: %s\n", my_prog_name(cnf), strerror(-rc));
      return(1);
   };

   // add arguments
   davici_kv(cnf->davici_req, "leases", "yes", (unsigned)strlen("yes"));

   // queue command
   my_verbose(cnf, "queueing vici command \"%s\" ...\n", command);
   rc = davici_queue(cnf->davici_conn, cnf->davici_req, my_davici_cb_command, cnf);
   if (rc < 0)
   {  fprintf(stderr, "%s: %s\n", my_prog_name(cnf), strerror(-rc));
      my_verbose(cnf, "canceling vici command \"%s\" ...\n", command);
      davici_cancel(cnf->davici_req);
      return(1);
   };
   cnf->queued++;

   return(0);
}


int
my_widget_diagnostics_list_conns(
         my_config_t *                 cnf )
{
   int                     rc;
   const char *            command;
   const char *            event;

   if (!(cnf))
      return(1);
   command  = "list-conns";
   event    = "list-conn";

   // initialize new command
   my_verbose(cnf, "initializing vici command \"%s\" ...\n", command);
   rc = davici_new_cmd(command, &cnf->davici_req);
   if (rc < 0)
   {  fprintf(stderr, "%s: %s\n", my_prog_name(cnf), strerror(-rc));
      return(1);
   };

   // add arguments
   if ((cnf->ike_sa))
      davici_kv(cnf->davici_req, "ike", cnf->ike_sa, (unsigned)strlen(cnf->ike_sa));

   // queue command
   my_verbose(cnf, "queueing vici command \"%s\" with event \"%s\" ...\n", command, event);
   rc = davici_queue_streamed(cnf->davici_conn, cnf->davici_req, my_davici_cb_command, event, my_davici_cb_event, cnf);
   if (rc < 0)
   {  fprintf(stderr, "%s: %s\n", my_prog_name(cnf), strerror(-rc));
      my_verbose(cnf, "canceling vici command \"%s\" ...\n", command);
      davici_cancel(cnf->davici_req);
      return(1);
   };
   cnf->queued++;

   return(0);
}


int
my_widget_diagnostics_list_sas(
         my_config_t *                 cnf )
{
   int                     rc;
   const char *            command;
   const char *            event;

   if (!(cnf))
      return(1);
   command  = "list-sas";
   event    = "list-sa";

   // initialize new command
   my_verbose(cnf, "initializing vici command \"%s\" ...\n", command);
   rc = davici_new_cmd(command, &cnf->davici_req);
   if (rc < 0)
   {  fprintf(stderr, "%s: %s\n", my_prog_name(cnf), strerror(-rc));
      return(1);
   };

   // add arguments
   if ((cnf->ike_sa))
      davici_kv(cnf->davici_req, "ike", cnf->ike_sa, (unsigned)strlen(cnf->ike_sa));

   // queue command
   my_verbose(cnf, "queueing vici command \"%s\" with event \"%s\" ...\n", command, event);
   rc = davici_queue_streamed(cnf->davici_conn, cnf->davici_req, my_davici_cb_command, event, my_davici_cb_event, cnf);
   if (rc < 0)
   {  fprintf(stderr, "%s: %s\n", my_prog_name(cnf), strerror(-rc));
      my_verbose(cnf, "canceling vici command \"%s\" ...\n", command);
      davici_cancel(cnf->davici_req);
      return(1);
   };
   cnf->queued++;

   return(0);
}


int
my_widget_diagnostics_stats(
         my_config_t *                 cnf )
{
   int                     rc;
   const char *            command;

   if (!(cnf))
      return(1);
   command  = "stats";

   // initialize new command
   my_verbose(cnf, "initializing vici command \"%s\" ...\n", command);
   rc = davici_new_cmd(command, &cnf->davici_req);
   if (rc < 0)
   {  fprintf(stderr, "%s: %s\n", my_prog_name(cnf), strerror(-rc));
      return(1);
   };

   // queue command
   my_verbose(cnf, "queueing vici command \"%s\" ...\n", command);
   rc = davici_queue(cnf->davici_conn, cnf->davici_req, my_davici_cb_command, cnf);
   if (rc < 0)
   {  fprintf(stderr, "%s: %s\n", my_prog_name(cnf), strerror(-rc));
      my_verbose(cnf, "canceling vici command \"%s\" ...\n", command);
      davici_cancel(cnf->davici_req);
      return(1);
   };
   cnf->queued++;

   return(0);
}


int
my_widget_diagnostics_version(
         my_config_t *                 cnf )
{
   int                     rc;
   const char *            command;

   if (!(cnf))
      return(1);
   command  = "version";

   // initialize new command
   my_verbose(cnf, "initializing vici command \"%s\" ...\n", command);
   rc = davici_new_cmd(command, &cnf->davici_req);
   if (rc < 0)
   {  fprintf(stderr, "%s: %s\n", my_prog_name(cnf), strerror(-rc));
      return(1);
   };

   // queue command
   my_verbose(cnf, "queueing vici command \"%s\" ...\n", command);
   rc = davici_queue(cnf->davici_conn, cnf->davici_req, my_davici_cb_command, cnf);
   if (rc < 0)
   {  fprintf(stderr, "%s: %s\n", my_prog_name(cnf), strerror(-rc));
      my_verbose(cnf, "canceling vici command \"%s\" ...\n", command);
      davici_cancel(cnf->davici_req);
      return(1);
   };
   cnf->queued++;

   return(0);
}

/* end of source */
