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
#define __SRC_DAVICICTL_PARSER_C 1


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

//-------------------------//
// debug format prototypes //
//-------------------------//
#pragma mark debug format prototypes

static int
my_parse_res_debug(
         const char *                  name,
         struct davici_response *      res,
         my_config_t *                 cnf,
         int                           is_event );


static int
my_parse_res_debug_print(
         unsigned                      level,
         const char *                  name,
         const char *                  key,
         const char *                  val );


//------------------------//
// json format prototypes //
//------------------------//
#pragma mark json format prototypes

static int
my_parse_footer_json(
         my_config_t *                 cnf );


static int
my_parse_res_json(
         const char *                  name,
         struct davici_response *      res,
         my_config_t *                 cnf,
         int                           is_event );


static int
my_parse_res_json_delim(
         my_config_t *                 cnf,
         int                           level );


//------------------------//
// vici format prototypes //
//------------------------//
#pragma mark vici format prototypes

static int
my_parse_res_vici(
         const char *                  name,
         struct davici_response *      res,
         my_config_t *                 cnf,
         int                           is_event );


static int
my_parse_res_vici_delim(
         my_config_t *                 cnf,
         int                           level );


//-----------------------//
// xml format prototypes //
//-----------------------//
#pragma mark xml format prototypes

static int
my_parse_footer_xml(
         my_config_t *                 cnf );


static int
my_parse_res_xml(
         const char *                  name,
         struct davici_response *      res,
         my_config_t *                 cnf,
         int                           is_event );


static int
my_parse_res_xml_delim(
         my_config_t *                 cnf,
         int                           level );


static void
my_parse_res_xml_sect_free(
         char **                       sections );


static int
my_parse_res_xml_sect_set(
         my_config_t *                 cnf,
         char **                       sects,
         int                           level,
         const char *                  sect );


//-----------------------//
// yaml format prototypes //
//-----------------------//
#pragma mark yaml format prototypes

static int
my_parse_res_yaml(
         const char *                  name,
         struct davici_response *      res,
         my_config_t *                 cnf,
         int                           is_event );


static int
my_parse_res_yaml_delim(
         my_config_t *                 cnf,
         int                           level );


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
my_parse_footer(
         my_config_t *                 cnf )
{
   if (!(cnf->res_last_name))
      return(0);

   switch(cnf->format_out)
   {  case MY_FMT_JSON:    return(my_parse_footer_json(cnf));
      case MY_FMT_XML:     return(my_parse_footer_xml(cnf));
      default:             break;
   };

   return(0);
}


int
my_parse_res(
         const char *                  name,
         struct davici_response *      res,
         my_config_t *                 cnf,
         int                           is_event )
{
   switch(cnf->format_out)
   {  case MY_FMT_DEBUG:   return(my_parse_res_debug(name, res, cnf, is_event));
      case MY_FMT_JSON:    return(my_parse_res_json(name, res, cnf, is_event));
      case MY_FMT_VICI:    return(my_parse_res_vici(name, res, cnf, is_event));
      case MY_FMT_XML:     return(my_parse_res_xml(name, res, cnf, is_event));
      case MY_FMT_YAML:    return(my_parse_res_yaml(name, res, cnf, is_event));
      default:             break;
   };
   cnf->flags |= MY_FLG_PRETTY;
   return(my_parse_res_vici(name, res, cnf, is_event));
}


//------------------------//
// debug format functions //
//------------------------//
#pragma mark debug format functions

