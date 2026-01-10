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
#define __SRC_DAVICICTL_C 1


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


///////////////////
//               //
//  Definitions  //
//               //
///////////////////
// MARK: - Definitions

#undef   DAVUTL_SHORT_OPT
#define  DAVUTL_SHORT_OPT "hqs:Vv"

#undef   DAVUTL_LONG_OPT
#define  DAVUTL_LONG_OPT \
   { "help",            no_argument,         NULL, 'h' }, \
   { "quiet",           no_argument,         NULL, 'q' }, \
   { "silent",          no_argument,         NULL, 'q' }, \
   { "socket",          required_argument,   NULL, 's' }, \
   { "version",         no_argument,         NULL, 'V' }, \
   { "verbose",         no_argument,         NULL, 'v' }, \
   { NULL, 0, NULL, 0 }


//////////////
//          //
//  Macros  //
//          //
//////////////
// MARK: - Macros

#undef   DAVUTL_LONG
#define  DAVUTL_LONG(...) (const struct option []) { __VA_ARGS__ DAVUTL_LONG_OPT }


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

extern int
main(
         int                           argc,
         char **                       argv );


static int
my_arguments(
         my_config_t *                 cnf,
         int                           argc,
         char * const *                argv );


static void
my_free(
         my_config_t *                 cnf );


static my_widget_t *
my_lookup_widget(
         const char *                  wname,
         int                           exact );


static void
my_signal_handler(
         int                           sig );


static int
my_usage(
         my_config_t *                 cnf );


static int
my_version(
         my_config_t *                 cnf );


//-------------------//
// davici prototypes //
//-------------------//
#pragma mark davici prototypes

static int
my_davici_fdcb(
         struct davici_conn *          conn,
         int                           fd,
         int                           ops,
         void *                        user );


//-------------------//
// parser prototypes //
//-------------------//
#pragma mark parser prototypes

static int
my_parse_response(
         const char *                  name,
         struct davici_response *      res,
         my_config_t *                 cnf );


//--------------------//
// widgets prototypes //
//--------------------//
#pragma mark widgets prototypes

static int
my_widget_generic_command(
         my_config_t *                 cnf );


static int
my_widget_generic_event(
         my_config_t *                 cnf );


/////////////////
//             //
//  Variables  //
//             //
/////////////////
// MARK: - Variables

#pragma mark my_should_exit
static int my_should_exit = 0;


