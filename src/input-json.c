/*
** Copyright (C) 2009-2019 Quadrant Information Security <quadrantsec.com>
** Copyright (C) 2009-2019 Champ Clark III <cclark@quadrantsec.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/* Read data from fifo in a JSON format */

#ifdef HAVE_CONFIG_H
#include "config.h"             /* From autoconf */
#endif

#ifdef HAVE_LIBFASTJSON

#include <stdio.h>
#include <string.h>

#include "sagan.h"
#include "sagan-defs.h"
#include "sagan-config.h"
#include "version.h"
#include "input-pipe.h"

struct _SaganCounters *counters;
struct _SaganConfig *config;
struct _SaganDebug *debug;

struct _Syslog_JSON_Map *Syslog_JSON_Map;

void SyslogInput_JSON( char *syslog_string, struct _Sagan_Proc_Syslog *SaganProcSyslog_LOCAL )
{

    struct json_object *json_obj = NULL;
    struct json_object *json_obj2 = NULL;
    struct json_object *tmp = NULL;

    const char *val_str = NULL;

    uint16_t json_str_count=1;
    uint16_t a;

    bool has_message = false;

    char json_str[JSON_MAX_NEST][JSON_MAX_SIZE] = { { 0 } };

    memset(SaganProcSyslog_LOCAL, 0, sizeof(_Sagan_Proc_Syslog));

    //memcpy(SaganProcSyslog_LOCAL->syslog_message, "UNDEFINED\0", MAX_SYSLOGMSG);
    memcpy(SaganProcSyslog_LOCAL->syslog_program, "UNDEFINED\0", sizeof(SaganProcSyslog_LOCAL->syslog_program));
    memcpy(SaganProcSyslog_LOCAL->syslog_time, "UNDEFINED\0", sizeof(SaganProcSyslog_LOCAL->syslog_time));
    memcpy(SaganProcSyslog_LOCAL->syslog_date, "UNDEFINED\0", sizeof(SaganProcSyslog_LOCAL->syslog_date));
    memcpy(SaganProcSyslog_LOCAL->syslog_tag, "UNDEFINED\0", sizeof(SaganProcSyslog_LOCAL->syslog_tag));
    memcpy(SaganProcSyslog_LOCAL->syslog_level, "UNDEFINED\0", sizeof(SaganProcSyslog_LOCAL->syslog_level));
    memcpy(SaganProcSyslog_LOCAL->syslog_priority, "UNDEFINED\0", sizeof(SaganProcSyslog_LOCAL->syslog_priority));
    memcpy(SaganProcSyslog_LOCAL->syslog_facility, "UNDEFINED\0", sizeof(SaganProcSyslog_LOCAL->syslog_facility));
    memcpy(SaganProcSyslog_LOCAL->syslog_host, "0.0.0.0\0", sizeof(SaganProcSyslog_LOCAL->syslog_host));

   SaganProcSyslog_LOCAL->md5[0] = '\0';

    /* If the json isn't nested,  we can do this the easy way */

    if ( Syslog_JSON_Map->is_nested == false )
        {

            json_obj = json_tokener_parse(syslog_string);

            if ( json_obj == NULL )
                {

                    if ( syslog_string != NULL )
                        {
                            Sagan_Log(WARN, "[%s, line %d] Libfastjson failed to decode JSON. Got: %s", __FILE__, __LINE__, syslog_string);
                        }
                    else
                        {
                            Sagan_Log(WARN, "[%s, line %d] Libfastjson failed to decode JSON. Got NULL data.", __FILE__, __LINE__);
                        }


                    json_object_put(json_obj);

                    __atomic_add_fetch(&counters->malformed_json_input_count, 1, __ATOMIC_SEQ_CST);
                    return;
                }


            __atomic_add_fetch(&counters->json_input_count, 1, __ATOMIC_SEQ_CST);

            if ( json_object_object_get_ex(json_obj, Syslog_JSON_Map->syslog_map_host, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->syslog_host, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->syslog_host));
                }

            if ( json_object_object_get_ex(json_obj, Syslog_JSON_Map->syslog_map_facility, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->syslog_facility, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->syslog_facility));
                }

            if ( json_object_object_get_ex(json_obj, Syslog_JSON_Map->syslog_map_priority, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->syslog_priority, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->syslog_priority));
                }

            if ( json_object_object_get_ex(json_obj, Syslog_JSON_Map->syslog_map_level, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->syslog_level, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->syslog_level));
                }

            if ( json_object_object_get_ex(json_obj, Syslog_JSON_Map->syslog_map_tag, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->syslog_tag, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->syslog_tag));
                }

            if ( json_object_object_get_ex(json_obj, Syslog_JSON_Map->syslog_map_date, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->syslog_date, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->syslog_date));
                }

            if ( json_object_object_get_ex(json_obj, Syslog_JSON_Map->src_ip, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->src_ip, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->src_ip));
                }

            if ( json_object_object_get_ex(json_obj, Syslog_JSON_Map->dst_ip, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->dst_ip, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->dst_ip));
                }

            if ( json_object_object_get_ex(json_obj, Syslog_JSON_Map->src_port, &tmp))
                {
                    SaganProcSyslog_LOCAL->src_port = atoi( json_object_get_string(tmp) );
                }

            if ( json_object_object_get_ex(json_obj, Syslog_JSON_Map->dst_port, &tmp))
                {
                    SaganProcSyslog_LOCAL->dst_port = atoi( json_object_get_string(tmp) );
                }

            if ( json_object_object_get_ex(json_obj, Syslog_JSON_Map->syslog_map_time, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->syslog_time, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->syslog_time));
                }

            if ( json_object_object_get_ex(json_obj, Syslog_JSON_Map->syslog_map_program, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->syslog_program, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->syslog_program));
                }

            if ( !strcmp(Syslog_JSON_Map->syslog_map_message, "%JSON%" ) )
                {
                    strlcpy(SaganProcSyslog_LOCAL->syslog_message, syslog_string, sizeof(SaganProcSyslog_LOCAL->syslog_message));
                    has_message = true;

                }


            else if ( json_object_object_get_ex(json_obj, Syslog_JSON_Map->syslog_map_message, &tmp))
                {


                    snprintf(SaganProcSyslog_LOCAL->syslog_message, sizeof(SaganProcSyslog_LOCAL->syslog_message)," %s", json_object_get_string(tmp));
                    SaganProcSyslog_LOCAL->syslog_message[ (sizeof(SaganProcSyslog_LOCAL->syslog_message) -1 ) ] = '\0';
                    has_message = true;
                }

            if ( json_object_object_get_ex(json_obj, Syslog_JSON_Map->proto, &tmp))
                {

                    if ( !strcmp( json_object_get_string(tmp) , "tcp" ) || !strcmp( json_object_get_string(tmp), "TCP" ) )
                        {
                            SaganProcSyslog_LOCAL->proto = 6;
                        }

                    else if ( !strcmp( json_object_get_string(tmp), "udp" ) || !strcmp( json_object_get_string(tmp), "UDP" ) )
                        {
                            SaganProcSyslog_LOCAL->proto = 17;
                        }

                    else if ( !strcmp( json_object_get_string(tmp), "icmp" ) || !strcmp( json_object_get_string(tmp), "ICMP" ) )
                        {
                            SaganProcSyslog_LOCAL->proto = 1;
                        }

                }

            if ( json_object_object_get_ex(json_obj, Syslog_JSON_Map->md5, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->md5, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->md5));
                }

            if ( json_object_object_get_ex(json_obj, Syslog_JSON_Map->sha1, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->sha1, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->sha1));
                }

            if ( json_object_object_get_ex(json_obj, Syslog_JSON_Map->sha256, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->sha256, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->sha256));
                }

            if ( json_object_object_get_ex(json_obj, Syslog_JSON_Map->filename, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->filename, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->filename));
                }

            if ( json_object_object_get_ex(json_obj, Syslog_JSON_Map->hostname, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->hostname, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->hostname));
                }

            if ( json_object_object_get_ex(json_obj, Syslog_JSON_Map->url, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->url, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->url));
                }

            if ( json_object_object_get_ex(json_obj, Syslog_JSON_Map->ja3, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->ja3, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->ja3));
                }

            if ( json_object_object_get_ex(json_obj, Syslog_JSON_Map->flow_id, &tmp))
                {
                    SaganProcSyslog_LOCAL->flow_id = atol( json_object_get_string(tmp) );
                }


        }
    else
        {

            /* The raw syslog is the first "nested" level".  Copy that */

            strlcpy(json_str[0], syslog_string, sizeof(json_str[0]));
            json_obj = json_tokener_parse(syslog_string);

            if ( json_obj == NULL )
                {

                    if ( debug->debugmalformed )
                        {
                            Sagan_Log(WARN, "[%s, line %d] Libfastjson failed to decode JSON input. The log line was: \"%s\"", __FILE__, __LINE__, syslog_string);
                        }

                    json_object_put(json_obj);

                    __atomic_add_fetch(&counters->malformed_json_input_count, 1, __ATOMIC_SEQ_CST);
                    return;
                }

            struct json_object_iterator it = json_object_iter_begin(json_obj);
            struct json_object_iterator itEnd = json_object_iter_end(json_obj);


            /* Search through all key/values looking for embedded JSON */


            while (!json_object_iter_equal(&it, &itEnd))
                {

                    const char *key = json_object_iter_peek_name(&it);
                    struct json_object *const val = json_object_iter_peek_value(&it);

                    const char *val_str = json_object_get_string(val);

                    if ( debug->debugjson )
                        {
                            Sagan_Log(DEBUG, "Key: \"%s\", Value: \"%s\"", key, val_str );

                        }


                    /* Is there nested JSON */

                    if ( val_str != NULL && val_str[0] == '{' )
                        {
                            /* Validate it before handing it to the parser to save CPU */

//                   if ( Sagan_strstr(val_str, ":") && Sagan_strstr(val_str, "\"" ) )
//                  {

                            json_obj2 = json_tokener_parse(val_str);


                            if ( json_obj2 != NULL )
                                {

                                    strlcpy(json_str[json_str_count], val_str, sizeof(json_str[json_str_count]));
                                    json_str_count++;

                                    struct json_object_iterator it2 = json_object_iter_begin(json_obj2);
                                    struct json_object_iterator itEnd2 = json_object_iter_end(json_obj2);

                                    /* Look for any second tier/third tier JSON */

                                    while (!json_object_iter_equal(&it2, &itEnd2))
                                        {

                                            const char *key2 = json_object_iter_peek_name(&it2);
                                            struct json_object *const val2 = json_object_iter_peek_value(&it2);

                                            const char *val_str2 = json_object_get_string(val2);

                                            if ( val_str2[0] == '{' )
                                                {

                                                    strlcpy(json_str[json_str_count], val_str2, sizeof(json_str[json_str_count]));
                                                    json_str_count++;

                                                }

                                            json_object_iter_next(&it2);

                                        }

                                } /* json_obj2 != NULL */

                            json_object_put(json_obj2);

                        }


                    json_object_iter_next(&it);

                }

        }