int
my_parse_res_debug(
         const char *                  name,
         struct davici_response *      res,
         my_config_t *                 cnf,
         int                           is_event )
{
   int               rc;
   char              val[4096];
   const char *      key;
   unsigned          level;
   char              title[64];

   if (!(cnf))
      return(0);

   my_strlcpy(title, "VICI ", sizeof(title));
   my_strlcat(title, (((is_event)) ? "Event" : "Reply"), sizeof(title));
   my_parse_res_debug_print(0, title, name, NULL);

   level = davici_get_level(res);

   while((rc = davici_parse(res)) >= 0)
   {  switch(rc)
      {  case DAVICI_END:
            return(0);

         case DAVICI_SECTION_START:
            key = davici_get_name(res);
            my_parse_res_debug_print(level, "DAVICI_SECTION_START", key, NULL);
            break;

         case DAVICI_SECTION_END:
            my_parse_res_debug_print(level, "DAVICI_SECTION_END", NULL, NULL);
            break;

         case DAVICI_KEY_VALUE:
            rc = davici_get_value_str(res, val, sizeof(val));
            if (rc < 0)
            {  fprintf(stderr, "%s: davici_get_value_str(): %s\n", PROGRAM_NAME, strerror(-rc));
               return(0);
            };
            key = davici_get_name(res);
            my_parse_res_debug_print(level, "DAVICI_KEY_VALUE", key, val);
            break;

         case DAVICI_LIST_START:
            key = davici_get_name(res);
            my_parse_res_debug_print(level, "DAVICI_LIST_START", key, NULL);
            break;

         case DAVICI_LIST_ITEM:
            rc = davici_get_value_str(res, val, sizeof(val));
            if (rc < 0)
            {  fprintf(stderr, "%s: davici_get_value_str(): %s\n", PROGRAM_NAME, strerror(-rc));
               return(0);
            };
            my_parse_res_debug_print(level, "DAVICI_LIST_ITEM", val, NULL);
            break;

         case DAVICI_LIST_END:
            my_parse_res_debug_print(level, "DAVICI_LIST_END", NULL, NULL);
            break;

         default:
            my_parse_res_debug_print(level, "UNKNOWN", NULL, NULL);
            break;
      };
      level = davici_get_level(res);
   };

   return(rc);
}


int
my_parse_res_debug_print(
         unsigned                      level,
         const char *                  name,
         const char *                  key,
         const char *                  val )
{
   char buff[64];

   my_strlcpy(buff, name,  sizeof(buff));
   my_strlcat(buff, ":",   sizeof(buff));

   if (!(key))
      return(printf("%-24s\n", buff));
   if (!(val))
      return(printf("%-24s%*s%s\n", buff, (level*3), "", key));
   return(printf("%-24s%*s%s = \"%s\"\n", buff, (level*3), "", key, val));
}


//-----------------------//
// json format functions //
//-----------------------//
#pragma mark json format functions

int
my_parse_footer_json(
         my_config_t *                 cnf )
{
   if ((cnf->widget->flags & MY_FLG_STREAM))
   {  printf(((cnf->flags & MY_FLG_PRETTY)) ? "\n]\n" : "]\n");
      return(0);
   };
   printf(((cnf->flags & MY_FLG_PRETTY)) ? "\n   }\n}\n" : "}}\n");
   return(0);
}