#pragma mark my_widget_map[]
static my_widget_t my_widget_map[] =
{
   // alert widget
   {  .name          = "alert",
      .aliases       = NULL,
      .desc          = "displays alert events",
      .davici_cmd    = NULL,
      .davici_event  = "alert",
      .usage         = "[OPTIONS]",
      .short_opt     = NULL,
      .long_opt      = NULL,
      .arg_min       = 0,
      .arg_max       = 0,
      .func_exec     = &my_widget_generic_event,
      .func_usage    = NULL,
   },

   // child-updown widget
   {  .name          = "child-updown",
      .aliases       = NULL,
      .desc          = "displays child-updown events",
      .davici_cmd    = NULL,
      .davici_event  = "child-updown",
      .usage         = "[OPTIONS]",
      .short_opt     = NULL,
      .long_opt      = NULL,
      .arg_min       = 0,
      .arg_max       = 0,
      .func_exec     = &my_widget_generic_event,
      .func_usage    = NULL,
   },

   // child-rekey widget
   {  .name          = "child-rekey",
      .aliases       = NULL,
      .desc          = "displays child-rekey events",
      .davici_cmd    = NULL,
      .davici_event  = "child-rekey",
      .usage         = "[OPTIONS]",
      .short_opt     = NULL,
      .long_opt      = NULL,
      .arg_min       = 0,
      .arg_max       = 0,
      .func_exec     = &my_widget_generic_event,
      .func_usage    = NULL,
   },

   // clear-creds widget
   {  .name          = "clear-creds",
      .aliases       = NULL,
      .desc          = "clears loaded certs, private keys and shared keys",
      .davici_cmd    = "clear-creds",
      .davici_event  = NULL,
      .usage         = "[OPTIONS]",
      .short_opt     = NULL,
      .long_opt      = NULL,
      .arg_min       = 0,
      .arg_max       = 0,
      .func_exec     = &my_widget_generic_command,
      .func_usage    = NULL,
   },

   // get-algorithms widget
   {  .name          = "get-algorithms",
      .aliases       = NULL,
      .desc          = "lists loaded algorithms and their implementation",
      .davici_cmd    = "get-algorithms",
      .davici_event  = NULL,
      .usage         = "[OPTIONS]",
      .short_opt     = NULL,
      .long_opt      = NULL,
      .arg_min       = 0,
      .arg_max       = 0,
      .func_exec     = &my_widget_generic_command,
      .func_usage    = NULL,
   },

   // get-authorities widget
   {  .name          = "get-authorities",
      .aliases       = NULL,
      .desc          = "lists loaded CA names",
      .davici_cmd    = "get-authorities",
      .davici_event  = NULL,
      .usage         = "[OPTIONS]",
      .short_opt     = NULL,
      .long_opt      = NULL,
      .arg_min       = 0,
      .arg_max       = 0,
      .func_exec     = &my_widget_generic_command,
      .func_usage    = NULL,
   },

   // get-conns widget
   {  .name          = "get-conns",
      .aliases       = NULL,
      .desc          = "lists connections loaded over vici",
      .davici_cmd    = "get-conns",
      .davici_event  = NULL,
      .usage         = "[OPTIONS]",
      .short_opt     = NULL,
      .long_opt      = NULL,
      .arg_min       = 0,
      .arg_max       = 0,
      .func_exec     = &my_widget_generic_command,
      .func_usage    = NULL,
   },

   // get-keys widget
   {  .name          = "get-keys",
      .aliases       = NULL,
      .desc          = "lists private keys identifiers loaded over vici",
      .davici_cmd    = "get-keys",
      .davici_event  = NULL,
      .usage         = "[OPTIONS]",
      .short_opt     = NULL,
      .long_opt      = NULL,
      .arg_min       = 0,
      .arg_max       = 0,
      .func_exec     = &my_widget_generic_command,
      .func_usage    = NULL,
   },

   // get-shared widget
   {  .name          = "get-shared",
      .aliases       = NULL,
      .desc          = "lists unique shared key identifiers loaded over vici",
      .davici_cmd    = "get-shared",
      .davici_event  = NULL,
      .usage         = "[OPTIONS]",
      .short_opt     = NULL,
      .long_opt      = NULL,
      .arg_min       = 0,
      .arg_max       = 0,
      .func_exec     = &my_widget_generic_command,
      .func_usage    = NULL,
   },

   // log widget
   {  .name          = "log",
      .aliases       = NULL,
      .desc          = "displays debug log messages",
      .davici_cmd    = NULL,
      .davici_event  = "log",
      .usage         = "[OPTIONS]",
      .short_opt     = NULL,
      .long_opt      = NULL,
      .arg_min       = 0,
      .arg_max       = 0,
      .func_exec     = &my_widget_generic_event,
      .func_usage    = NULL,
   },

   // ike-rekey widget
   {  .name          = "ike-rekey",
      .aliases       = NULL,
      .desc          = "displays ike-rekey events",
      .davici_cmd    = NULL,
      .davici_event  = "ike-rekey",
      .usage         = "[OPTIONS]",
      .short_opt     = NULL,
      .long_opt      = NULL,
      .arg_min       = 0,
      .arg_max       = 0,
      .func_exec     = &my_widget_generic_event,
      .func_usage    = NULL,
   },

   // ike-update widget
   {  .name          = "ike-update",
      .aliases       = NULL,
      .desc          = "displays ike-update events",
      .davici_cmd    = NULL,
      .davici_event  = "ike-update",
      .usage         = "[OPTIONS]",
      .short_opt     = NULL,
      .long_opt      = NULL,
      .arg_min       = 0,
      .arg_max       = 0,
      .func_exec     = &my_widget_generic_event,
      .func_usage    = NULL,
   },

   // ike-updown widget
   {  .name          = "ike-updown",
      .aliases       = NULL,
      .desc          = "displays ike-updown events",
      .davici_cmd    = NULL,
      .davici_event  = "ike-updown",
      .usage         = "[OPTIONS]",
      .short_opt     = NULL,
      .long_opt      = NULL,
      .arg_min       = 0,
      .arg_max       = 0,
      .func_exec     = &my_widget_generic_event,
      .func_usage    = NULL,
   },

   // reload-settings widget
   {  .name          = "reload-settings",
      .aliases       = NULL,
      .desc          = "reloads strongswan.conf settings and plugins",
      .davici_cmd    = "reload-settings",
      .davici_event  = NULL,
      .usage         = "[OPTIONS]",
      .short_opt     = NULL,
      .long_opt      = NULL,
      .arg_min       = 0,
      .arg_max       = 0,
      .func_exec     = &my_widget_generic_command,
      .func_usage    = NULL,
   },

   // stats widget
   {  .name          = "stats",
      .aliases       = NULL,
      .desc          = "returns IKE daemon statistics",
      .davici_cmd    = "stats",
      .davici_event  = NULL,
      .usage         = "[OPTIONS]",
      .short_opt     = NULL,
      .long_opt      = NULL,
      .arg_min       = 0,
      .arg_max       = 0,
      .func_exec     = &my_widget_generic_command,
      .func_usage    = NULL,
   },

   // version widget
   {  .name          = "version",
      .aliases       = NULL,
      .desc          = "returns daemon and system versions",
      .davici_cmd    = "version",
      .davici_event  = NULL,
      .usage         = "[OPTIONS]",
      .short_opt     = NULL,
      .long_opt      = NULL,
      .arg_min       = 0,
      .arg_max       = 0,
      .func_exec     = &my_widget_generic_command,
      .func_usage    = NULL,
   },

   {  .name          = NULL,
      .aliases       = NULL,
      .desc          = NULL,
      .davici_cmd    = NULL,
      .davici_event  = NULL,
      .usage         = NULL,
      .short_opt     = NULL,
      .long_opt      = NULL,
      .arg_min       = 0,
      .arg_max       = 0,
      .func_exec     = NULL,
      .func_usage    = NULL,
   },
};


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
   my_config_t *              cnf;
   const char *               prog_name;

   // determine program name
   if ((prog_name = strrchr(argv[0], '/')) != NULL)
      prog_name = &prog_name[1];
   if (!(prog_name))
      prog_name = argv[0];

   // allocate and initialize config memory
   if ((cnf = malloc(sizeof(my_config_t))) == NULL)
   {  fprintf(stderr, "%s; out of virtual memory\n", prog_name);
      return(1);
   };
   memset(cnf, 0, sizeof(my_config_t));
   cnf->argc            = argc;
   cnf->argv            = argv;
   cnf->prog_name       = prog_name;
   cnf->vici_sockpath   = MY_SOCK_PATH;
   cnf->pollfd.fd       = -1;

   // check for symlink alias
   if ((cnf->widget = my_lookup_widget(cnf->prog_name, 1)) != NULL)
      cnf->symlinked = 1;

   // processing common cli arguments
   if (!(cnf->widget))
   {  if ((rc = my_arguments(cnf, argc, argv)) != 0)
      {  my_free(cnf);
         return((rc == -1) ? 0 : 1);
      };
      if ((cnf->widget = my_lookup_widget(cnf->argv[0], 0)) == NULL)
      {  fprintf(stderr, "%s: unknown or missing widget name\n", PROGRAM_NAME);
         fprintf(stderr, "Try `%s --help' for more information.\n", cnf->prog_name);
         my_free(cnf);
         return(1);
      };
   };

   // processing widget cli arguments
   if ((rc = my_arguments(cnf, cnf->argc, cnf->argv)) != 0)
   {  my_free(cnf);
      return((rc == -1) ? 0 : 1);
   };

   // set signal handlers
   signal(SIGHUP,    my_signal_handler);
   signal(SIGINT,    my_signal_handler);
   signal(SIGTERM,   my_signal_handler);
   signal(SIGUSR1,   SIG_IGN);
   signal(SIGUSR2,   SIG_IGN);
   signal(SIGPIPE,   SIG_IGN);

   // connect to vici socket
   my_verbose(cnf, "connecting to vici socket ...\n");
   rc = davici_connect_unix(cnf->vici_sockpath, my_davici_fdcb, cnf, &cnf->davici_conn);
   if (rc < 0)
   {  fprintf(stderr, "%s: %s\n", my_prog_name(cnf), strerror(-rc));
      return(1);
   };

   rc = cnf->widget->func_exec(cnf);

   my_free(cnf);

   return(rc);
}


