dnl $Id$
dnl config.m4 for extension veriexec

PHP_ARG_ENABLE(veriexec, whether to enable Verified Code support,
[  --enable-veriexec           Enable veriexec support])

if test "$PHP_VERIEXEC" != "no"; then

  PHP_NEW_EXTENSION(veriexec, veriexec.c, $ext_shared)
fi