int
my_parse_res_json(
         const char *                  name,
         struct davici_response *      res,
         my_config_t *                 cnf,
         int                           is_event )
{
   int               rc;
   char              val[4096];
   const char *      key;
   unsigned          level;

   if (!(cnf))
      return(0);

   // print JSON header
   if (!(cnf->res_last_name))
      printf(((cnf->widget->flags & MY_FLG_STREAM)) ? "[" : "{");

   // print event/command section start
   if ((cnf->widget->flags & MY_FLG_STREAM))
   {  my_parse_res_json_delim(cnf, 0);
      printf("\"%s-%s\": {", name, (((is_event)) ? "event" : "reply"));
      cnf->last_was_item = 0;
   } else
   {  if ( (!(cnf->res_last_name)) || ((strcasecmp(name, cnf->res_last_name))) )
      {  my_parse_res_json_delim(cnf, 0);
         if ((cnf->res_last_name))
         {  printf("}");
            cnf->last_was_item = 1;
            my_parse_res_json_delim(cnf, 0);
         };
         printf("\"%s-%s\": {", name, (((is_event)) ? "event" : "reply"));
         cnf->last_was_item = 0;
      };
   };

   cnf->res_last_name = name;

   level = davici_get_level(res) + 1;

   while((rc = davici_parse(res)) >= 0)
   {  switch(rc)
      {  case DAVICI_END:
            if ((cnf->widget->flags & MY_FLG_STREAM))
            {  cnf->last_was_item = 0;
               my_parse_res_json_delim(cnf, 0);
               printf("}");
            };
            cnf->last_was_item = 1;
            return(0);

         case DAVICI_SECTION_START:
            key = davici_get_name(res);
            my_parse_res_json_delim(cnf, level);
            printf("\"%s\": {", key);
            cnf->last_was_item = 0;
            break;

         case DAVICI_SECTION_END:
            cnf->last_was_item = 0;
            my_parse_res_json_delim(cnf, level-1);
            printf("}");
            cnf->last_was_item = 1;
            break;

         case DAVICI_KEY_VALUE:
            rc = davici_get_value_str(res, val, sizeof(val));
            if (rc < 0)
            {  fprintf(stderr, "%s: davici_get_value_str(): %s\n", PROGRAM_NAME, strerror(-rc));
               return(0);
            };
            key = davici_get_name(res);
            my_parse_res_json_delim(cnf, level);
            printf("\"%s\": \"%s\"", key, val);
            cnf->last_was_item = 1;
            break;

         case DAVICI_LIST_START:
            key = davici_get_name(res);
            my_parse_res_json_delim(cnf, level);
            printf("\"%s\": [", key);
            cnf->last_was_item = 0;
            break;

         case DAVICI_LIST_ITEM:
            rc = davici_get_value_str(res, val, sizeof(val));
            if (rc < 0)
            {  fprintf(stderr, "%s: davici_get_value_str(): %s\n", PROGRAM_NAME, strerror(-rc));
               return(0);
            };
            my_parse_res_json_delim(cnf, level);
            printf("\"%s\"", val);
            cnf->last_was_item = 1;
            break;

         case DAVICI_LIST_END:
            cnf->last_was_item = 0;
            my_parse_res_json_delim(cnf, level-1);
            printf("]");
            cnf->last_was_item = 1;
            break;

         default:
            printf("UNKNOWN\n");
            cnf->last_was_item = 0;
            break;
      };
      level = davici_get_level(res) + 1;
   };

   return(rc);
}


int
my_parse_res_json_delim(
         my_config_t *                 cnf,
         int                           level )
{
   level++;
   if ((cnf->flags & MY_FLG_PRETTY))
      printf(((cnf->last_was_item)) ? ",\n%*s" : "\n%*s", (level*3), "");
   else
      printf(((cnf->last_was_item)) ? ", " : "");
   return(0);
}


//-----------------------//
// vici format functions //
//-----------------------//
#pragma mark vici format functions

int
my_parse_res_vici(
         const char *                  name,
         struct davici_response *      res,
         my_config_t *                 cnf,
         int                           is_event )
{
   int               rc;
   char              val[4096];
   const char *      key;
   unsigned          level;

   if (!(cnf))
      return(0);

   level = davici_get_level(res) + 1;

   printf("%s %s {", name, (((is_event)) ? "event" : "reply"));
   while((rc = davici_parse(res)) >= 0)
   {  switch(rc)
      {  case DAVICI_END:
            cnf->last_was_item = 0;
            my_parse_res_vici_delim(cnf, level-1);
            printf("}\n");
            return(0);

         case DAVICI_SECTION_START:
            key = davici_get_name(res);
            my_parse_res_vici_delim(cnf, level);
            printf("%s {", key);
            cnf->last_was_item = 0;
            break;

         case DAVICI_SECTION_END:
            cnf->last_was_item = 0;
            my_parse_res_vici_delim(cnf, level-1);
            printf("}");
            cnf->last_was_item = 1;
            break;

         case DAVICI_KEY_VALUE:
            rc = davici_get_value_str(res, val, sizeof(val));
            if (rc < 0)
            {  fprintf(stderr, "%s: davici_get_value_str(): %s\n", PROGRAM_NAME, strerror(-rc));
               return(0);
            };
            key = davici_get_name(res);
            my_parse_res_vici_delim(cnf, level);
            if ((cnf->flags & MY_FLG_PRETTY))
               printf("%s = %s", key, val);
            else
               printf("%s=%s", key, val);
            cnf->last_was_item = 1;
            break;

         case DAVICI_LIST_START:
            key = davici_get_name(res);
            my_parse_res_vici_delim(cnf, level);
            printf("%s = [", key);
            cnf->last_was_item = 0;
            break;

         case DAVICI_LIST_ITEM:
            rc = davici_get_value_str(res, val, sizeof(val));
            if (rc < 0)
            {  fprintf(stderr, "%s: davici_get_value_str(): %s\n", PROGRAM_NAME, strerror(-rc));
               return(0);
            };
            my_parse_res_vici_delim(cnf, level);
            printf("%s", val);
            cnf->last_was_item = 0;
            break;

         case DAVICI_LIST_END:
            my_parse_res_vici_delim(cnf, level-1);
            printf("]");
            cnf->last_was_item = 1;
            break;

         default:
            printf("UNKNOWN\n");
            cnf->last_was_item = 0;
            break;
      };
      level = davici_get_level(res) + 1;
   };

   return(rc);
}