int
my_arguments(
         my_config_t *                 cnf,
         int                           argc,
         char * const *                argv )
{
   int                        c;
   int                        opt_index;
   const struct option *      long_opt;
   const char *               short_opt;
   const my_widget_t *        widget;

   widget         = cnf->widget;
   optind         = 0;
   opt_index      = 0;
   short_opt      = ( ((widget)) && ((widget->short_opt)) )
                  ? widget->short_opt
                  : "+" DAVUTL_SHORT_OPT;
   long_opt       = ( ((widget)) && ((widget->long_opt)) )
                  ? cnf->widget->long_opt
                  : DAVUTL_LONG();

   while((c = getopt_long(argc, argv, short_opt, long_opt, &opt_index)) != -1)
   {  switch(c)
      {  case -1:       /* no more arguments */
         case 0:        /* long options toggles */
         break;

         case 'h':
            my_usage(cnf);
            return(-1);

         case 'q':
            cnf->quiet = 1;
            if ((cnf->verbose))
            {  fprintf(stderr, "%s: incompatible options `-q' and `-v'\n", my_prog_name(cnf));
               fprintf(stderr, "Try `%s --help' for more information.\n",  my_prog_name(cnf));
               return(1);
            };
            break;

         case 's':
            cnf->vici_sockpath = optarg;
            break;

         case 'V':
            my_version(cnf);
            return(-1);

         case 'v':
            cnf->verbose++;
            if ((cnf->quiet))
            {  fprintf(stderr, "%s: incompatible options `-q' and `-v'\n", my_prog_name(cnf));
               fprintf(stderr, "Try `%s --help' for more information.\n", my_prog_name(cnf));
               return(1);
            };
            break;

         case '?':
            fprintf(stderr, "Try `%s --help' for more information.\n", my_prog_name(cnf));
            return(1);

         default:
            fprintf(stderr, "%s: unrecognized option `--%c'\n", my_prog_name(cnf), c);
            fprintf(stderr, "Try `%s --help' for more information.\n", my_prog_name(cnf));
            return(1);
      };
   };

   cnf->argc   = (argc - optind);
   cnf->argv   = &argv[optind];

   if (!(widget))
   {  if (cnf->argc < 1)
      {  fprintf(stderr, "%s: missing required argument\n", my_prog_name(cnf));
         fprintf(stderr, "Try `%s --help' for more information.\n", my_prog_name(cnf));
         return(1);
      };
      return(0);
   };

   if (cnf->argc < widget->arg_min)
   {  fprintf(stderr, "%s: missing required argument\n", my_prog_name(cnf));
      fprintf(stderr, "Try `%s --help' for more information.\n", my_prog_name(cnf));
      return(1);
   };
   if ( (cnf->argc > widget->arg_max) && (widget->arg_max >= widget->arg_min) )
   {  fprintf(stderr, "%s: unknown argument -- `%s'\n", my_prog_name(cnf), cnf->argv[widget->arg_max]);
      fprintf(stderr, "Try `%s --help' for more information.\n", my_prog_name(cnf));
      return(1);
   };

   return(0);
}


