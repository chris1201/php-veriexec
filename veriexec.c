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
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_veriexec.h"

ZEND_DECLARE_MODULE_GLOBALS(veriexec)


/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
        ZEND_INI_ENTRY("zend.veriexec_file",                  "sigs.dat",           ZEND_INI_SYSTEM,           NULL)
        ZEND_INI_ENTRY("zend.veriexec_mode",                  "2",           ZEND_INI_SYSTEM,           NULL)
PHP_INI_END()
/* }}} */

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_veriexec_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_veriexec_compiled)
{
	char *arg = NULL;
	int arg_len, len;
	char *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	len = spprintf(&strg, 0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "veriexec", arg);
	RETURN_STRINGL(strg, len, 0);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/


/* {{{ php_veriexec_init_globals
 */
static void php_veriexec_init_globals(zend_veriexec_globals *veriexec_globals)
{
	veriexec_globals->zend_veriexec_file = NULL;
	veriexec_globals->zend_veriexec_mode = 2;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(veriexec)
{
	TSRMLS_FETCH();

	REGISTER_INI_ENTRIES();

 	zend_hash_init(&VERIEXEC_G(zend_veriexec_table), 32, NULL, NULL, 0);

zend_printf("%s\n", VERIEXEC_G(zend_veriexec_file));

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(veriexec)
{
	UNREGISTER_INI_ENTRIES();
	
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(veriexec)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "Verified Code support", "enabled");
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ veriexec_functions[]
 *
 * Every user visible function must have an entry in veriexec_functions[].
 */
const zend_function_entry veriexec_functions[] = {
	PHP_FE(confirm_veriexec_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE_END	/* Must be the last line in veriexec_functions[] */
};
/* }}} */

/* {{{ veriexec_module_entry
 */
zend_module_entry veriexec_module_entry = {
	STANDARD_MODULE_HEADER,
	"veriexec",
	veriexec_functions,
	PHP_MINIT(veriexec),
	PHP_MSHUTDOWN(veriexec),
	NULL,
	NULL,
	PHP_MINFO(veriexec),
	PHP_VERIEXEC_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_VERIEXEC
ZEND_GET_MODULE(veriexec)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