/* This json_object_put works fine with the above (no leak but faults with the 
   below */

json_object_put(json_obj);

    /* Search through the nest to see if we can find out values */


    for ( a = 0; a < json_str_count; a++ )
        {

            struct json_object *json_obj_sub = NULL;
            json_obj_sub = json_tokener_parse(json_str[a]);

            if ( json_obj_sub == NULL )
                {
                    Sagan_Log(WARN, "[%s, line %d] Detected JSON nest but Libfastjson errors. The log line was: \"%s\"", __FILE__, __LINE__, json_str[a]);
                    json_object_put(json_obj_sub);

                    __atomic_add_fetch(&counters->malformed_json_input_count, 1, __ATOMIC_SEQ_CST);
                    return;
                }

            __atomic_add_fetch(&counters->json_input_count, 1, __ATOMIC_SEQ_CST);


            if ( !strcmp(Syslog_JSON_Map->syslog_map_message, "%JSON%" ) )
                {

                    strlcpy(SaganProcSyslog_LOCAL->syslog_message, syslog_string, sizeof(SaganProcSyslog_LOCAL->syslog_message));
                    has_message = true;

                }

            else if ( json_object_object_get_ex(json_obj_sub, Syslog_JSON_Map->syslog_map_message, &tmp))
                {

                    snprintf(SaganProcSyslog_LOCAL->syslog_message, sizeof(SaganProcSyslog_LOCAL->syslog_message)," %s", json_object_get_string(tmp));
                    SaganProcSyslog_LOCAL->syslog_message[ (sizeof(SaganProcSyslog_LOCAL->syslog_message) -1 ) ] = '\0';
                    has_message = true;
                }

            if ( json_object_object_get_ex(json_obj_sub, Syslog_JSON_Map->syslog_map_host, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->syslog_host, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->syslog_host));
                }

            if ( json_object_object_get_ex(json_obj_sub, Syslog_JSON_Map->syslog_map_facility, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->syslog_facility, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->syslog_facility));
                }

            if ( json_object_object_get_ex(json_obj_sub, Syslog_JSON_Map->syslog_map_priority, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->syslog_priority, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->syslog_priority));
                }

            if ( json_object_object_get_ex(json_obj_sub, Syslog_JSON_Map->syslog_map_level, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->syslog_level, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->syslog_level));
                }

            if ( json_object_object_get_ex(json_obj_sub, Syslog_JSON_Map->syslog_map_tag, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->syslog_tag, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->syslog_tag));
                }

            if ( json_object_object_get_ex(json_obj_sub, Syslog_JSON_Map->syslog_map_date, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->syslog_date, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->syslog_date));
                }

            if ( json_object_object_get_ex(json_obj_sub, Syslog_JSON_Map->syslog_map_time, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->syslog_time, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->syslog_time));
                }

            if ( json_object_object_get_ex(json_obj_sub, Syslog_JSON_Map->syslog_map_program, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->syslog_program, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->syslog_program));
                }


            if ( json_object_object_get_ex(json_obj_sub, Syslog_JSON_Map->src_ip, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->src_ip, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->src_ip));
                }

            if ( json_object_object_get_ex(json_obj_sub, Syslog_JSON_Map->dst_ip, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->dst_ip, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->dst_ip));
                }


            if ( json_object_object_get_ex(json_obj_sub, Syslog_JSON_Map->src_port, &tmp))
                {
                    SaganProcSyslog_LOCAL->src_port = atoi( json_object_get_string(tmp) );
                }

            if ( json_object_object_get_ex(json_obj_sub, Syslog_JSON_Map->dst_port, &tmp))
                {
                    SaganProcSyslog_LOCAL->dst_port = atoi( json_object_get_string(tmp) );
                }

            if ( json_object_object_get_ex(json_obj_sub, Syslog_JSON_Map->proto, &tmp))
                {

                    if ( !strcmp( json_object_get_string(tmp) , "tcp" ) || !strcmp( json_object_get_string(tmp), "TCP" ) )
                        {
                            SaganProcSyslog_LOCAL->proto = 6;
                        }

                    else if ( !strcmp( json_object_get_string(tmp), "udp" ) || !strcmp( json_object_get_string(tmp), "UDP" ) )
                        {
                            SaganProcSyslog_LOCAL->proto = 17;
                        }

                    else if ( !strcmp( json_object_get_string(tmp), "icmp" ) || !strcmp( json_object_get_string(tmp), "ICMP" ) )
                        {
                            SaganProcSyslog_LOCAL->proto = 1;
                        }

                }

            if ( json_object_object_get_ex(json_obj_sub, Syslog_JSON_Map->md5, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->md5, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->md5));
                }

            if ( json_object_object_get_ex(json_obj_sub, Syslog_JSON_Map->sha1, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->sha1, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->sha1));
                }

            if ( json_object_object_get_ex(json_obj_sub, Syslog_JSON_Map->sha256, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->sha256, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->sha256));
                }

            if ( json_object_object_get_ex(json_obj_sub, Syslog_JSON_Map->filename, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->filename, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->filename));
                }

            if ( json_object_object_get_ex(json_obj_sub, Syslog_JSON_Map->hostname, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->hostname, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->hostname));
                }

            if ( json_object_object_get_ex(json_obj_sub, Syslog_JSON_Map->url, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->url, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->url));
                }

            if ( json_object_object_get_ex(json_obj_sub, Syslog_JSON_Map->ja3, &tmp))
                {
                    strlcpy(SaganProcSyslog_LOCAL->ja3, json_object_get_string(tmp), sizeof(SaganProcSyslog_LOCAL->ja3));
                }

            if ( json_object_object_get_ex(json_obj_sub, Syslog_JSON_Map->flow_id, &tmp))
                {   
                    SaganProcSyslog_LOCAL->flow_id = atol( json_object_get_string(tmp) );
                }

	json_object_put(json_obj_sub);

        }

    if ( has_message == false )
        {
            Sagan_Log(WARN, "[%s, line %d] Received JSON which has no decoded 'message' value. The log line was: \"%s\"", __FILE__, __LINE__, syslog_string);
        }

	//json_object_put(json_obj);


}

#endif