void
my_free(
         my_config_t *                 cnf )
{
   if (!(cnf))
      return;

   if ((cnf->davici_conn))
   {  my_verbose(cnf, "disconnecting from vici socket ...\n");
      davici_disconnect(cnf->davici_conn);
   };

   free(cnf);

   return;
}


my_widget_t *
my_lookup_widget(
         const char *                  wname,
         int                           exact )
{
   int                        x;
   int                        y;
   size_t                     len;
   size_t                     wname_len;
   const char *               alias;
   my_widget_t *              match;
   my_widget_t *              widget;

   // strip program prefix from widget name
   len = strlen(PROGRAM_NAME);
   if (!(strncasecmp(wname, PROGRAM_NAME, len)))
   {
      wname = &wname[len];
      if (wname[0] == '-')
         wname = &wname[1];
   } else
   {
      len = strlen("davici");
      if (!(strncasecmp(wname, "davici", len)))
         wname = &wname[len];
   };
   if (!(wname[0]))
      return(NULL);

   match       = NULL;
   wname_len   = strlen(wname);

   // loop through widgets looking for match
   for(x = 0; ((my_widget_map[x].name)); x++)
   {  // check widget
      widget = &my_widget_map[x];
      if (widget->func_exec == NULL)
         continue;
      widget->alias_idx = -1;

      // compare widget name for match
      if (!(strncmp(widget->name, wname, wname_len)))
      {  if (widget->name[wname_len] == '\0')
            return(widget);
         if ( ((match)) && (match != widget) )
            return(NULL);
         if (exact == 0)
            match = widget;
      };

      if (!(widget->aliases))
         continue;

      for(y = 0; ((widget->aliases[y])); y++)
      {  alias = widget->aliases[y];
         if (!(strncmp(alias, wname, wname_len)))
         {  if (alias[wname_len] == '\0')
            {  widget->alias_idx = y;
               return(widget);
            };
            if ( ((match)) && (match != widget) )
               return(NULL);
            if (exact == 0)
               match = widget;
         };
      };
   };

   return((exact == 0) ? match : NULL);
}


