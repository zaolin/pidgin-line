/* purple
 *
 * Purple is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#ifndef _PURPLE_INTERNAL_H_
#define _PURPLE_INTERNAL_H_
/*
 * SECTION:internal
 * @section_id: libpurple-internal
 * @short_description: <filename>internal.h</filename>
 * @title: Internal definitions and includes
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* for SIOCGIFCONF  in SKYOS */
#ifdef SKYOS
#include <net/sockios.h>
#endif
/*
 * If we're using NLS, make sure gettext works.  If not, then define
 * dummy macros in place of the normal gettext macros.
 *
 * Also, the perl XS config.h file sometimes defines _  So we need to
 * make sure _ isn't already defined before trying to define it.
 *
 * The Singular/Plural/Number ngettext dummy definition below was
 * taken from an email to the texinfo mailing list by Manuel Guerrero.
 * Thank you Manuel, and thank you Alex's good friend Google.
 */
#ifdef ENABLE_NLS
#  include <locale.h>
#  ifndef __APPLE_CC__
#    define __APPLE_CC__ 0
#  endif
#  include <libintl.h>
#  undef printf
#  define _(String) ((const char *)dgettext(PACKAGE, String))
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  include <locale.h>
#  define N_(String) (String)
#  ifndef _
#    define _(String) ((const char *)String)
#  endif
#  define ngettext(Singular, Plural, Number) ((Number == 1) ? ((const char *)Singular) : ((const char *)Plural))
#  define dngettext(Domain, Singular, Plural, Number) ((Number == 1) ? ((const char *)Singular) : ((const char *)Plural))
#endif

#ifdef HAVE_ENDIAN_H
# include <endian.h>
#endif

#define MSG_LEN 2048
/* The above should normally be the same as BUF_LEN,
 * but just so we're explicitly asking for the max message
 * length. */
#define BUF_LEN MSG_LEN
#define BUF_LONG BUF_LEN * 2

#include <sys/types.h>
#ifndef _WIN32
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/time.h>
#endif
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef HAVE_ICONV
#include <iconv.h>
#endif

#ifdef HAVE_LANGINFO_CODESET
#include <langinfo.h>
#endif

#include <gmodule.h>

#ifdef PURPLE_PLUGINS
# ifdef HAVE_DLFCN_H
#  include <dlfcn.h>
# endif
#endif

#ifndef _WIN32
# include <netinet/in.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <sys/un.h>
# include <sys/utsname.h>
# include <netdb.h>
# include <signal.h>
# include <unistd.h>
#endif

#ifndef HOST_NAME_MAX
# define HOST_NAME_MAX 255
#endif

#include <glib.h>
#include <glib/gstdio.h>

#ifdef _WIN32
#include "win32/win32dep.h"
#endif

#ifdef HAVE_CONFIG_H
#if SIZEOF_TIME_T == 4
#	define PURPLE_TIME_T_MODIFIER "lu"
#elif SIZEOF_TIME_T == 8
#	define PURPLE_TIME_T_MODIFIER "zu"
#else
#error Unknown size of time_t
#endif
#endif

#define PURPLE_STATIC_ASSERT(condition, message) \
	{ typedef char static_assertion_failed_ ## message \
	[(condition) ? 1 : -1]; static_assertion_failed_ ## message dummy; \
	(void)dummy; }

/* This is meant to track use-after-free errors.
 * TODO: it should be disabled in released code. */
#define PURPLE_ASSERT_CONNECTION_IS_VALID(gc) \
	_purple_assert_connection_is_valid(gc, __FILE__, __LINE__)

#ifdef __clang__

#define PURPLE_BEGIN_IGNORE_CAST_ALIGN \
	_Pragma ("clang diagnostic push") \
	_Pragma ("clang diagnostic ignored \"-Wcast-align\"")

#define PURPLE_END_IGNORE_CAST_ALIGN \
	_Pragma ("clang diagnostic pop")

#else

#define PURPLE_BEGIN_IGNORE_CAST_ALIGN
#define PURPLE_END_IGNORE_CAST_ALIGN

#endif /* __clang__ */

#include <glib-object.h>

#ifdef __COVERITY__

/* avoid TAINTED_SCALAR warning */
#undef g_utf8_next_char
#define g_utf8_next_char(p) (char *)((p) + 1)

#endif

typedef union
{
	struct sockaddr sa;
	struct sockaddr_in in;
	struct sockaddr_in6 in6;
	struct sockaddr_storage storage;
} common_sockaddr_t;

#define PURPLE_WEBSITE "https://pidgin.im/"
#define PURPLE_DEVEL_WEBSITE "https://developer.pidgin.im/"

/* BACKPORTS: deleted internal function prototypes */

/**
 * _purple_socket_init: (skip)
 *
 * Initializes the #PurpleSocket subsystem.
 */
void
_purple_socket_init(void);

/**
 * _purple_socket_uninit: (skip)
 *
 * Uninitializes the #PurpleSocket subsystem.
 */
void
_purple_socket_uninit(void);

#endif /* _PURPLE_INTERNAL_H_ */
