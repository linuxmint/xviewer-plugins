 Process this file with autoconf to produce a configure script.

EOG_MAJOR=3.0.0
EOG_PLUGINS_MAJOR=3.0.0

AC_INIT([eog-plugins], [3.14.0], [http://bugzilla.gnome.org/enter_bug.cgi?product=eog&component=plugins])

AC_CONFIG_SRCDIR([plugins])
AC_PREFIX_PROGRAM([eog])

AC_SUBST(EOG_PLUGINS_MAJOR)

AC_PREREQ(2.59)

AM_INIT_AUTOMAKE([1.11 foreign tar-ustar dist-xz no-dist-gzip])
AC_CONFIG_MACRO_DIR([m4])
# Support silencing the build output if supported (automake-1.11+)
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AM_MAINTAINER_MODE
AC_CONFIG_HEADERS([config.h])

LT_PREREQ([2.2.6])
LT_INIT([disable-static])

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# ***********
# Translation 
# ***********

AM_GLIB_GNU_GETTEXT
IT_PROG_INTLTOOL([0.50.1])

GETTEXT_PACKAGE=eog-plugins
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package])

# **********
# GNOME Libs 
# **********

PKG_CHECK_MODULES([EOG], [
	glib-2.0 >= 2.32.0
	gtk+-3.0 >= 3.3.8
	libpeas-1.0 >= 0.7.4
	eog >= 3.11.4
])
AC_SUBST(EOG_LIBS)
AC_SUBST(EOG_CFLAGS)

# *******
# Plugins
# *******

#FIXME
EOG_HAS_PYTHON=1
#/FIXME

AC_MSG_CHECKING([which plugins to build])

ALL_PLUGINS="postr map fit-to-width exif-display send-by-mail postasa hide-titlebar light-theme"
USEFUL_PLUGINS="postr map fit-to-width exif-display send-by-mail postasa hide-titlebar light-theme"
DEFAULT_PLUGINS="postr map fit-to-width exif-display send-by-mail postasa hide-titlebar light-theme"

PYTHON_ALL_PLUGINS="slideshowshuffle pythonconsole fullscreenbg export-to-folder"
PYTHON_USEFUL_PLUGINS="slideshowshuffle pythonconsole fullscreenbg export-to-folder"
PYTHON_DEFAULT_PLUGINS="slideshowshuffle pythonconsole fullscreenbg export-to-folder"

DIST_PLUGINS="$ALL_PLUGINS $PYTHON_ALL_PLUGINS"

if test $EOG_HAS_PYTHON
then
	ALL_PLUGINS="$ALL_PLUGINS $PYTHON_ALL_PLUGINS"
	USEFUL_PLUGINS="$USEFUL_PLUGINS $PYTHON_USEFUL_PLUGINS"
	DEFAULT_PLUGINS="$DEFAULT_PLUGINS $PYTHON_DEFAULT_PLUGINS"
else
	AC_MSG_WARN([eog compiled without python support, plugins $PYTHON_ALL_PLUGINS will not be available])
fi
AC_ARG_WITH([plugins],
	    [AS_HELP_STRING([--with-plugins=LIST],
			    [Comma-separated list specifying the plugins to build.
			    Available: map, exif-display, fit-to-width,
			    fullscreenbg, postr, postasa, pythonconsole,
			    slideshowshuffle, as well as the aliases default,
			    all, and really-all])],
	    [plugins=$with_plugins],
	    [plugins="default"])

if test "x$with_plugins" = xyes
then
	plugins="default"
fi