int
my_poll(
         my_config_t *                 cnf )
{
   int rc;

   // poll for responses
   my_verbose(cnf, "entering polling loop ...\n");
   while ( (!(my_should_exit)) && (cnf->pollfd.fd != -1) )
   {  if ((rc = poll(&cnf->pollfd, 1, 30000)) < 0)
      {  fprintf(stderr, "%s: poll(): %s\n", my_prog_name(cnf), strerror(errno));
         return(1);
      };
      if ((cnf->pollfd.revents & POLLIN))
      {  my_verbose(cnf, "reading data from vici socket ...\n");
         if ((rc = davici_read(cnf->davici_conn)) < 0)
         {  fprintf(stderr, "%s: davici_read(): %s\n", my_prog_name(cnf), strerror(-rc));
            return(1);
         };
      };
      if ((cnf->pollfd.revents & POLLOUT))
      {  my_verbose(cnf, "writing data to vici socket ...\n");
         if ((rc = davici_write(cnf->davici_conn)) < 0)
         {  fprintf(stderr, "%s: davici_write(): %s\n", my_prog_name(cnf), strerror(-rc));
            return(1);
         };
      };
   };

   return(0);
}


char *
my_prog_name(
         my_config_t *                 cnf )
{
   static char    buff[512];

   const char * prog_name;

   prog_name   = ((cnf->prog_name))
               ? cnf->prog_name
               : PROGRAM_NAME;

   if ( (!(cnf->widget)) || ((cnf->symlinked)) )
   {  snprintf(buff, sizeof(buff), "%s", prog_name);
      return(buff);
   };

   snprintf(buff, sizeof(buff), "%s %s", prog_name, cnf->widget->name);

   return(buff);
}