int
my_parse_res_vici_delim(
         my_config_t *                 cnf,
         int                           level )
{
   if ((cnf->flags & MY_FLG_PRETTY))
      printf(((cnf->last_was_item)) ? ",\n%*s" : "\n%*s", (level*3), "");
   else
      printf(((cnf->last_was_item)) ? " " : "");
   return(0);
}


//----------------------//
// xml format functions //
//----------------------//
#pragma mark xml format functions

int
my_parse_footer_xml(
         my_config_t *                 cnf )
{
   if (!(cnf))
      return(0);
   printf(((cnf->flags & MY_FLG_PRETTY)) ? "\n</vici>\n" : "</vici>\n");
   return(0);
}


int
my_parse_res_xml(
         const char *                  name,
         struct davici_response *      res,
         my_config_t *                 cnf,
         int                           is_event )
{
   int               rc;
   char              val[4096];
   const char *      key;
   unsigned          level;
   char *            sects[128];

   if (!(cnf))
      return(0);

   memset(sects, 0, sizeof(sects));

   // print JSON header
   if (!(cnf->res_last_name))
   {  printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
      printf("<vici>");
   };

   // print event/command section start
   my_parse_res_xml_delim(cnf, 0);
   printf("<%s-%s>", name, (((is_event)) ? "event" : "reply"));

   cnf->res_last_name = name;

   level = davici_get_level(res) + 1;

   while((rc = davici_parse(res)) >= 0)
   {  switch(rc)
      {  case DAVICI_END:
            my_parse_res_xml_delim(cnf, level-1);
            printf("</%s-%s>", name, (((is_event)) ? "event" : "reply"));
            my_parse_res_xml_sect_free(sects);
            return(0);

         case DAVICI_SECTION_START:
            key = davici_get_name(res);
            if ((rc = my_parse_res_xml_sect_set(cnf, sects, level, key)) != 0)
            {  my_parse_res_xml_sect_free(sects);
               return(rc);
            };
            my_parse_res_xml_delim(cnf, level);
            printf("<%s>", sects[level]);
            break;

         case DAVICI_SECTION_END:
            key = davici_get_name(res);
            my_parse_res_xml_delim(cnf, level-1);
            printf("</%s>", sects[level-1]);
            break;

         case DAVICI_KEY_VALUE:
            rc = davici_get_value_str(res, val, sizeof(val));
            if (rc < 0)
            {  fprintf(stderr, "%s: davici_get_value_str(): %s\n", PROGRAM_NAME, strerror(-rc));
               my_parse_res_xml_sect_free(sects);
               return(rc);
            };
            key = davici_get_name(res);
            my_parse_res_xml_delim(cnf, level);
            printf("<%s>%s</%s>", key, val, key);
            break;

         case DAVICI_LIST_START:
            key = davici_get_name(res);
            if ((rc = my_parse_res_xml_sect_set(cnf, sects, level, key)) != 0)
            {  my_parse_res_xml_sect_free(sects);
               return(rc);
            };
            my_parse_res_xml_delim(cnf, level);
            printf("<%s>", sects[level]);
            break;

         case DAVICI_LIST_ITEM:
            rc = davici_get_value_str(res, val, sizeof(val));
            if (rc < 0)
            {  fprintf(stderr, "%s: davici_get_value_str(): %s\n", PROGRAM_NAME, strerror(-rc));
               my_parse_res_xml_sect_free(sects);
               return(rc);
            };
            my_parse_res_xml_delim(cnf, level);
            printf("<item>%s<item>", val);
            break;

         case DAVICI_LIST_END:
            key = davici_get_name(res);
            my_parse_res_xml_delim(cnf, level-1);
            printf("</%s>", sects[level-1]);
            break;

         default:
            printf("UNKNOWN\n");
            cnf->last_was_item = 0;
            break;
      };
      level = davici_get_level(res) + 1;
   };

   my_parse_res_xml_sect_free(sects);

   return(rc);
}


