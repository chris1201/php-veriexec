/*
  +----------------------------------------------------------------------+
  | Verified Code Execution                                              |
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
  | Author: jmfield2                                                     |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "php_veriexec.h"

#define php_hash_int32  int32_t
#define php_hash_uint32 uint32_t
#define php_hash_int64  int64_t
#define php_hash_uint64 uint64_t

#include <stdint.h>

// Requires HASH extension 

/* SHA256 context. */
typedef struct {
        php_hash_uint32 state[8];               /* state */
        php_hash_uint32 count[2];               /* number of bits, modulo 2^64 */
        unsigned char buffer[64];       /* input buffer */
} PHP_SHA256_CTX;

extern void PHP_SHA256Init(PHP_SHA256_CTX *);
extern void PHP_SHA256Update(PHP_SHA256_CTX *, const unsigned char *, unsigned int);
extern void PHP_SHA256Final(unsigned char[32], PHP_SHA256_CTX *);

static inline void php_hash_bin2hex(char *out, const unsigned char *in, int in_len)
{
        static const char hexits[17] = "0123456789abcdef";
        int i;

        for(i = 0; i < in_len; i++) {
                out[i * 2]       = hexits[in[i] >> 4];
                out[(i * 2) + 1] = hexits[in[i] &  0x0F];
        }
}

#include "zend.h"
#include "zend_extensions.h"
#include "zend_compile.h"
#include "zend_list.h"
#include "zend_execute.h"
#include "zend_API.h"
#include "zend_ini.h"

ZEND_DECLARE_MODULE_GLOBALS(veriexec)

/* execution redirection functions */
zend_op_array* (*old_compile_file)(zend_file_handle* file_handle, int type TSRMLS_DC);
zend_op_array* veriexec_compile_file(zend_file_handle*, int TSRMLS_DC);

zend_op_array* (*old_compile_str)(zval *source_string, char *filename TSRMLS_DC);
zend_op_array* veriexec_compile_string(zval *source_string, char *filename TSRMLS_DC);

/* Main verification */
ZEND_API int zend_veriexec_verify(char *source_buf, uint32_t source_len);


/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
        ZEND_INI_ENTRY("zend.veriexec_file",                  "sigs.dat",           PHP_INI_SYSTEM,           NULL)
        ZEND_INI_ENTRY("zend.veriexec_mode",                  "2",           PHP_INI_SYSTEM,           NULL)
PHP_INI_END()
/* }}} */

/* {{{ php_veriexec_init_globals
 */
static void php_veriexec_init_globals(zend_veriexec_globals *veriexec_globals)
{
	veriexec_globals->zend_veriexec_file = NULL;
	veriexec_globals->zend_veriexec_mode = 2;
}
/* }}} */

/*
Find digital signature of source_buf and verify it exists in preloaded hash table

@return 0 if the hash did NOT exist and thus the verification failed
otherwise returns 1
*/
ZEND_API int zend_veriexec_verify(char *source_buf, uint32_t source_len) {
	char digest[32], hex[64];

/* XXX This should use more robust signatures that offer:
Authentication, Integrity, and possibly non-repudiation
instead of simply the integrity that a SHA256sum offers

OpenSSL priv/pubkey of digital signature
Provide 3 guarantees
Integrity .. Msg not altered
Authenticity ... Data signed by entity with priv key--used to sign
Non repudiation ... Entity can't claim they did not sign

A signs hash of data by encrypting w priv key
B checks signature by decrypting signed data using As pub key
..gets hash of data
*/

	PHP_SHA256_CTX ctx;
	HashTable *ht;

	PHP_SHA256Init(&ctx);
	PHP_SHA256Update(&ctx, source_buf, source_len);
	PHP_SHA256Final(digest, &ctx);
	php_hash_bin2hex(hex, digest, 32);

	ht = &VERIEXEC_G(zend_veriexec_table);

	if (!zend_hash_exists(ht, hex, strlen(hex))) {

		if (VERIEXEC_G(zend_veriexec_mode) == 3) {
			// Learning Mode
			zend_error(E_WARNING, "Code signature not found - adding to database.");
			// open _sigexec_file and add these sig

	        	FILE *_sigexec_fp = fopen(VERIEXEC_G(zend_veriexec_file), "a+");
			fprintf(_sigexec_fp, "%s\n", hex);
			fclose(_sigexec_fp);

			// add to runtime hash
			zend_hash_add(ht, hex, strlen(hex), "1", 1, NULL);

			return 1;
		}
		else if (VERIEXEC_G(zend_veriexec_mode) == 2) {
			// Warn only
			zend_error(E_WARNING, "Code signature not found in database - Continuing anyway.");
			return 1;
		}
		else if (VERIEXEC_G(zend_veriexec_mode) == 1) {
			// Warn and refuse to execute

		        zend_error(E_WARNING, "Code signature not found in database - Refusing to execute.");
			return 0;
		}
		else { /* XXX default deny mode */
	        	// Strict enforcement
        	        zend_error(E_ERROR, "Code signature enforcement level requires script execution to halt.");
                	zend_bailout();
	        }

		return 0;
	}

	return 1;
}