void
my_signal_handler(
         int                           sig )
{
   my_should_exit = 1;
   switch(sig)
   {  case SIGHUP:   printf("%s: caught SIGHUP signal\n",  PROGRAM_NAME); break;
      case SIGINT:   printf("%s: caught SIGINT signal\n",  PROGRAM_NAME); break;
      case SIGTERM:  printf("%s: caught SIGTERM signal\n", PROGRAM_NAME); break;
      default:       printf("%s: caught unknown signal\n", PROGRAM_NAME); break;
   };
   signal(sig, my_signal_handler);
   return;
}


int
my_usage(
         my_config_t *                 cnf )
{
   size_t                     pos;
   const char *               widget_name;
   const char *               widget_help;
   const char *               short_opt;
   const my_widget_t *        widget;

   assert(cnf != NULL);

   widget_name  = (!(cnf->widget)) ? "widget"               : cnf->widget->name;
   short_opt    = ((cnf->widget))  ? cnf->widget->short_opt : DAVUTL_SHORT_OPT;
   short_opt    = ((short_opt))    ? short_opt              : DAVUTL_SHORT_OPT;
   widget_help  = "";
   if ((cnf->widget))
      widget_help = ((cnf->widget->usage)) ? cnf->widget->usage : "";

   if ((widget = cnf->widget) == NULL)
   {  printf("Usage: %s [OPTIONS] <address> [ <address> [ ... <address> ] ]\n", PROGRAM_NAME);
      printf("       %s [OPTIONS] %s %s\n", PROGRAM_NAME, widget_name, widget_help);
      printf("       davici-%s %s\n", widget_name, widget_help);
      printf("       davici%s %s\n", widget_name, widget_help);
   } else if (cnf->symlinked == 0)
   {  widget_name = (widget->alias_idx == -1) ? widget_name : widget->aliases[widget->alias_idx];
      printf("Usage: %s %s %s\n", PROGRAM_NAME, widget_name, widget_help);
   }
   else
   {  printf("Usage: %s %s\n", cnf->prog_name, widget_help);
   };
   printf("OPTIONS:\n");
   if ((strchr(short_opt, 'h'))) printf("  -h, --help                print this help and exit\n");
   if ((strchr(short_opt, 'q'))) printf("  -q, --quiet, --silent     do not print messages\n");
   if ((strchr(short_opt, 's'))) printf("  -s path, --socket=path    path to vici socket\n");
   if ((strchr(short_opt, 'V'))) printf("  -V, --version             print version number and exit\n");
   if ((strchr(short_opt, 'v'))) printf("  -v, --verbose             print verbose messages\n");
   if (!(cnf->widget))
   {  printf("WIDGETS:\n");
      for(pos = 0; my_widget_map[pos].name != NULL; pos++)
      {  widget = &my_widget_map[pos];
         if ((widget->desc))
            printf("  %-25s %s\n", widget->name, widget->desc);
      };
      printf("\n");
      return(0);
   };

   if ((cnf->widget))
      if ((cnf->widget->func_usage))
         cnf->widget->func_usage(cnf);

   return(0);
}


void
my_verbose(
         my_config_t *                 cnf,
         const char *                  fmt,
         ... )
{
   va_list     args;
   if (!(cnf->verbose))
      return;
   va_start(args, fmt);
   vfprintf(stderr, fmt, args);
   va_end(args);
   return;
}


int
my_version(
         my_config_t *                 cnf )
{
   const char * prog_name;
   prog_name = ((cnf)) ? cnf->prog_name : PROGRAM_NAME;
   printf("%s (%s) %s\n", prog_name, PACKAGE_NAME, PACKAGE_VERSION);
   printf("%s\n", PACKAGE_COPYRIGHT);
   printf("All rights reserved.\n");
   printf("\n");
   return(0);
}