int
my_parse_res_xml_delim(
         my_config_t *                 cnf,
         int                           level )
{
   level++;
   if ((cnf->flags & MY_FLG_PRETTY))
      printf("\n%*s", (level*3), "");
   return(0);
}


void
my_parse_res_xml_sect_free(
         char **                       sections )
{
   int i;
   for(i = 0; (i < 128); i++)
   {  if ((sections[i]))
         free(sections[i]);
      sections[i] = NULL;
   };
   return;
}


int
my_parse_res_xml_sect_set(
         my_config_t *                 cnf,
         char **                       sects,
         int                           level,
         const char *                  sect )
{
   assert(level < 128);

   if ((sects[level]))
      sects[level] = NULL;

   if ((sects[level] = strdup(sect)) == NULL)
   {  fprintf(stderr, "%s: strdup(): %s\n", my_prog_name(cnf), strerror(errno));
      return(-errno);
   };

   return(0);
}


//-----------------------//
// yaml format functions //
//-----------------------//
#pragma mark yaml format functions

int
my_parse_res_yaml(
         const char *                  name,
         struct davici_response *      res,
         my_config_t *                 cnf,
         int                           is_event )
{
   int               rc;
   char              val[4096];
   const char *      key;
   unsigned          level;

   if (!(cnf))
      return(0);

   if (!(cnf->res_last_name))
      printf("---\n");

   if ( (!(cnf->res_last_name)) || ((strcasecmp(name, cnf->res_last_name))) )
   {  my_parse_res_yaml_delim(cnf, 0);
      printf(  "%s%s-%s:\n",
               (((cnf->widget->flags & MY_FLG_STREAM)) ? "- " : ""),
               name,
               (((is_event)) ? "event" : "reply")
            );
   };

   cnf->res_last_name = name;

   level = davici_get_level(res) + 1;

   while((rc = davici_parse(res)) >= 0)
   {  switch(rc)
      {  case DAVICI_END:
            cnf->last_was_item = 1;
            return(0);

         case DAVICI_SECTION_START:
            key = davici_get_name(res);
            my_parse_res_yaml_delim(cnf, level);
            printf("%s:\n", key);
            cnf->last_was_item = 0;
            break;

         case DAVICI_SECTION_END:
            cnf->last_was_item = 1;
            break;

         case DAVICI_KEY_VALUE:
            rc = davici_get_value_str(res, val, sizeof(val));
            if (rc < 0)
            {  fprintf(stderr, "%s: davici_get_value_str(): %s\n", PROGRAM_NAME, strerror(-rc));
               return(0);
            };
            key = davici_get_name(res);
            my_parse_res_yaml_delim(cnf, level);
            printf("%s: %s\n", key, val);
            cnf->last_was_item = 1;
            break;

         case DAVICI_LIST_START:
            key = davici_get_name(res);
            my_parse_res_yaml_delim(cnf, level);
            printf("%s:\n", key);
            cnf->last_was_item = 0;
            break;

         case DAVICI_LIST_ITEM:
            rc = davici_get_value_str(res, val, sizeof(val));
            if (rc < 0)
            {  fprintf(stderr, "%s: davici_get_value_str(): %s\n", PROGRAM_NAME, strerror(-rc));
               return(0);
            };
            my_parse_res_yaml_delim(cnf, level);
            printf("- %s\n", val);
            cnf->last_was_item = 0;
            break;

         case DAVICI_LIST_END:
            cnf->last_was_item = 1;
            break;

         default:
            printf("UNKNOWN\n");
            cnf->last_was_item = 0;
            break;
      };
      level = davici_get_level(res) + 1;
   };

   return(rc);
}


int
my_parse_res_yaml_delim(
         my_config_t *                 cnf,
         int                           level )
{
   if (!(cnf))
      return(0);
   printf(((cnf->last_was_item)) ? "%*s" : "%*s", (level*3), "");
   return(0);
}


/* end of source */
