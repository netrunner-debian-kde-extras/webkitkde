#
# Regular cron jobs for the webkitkde package
#
0 4	* * *	root	[ -x /usr/bin/webkitkde_maintenance ] && /usr/bin/webkitkde_maintenance