//------------------//
// davici functions //
//------------------//
#pragma mark davici functions

void
my_davici_cb_command(
         struct davici_conn *          conn,
         int                           err,
         const char *                  name,
         struct davici_response *      res,
         void *                        user )
{
   int            rc;
   my_config_t *  cnf;

   if (!(conn))
      return;

   cnf = (my_config_t *)user;

   my_verbose(cnf, "processing results of \"%s\" command ...\n", name);

   cnf->queued--;
   if (cnf->queued <= 0)
      my_should_exit = 1;

   if (err < 0)
   {  fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, name, strerror(-err));
      return;
   };

   if (!(res))
      return;

   rc = my_parse_response(name, res, cnf);
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
   my_config_t *     cnf;

   if (!(conn))
      return;

   cnf = (my_config_t *)user;

   my_verbose(cnf, "processing results of \"%s\" event ...\n", name);

   if (err < 0)
   {  fprintf(stderr, "%s: %s: %s\n", PROGRAM_NAME, name, strerror(-err));
      return;
   };

   if (!(res))
      return;

   rc = my_parse_response(name, res, cnf);
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
   my_config_t *  cnf;

   if (!(conn))
      return(0);

   cnf                      = (my_config_t *)user;
   cnf->pollfd.events     = ((ops & DAVICI_READ))      ? POLLIN    :  0;
   cnf->pollfd.events    |= ((ops & DAVICI_WRITE))     ? POLLOUT   :  0;
   cnf->pollfd.fd         = ((cnf->pollfd.events))   ? fd        : -1;

   return(0);
}


//-------------------//
// parser functions //
//-------------------//
#pragma mark parser functions

int
my_parse_response(
         const char *                  name,
         struct davici_response *      res,
         my_config_t *                 cnf )
{
   int               rc;
   char              str[4096];
   const char *      key;

   if (!(cnf))
      return(0);

   printf("command %s\n", name);
   while((rc = davici_parse(res)) >= 0)
   {  switch(rc)
      {  case DAVICI_END:
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


//-------------------//
// widgets functions //
//-------------------//
#pragma mark widgets functions

int
my_widget_generic_command(
         my_config_t *                 cnf )
{
   int               rc;
   const my_widget_t *     widget;

   if (!(cnf))
      return(1);
   widget   = cnf->widget;

   // initialize new command
   my_verbose(cnf, "initializing vici command \"%s\" ...\n", widget->davici_cmd);
   rc = davici_new_cmd(widget->davici_cmd, &cnf->davici_req);
   if (rc < 0)
   {  fprintf(stderr, "%s: %s\n", my_prog_name(cnf), strerror(-rc));
      return(1);
   };

   // queue command
   if (!(widget->davici_event))
   {  my_verbose(cnf, "queueing vici command \"list-sas\" with event \"%s\" ...\n", widget->davici_cmd);
      rc = davici_queue(cnf->davici_conn, cnf->davici_req, my_davici_cb_command, cnf);
   } else
   {  my_verbose(cnf, "queueing vici command \"%s\" with event \"%s\" ...\n", widget->davici_cmd, widget->davici_event);
      rc = davici_queue_streamed(cnf->davici_conn, cnf->davici_req, my_davici_cb_command, widget->davici_event, my_davici_cb_event, cnf);
   };
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


int
my_widget_generic_event(
         my_config_t *                 cnf )
{
   int               rc;
   const my_widget_t *     widget;

   if (!(cnf))
      return(1);
   widget   = cnf->widget;

   // register new event
   my_verbose(cnf, "registering vici event \"%s\" ...\n", widget->davici_event);
   rc = davici_register(cnf->davici_conn, widget->davici_event, my_davici_cb_event, cnf);
   if (rc < 0)
   {  fprintf(stderr, "%s: %s\n",  my_prog_name(cnf), strerror(-rc));
      return(1);
   };

   if ((my_poll(cnf)))
      return(1);

   return(0);
}


/* end of source */
