/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2015 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Joseph Fields (jmfield2@mail.usf.edu)                        |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_VERIEXEC_H
#define PHP_VERIEXEC_H

#define PHP_VERIEXEC_VERSION "0.1.0" 

#ifndef ZEND_EXT_API
# if WIN32|WINNT
#  define ZEND_EXT_API __declspec(dllexport)
# elif defined(__GNUC__) && __GNUC__ >= 4
#  define ZEND_EXT_API __attribute__ ((visibility("default")))
# else
#  define ZEND_EXT_API
# endif
#endif

#ifdef PHP_WIN32
#	define PHP_VERIEXEC_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_VERIEXEC_API __attribute__ ((visibility("default")))
#else
#	define PHP_VERIEXEC_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

ZEND_BEGIN_MODULE_GLOBALS(veriexec)
	HashTable zend_veriexec_table;
	char	  *zend_veriexec_file;
	long      zend_veriexec_mode;
ZEND_END_MODULE_GLOBALS(veriexec)

/* In every utility function you add that needs to use variables 
   in php_veriexec_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as VERIEXEC_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define VERIEXEC_G(v) TSRMG(veriexec_globals_id, zend_veriexec_globals *, v)
#else
#define VERIEXEC_G(v) (veriexec_globals.v)
#endif

#endif	/* PHP_VERIEXEC_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
