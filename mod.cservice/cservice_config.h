/* cservice_config.h */

#ifndef __CSERVICE_CONFIG_H
#define __CSERVICE_CONFIG_H "$Id: cservice_config.h,v 1.3 2002-01-16 23:24:16 morpheus Exp $"

/**
 * Define this if you wish for all SQL queries to be sent
 * the standard logging stream (elog).
 */
#define LOG_SQL

/**
 * Define this if you wish to log all cache hits.
 */
#define LOG_CACHE_HITS

/**
 * Define this if you wish for general debugging information
 * be logged.
 */
#define LOG_DEBUG

/**
 * The maximum number of results to return to the user on
 * an LBANLIST query.
 */
#define MAX_LBAN_RESULTS 10

/**
 * The maximum number of results to return to the user on
 * an ACCESS query.
 */
#define MAX_ACCESS_RESULTS 15

/**
 * The maximum number of search results to return to the user.
 */
#define MAX_SEARCH_RESULTS 10

/**
 * FEATURE_INVITE
 * Enable a feature that invites the user into channels he's not in if he logins and has the INVITE flag set
 */
#define FEATURE_INVITE

#define FEATURE_NOFORCE

#define FEATURE_MEMOSERV

#endif // __CSERVICE_CONFIG_H
