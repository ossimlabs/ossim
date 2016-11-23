#---
# Open Source Software Image Map(OSSIM) csh initialization script (csh).
# This file is installed by ossim rpm in: /etc/profile.d/ossim.csh
#---
if ( ! $?OSSIM_INSTALL_PREFIX ) then
   setenv OSSIM_INSTALL_PREFIX "/usr"
endif

if ( ( ! $?OSSIM_PREFS_FILE ) then
   setenv OSSIM_PREFS_FILE $OSSIM_INSTALL_PREFIX/share/ossim/ossim-site-preferences
endif

#---
# Stub to set path for non-standard install:
#---
# set ossim_bin=/usr/local/ossim/bin
# if ( "${path}" !~ *$ossim_bin* ) then
#   set path = ( $ossim_bin $path )
# endif