need_python=no
explicit_plugins=
PLUGINS=
IFS="${IFS=	}"; eogpl_save_ifs="$IFS"; IFS=","
for pl in $plugins
do
	if echo "$ALL_PLUGINS" | egrep "(^| )$pl(\$| )" > /dev/null
	then
		PLUGINS="$PLUGINS $pl"
		explicit_plugins="$explicit_plugins $pl"
	elif test "x$pl" = "xdefault"
	then
		PLUGINS="$PLUGINS $DEFAULT_PLUGINS"
	elif test "x$pl" = "xall"
	then
		PLUGINS="$PLUGINS $USEFUL_PLUGINS"
	elif test "x$pl" = "xreally-all"
	then
		PLUGINS="$PLUGINS $ALL_PLUGINS"
	else
		IFS=$eogpl_save_ifs
		AC_MSG_ERROR([the specified plugin $pl does not exist])
	fi
	
	if echo "$PYTHON_ALL_PLUGINS" | egrep "(^| )$pl(\$| )" > /dev/null
	then
		need_python=yes
	fi
done
IFS="$eogpl_save_ifs"

AC_MSG_RESULT([$PLUGINS])

# *****
# GSettings 
# *****

GLIB_GSETTINGS

# **************
# Python Support 
# **************

AC_MSG_CHECKING([whether Python support is requested])

AC_ARG_ENABLE([python],
	AS_HELP_STRING([--enable-python],[Enable python support]),
	[enable_python=$enableval have_python=$enableval],
	[enable_python=autodetect have_python=yes])

AC_MSG_RESULT([$enable_python])

if test "x$have_python" != "xno"; then
	AM_PATH_PYTHON([3.2],[],[:])
	if test "x$PYTHON" = "x:"; then
		have_python=no
	fi
fi

if test "x$have_python" != "xyes"; then
	if test "x$enable_python" = "xyes"; then
		AC_MSG_ERROR([Python not found])
	elif test "x$enable_python" = "xautodetect"; then
		enable_python=no
		AC_MSG_WARN([Python not found, disabling python support])
	fi
elif test "x$enable_python" != "xno"; then
	enable_python=yes
	AC_DEFINE([ENABLE_PYTHON],[1],[Define to compile with python support])
fi

AM_CONDITIONAL([ENABLE_PYTHON],[test "x$enable_python" = "xyes"])

disabled_plugins=

# *******************
# Plugin Dependencies
# *******************

check_plugin_defined() {
	if echo "$2" | grep -w "$1" > /dev/null;
	then
		return 1
	else
		return 0
	fi
}

plugin_defined() {
	check_plugin_defined "$1" "$PLUGINS"
	return $?
}

plugin_defined_explicit() {
	check_plugin_defined "$1" "$explicit_plugins"
	return $?
}

undef_plugin() {
	if test -z "$disabled_plugins"
	then
		disabled_plugins="$1 ($2)"
	else
		disabled_plugins="$disabled_plugins, $1 ($2)"
	fi
	
	PLUGINS=$(echo "$PLUGINS" | sed -e "s/[[^a-zA-Z0-9_]]$1[[ \t\r\n\f]]\{1,\}/ /" -e "s/[[^a-zA-Z0-9_]]$1$//" -e "s/^$1[[ \t\r\n\f]]\{1,\}//")
}

# Disable python plugins if there is no python support
if test "x$enable_python" = "xno"
then
	for pl in $PYTHON_ALL_PLUGINS
	do
		undef_plugin "$pl" "no python support"
	done
fi

# *****
# Postr 
# *****
plugin_defined postr
if test "$?" = 1
then
	AC_PATH_PROG(POSTR, postr)
	if test -z "$POSTR"
	then
		undef_plugin postr "postr was not found"
	fi
fi

# *******
# Postasa
# *******
plugin_defined postasa
if test "$?" = 1
then
    PKG_CHECK_MODULES(POSTASA,
        [ libgdata >= 0.6.0 ],
	[have_postasa=yes],
	[have_postasa=no])
    AC_SUBST(POSTASA_LIBS)
    AC_SUBST(POSTASA_CFLAGS)
    if test "x$have_postasa" = "xno"
    then
        undef_plugin postasa "libgdata was not found"
    else
         PKG_CHECK_EXISTS([ libgdata >= 0.8 ],
	     AC_DEFINE([HAVE_LIBGDATA_0_8], [1],
	               [Define to be compatible with the API of libgdata-0.8]),
	)
        PKG_CHECK_EXISTS([ libgdata >= 0.9.1 ],
	     AC_DEFINE([HAVE_LIBGDATA_0_9], [1],
	               [Define to be compatible with the API of libgdata-0.9]),
	)


    fi
fi

# ***
# Map
# ***
plugin_defined map
if test "$?" = 1
then
	
	PKG_CHECK_MODULES(CHAMPLAIN,
		[ champlain-0.12 >= 0.9.0,
		  champlain-gtk-0.12 >= 0.9.0,
		  clutter-1.0 >= 1.9.4,
		  clutter-gtk-1.0 >= 1.1.2,
		  libexif >= 0.6.16 ],
		[],[have_libchamplain=no])
	AC_SUBST(CHAMPLAIN_LIBS)
	AC_SUBST(CHAMPLAIN_CFLAGS)
	if test "x$have_libchamplain" = "xno"
	then
		undef_plugin map "libchamplain or libexif was not found"
	fi
fi

# ***********
# ExifDisplay
# ***********
plugin_defined exif-display
if test "$?" = 1
then

    PKG_CHECK_MODULES(EXIFDISPLAY,
        [ libexif >= 0.6.16 ],
        [],[have_exifdisplay=no])
    AC_SUBST(EXIFDISPLAY_LIBS)
    AC_SUBST(EXIFDISPLAY_CFLAGS)
    if test "x$have_exifdisplay" = "xno"
    then
        undef_plugin exif-display "libexif was not found"
    fi
fi

# ***********
# PythonConsole
# ***********
plugin_defined pythonconsole
if test "$?" = 1
then

    PKG_CHECK_EXISTS([ gsettings-desktop-schemas ],
        [],
	undef_plugin pythonconsole "gsettings-desktop-schemas missing"
    )
fi


if test -z "$disabled_plugins"
then
	disabled_plugins="none"
fi

PLUGINS=$(echo $PLUGINS | tr ' ' '\n' | sort | uniq | tr '\n' ' ')
AC_SUBST([DIST_PLUGINS])
AC_SUBST([BUILD_PLUGINS],[$PLUGINS])

# ****
# Misc 
# ****
AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal)
AC_PATH_PROG(GLIB_COMPILE_RESOURCES, glib-compile-resources)

PLUGIN_LIBTOOL_FLAGS="-module -avoid-version"
AC_SUBST(PLUGIN_LIBTOOL_FLAGS)

EOG_PLUGINS_LIBS_DIR="$libdir/eog/plugins"
AC_SUBST(EOG_PLUGINS_LIBS_DIR)

EOG_PLUGINS_DATA_DIR="$datadir/eog/plugins"
AC_SUBST(EOG_PLUGINS_DATA_DIR)

AC_CONFIG_FILES([
Makefile
plugins/Makefile
plugins/map/Makefile
plugins/fit-to-width/Makefile
plugins/exif-display/Makefile
plugins/exif-display/org.gnome.eog.plugins.exif-display.gschema.xml
plugins/export-to-folder/Makefile
plugins/export-to-folder/org.gnome.eog.plugins.export-to-folder.gschema.xml
plugins/fullscreenbg/Makefile
plugins/fullscreenbg/org.gnome.eog.plugins.fullscreenbg.gschema.xml
plugins/hide-titlebar/Makefile
plugins/light-theme/Makefile
plugins/send-by-mail/Makefile
plugins/slideshowshuffle/Makefile
plugins/postr/Makefile
plugins/postasa/Makefile
plugins/pythonconsole/Makefile
plugins/pythonconsole/org.gnome.eog.plugins.pythonconsole.gschema.xml
po/Makefile.in])

AC_OUTPUT

echo "
Configure summary:

	Source code location:..: ${srcdir}
	Compiler...............: ${CC}
	Prefix ................: ${prefix}
	Python Support ........: $enable_python
	Plugins ...............: $PLUGINS
	Disabled plugins ......: $disabled_plugins

Note: you have to install these plugins into the same prefix than your eog
installation (probably /usr if you're using your distro packages, /usr/local
if you have compiled it on your own).
"