/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
void veriexec_shutdown(zend_extension *extension)
{
	zend_compile_file = old_compile_file;
	zend_compile_string = old_compile_str;
	
	return;
}
/* }}} */

PHP_MINIT_FUNCTION(veriexec) 
{
	FILE *_sigexec_fp;
	char _sigexec_buf[1024];
	HashTable *ht;

	TSRMLS_FETCH();

	REGISTER_INI_ENTRIES();

	ht = &VERIEXEC_G(zend_veriexec_table);
	VERIEXEC_G(zend_veriexec_file) = INI_STR("zend.veriexec_file");
	VERIEXEC_G(zend_veriexec_mode) = INI_INT("zend.veriexec_mode");

   	zend_hash_init(ht, 32, NULL, NULL, 0);
        _sigexec_fp = fopen(VERIEXEC_G(zend_veriexec_file), "r+");
	if (_sigexec_fp != NULL) {
	        while (!feof(_sigexec_fp)) {
        	        fgets(_sigexec_buf, sizeof(_sigexec_buf) - 1, _sigexec_fp);

                	if (_sigexec_buf[strlen(_sigexec_buf)-1] == 10)
                        	_sigexec_buf[strlen(_sigexec_buf)-1] = 0; // chomp newline

	                zend_hash_add(ht, _sigexec_buf, strlen(_sigexec_buf), "1", 1, NULL);
        	}
	        fclose(_sigexec_fp);
	}

	// XXX log
        zend_printf("%d Digital Signatures Loaded\n", ht->nNumOfElements);
}

PHP_MSHUTDOWN_FUNCTION(veriexec)
{
	UNREGISTER_INI_ENTRIES();
}

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
	PHP_FE_END	/* Must be the last line in veriexec_functions[] */
};
/* }}} */


ZEND_API zend_op_array *veriexec_compile_file(zend_file_handle *file_handle, int type TSRMLS_DC) {


        if (open_file_for_scanning(file_handle TSRMLS_CC)==FAILURE) {
                if (type==ZEND_REQUIRE) {
                        zend_message_dispatcher(ZMSG_FAILED_REQUIRE_FOPEN, file_handle->filename TSRMLS_CC);
                        zend_bailout();
                } else {
                        zend_message_dispatcher(ZMSG_FAILED_INCLUDE_FOPEN, file_handle->filename TSRMLS_CC);
                }

		return NULL;
	}

        if (!zend_veriexec_verify(file_handle->handle.stream.mmap.buf, strlen(file_handle->handle.stream.mmap.buf))) {
                // Allow script to continue
	        return NULL;
        }

	return old_compile_file(file_handle, type);
}

zend_op_array *veriexec_compile_string(zval *source_string, char *filename TSRMLS_DC) {

	if (!zend_veriexec_verify(Z_STRVAL_P(source_string), Z_STRLEN_P(source_string))) {
               // Allow script to continue execution
               return NULL;
        }


	return old_compile_str(source_string, filename);
}

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
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES
};

/* {{{ PHP_MINIT_FUNCTION
 */
static int veriexec_startup(zend_extension *extension)
{

	TSRMLS_FETCH();

	ZEND_INIT_MODULE_GLOBALS(veriexec, php_veriexec_init_globals, NULL);

	old_compile_file = zend_compile_file;
	old_compile_str = zend_compile_string;

	zend_compile_file = veriexec_compile_file;
	zend_compile_string = veriexec_compile_string;

	return zend_startup_module(&veriexec_module_entry);

}
/* }}} */


ZEND_EXTENSION();

ZEND_EXT_API zend_extension zend_extension_entry = {
	"veriexec",
	PHP_VERIEXEC_VERSION,
	"jmfield2",
	"http://github.com/jmfield2/php-veriexec",
	"Copyright 2015",
	veriexec_startup,
	veriexec_shutdown,
	NULL,           /* activate_func_t */
	NULL,           /* deactivate_func_t */
	NULL,           /* message_handler_func_t */
	NULL,           /* op_array_handler_func_t */
	NULL, /* statement_handler_func_t */
	NULL,           /* fcall_begin_handler_func_t */
	NULL,           /* fcall_end_handler_func_t */
	NULL,   /* op_array_ctor_func_t */
	NULL,           /* op_array_dtor_func_t */
	STANDARD_ZEND_EXTENSION_PROPERTIES
};
// PHP_MINFO XX

/* }}} */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
