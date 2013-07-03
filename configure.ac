AC_INIT([hooky], [1.0])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AC_CHECK_LIB([event], [event_base_new])
AC_CHECK_LIB([event_openssl], [bufferevent_get_openssl_error])
AC_CHECK_LIB([crypto], [ERR_lib_error_string])
AC_CHECK_LIB([json-c], [json_tokener_parse])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT

