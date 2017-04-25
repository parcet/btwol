#! /bin/sh
### BEGIN INIT INFO
# Provides:          btwol
# Required-Start:    $local_fs $syslog $remote_fs bluetooth
# Required-Stop:     $local_fs $syslog $remote_fs
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Start btwol daemon
### END INIT INFO

PATH=/sbin:/bin:/usr/sbin:/usr/bin
DESC=btwol
DAEMON=/usr/sbin/btwold
NAME=btwol.sh
SCRIPTNAME=/etc/init.d/$NAME

test -f $DAEMON || exit 0

# FIXME: any of the sourced files may fail if/with syntax errors
test -f /etc/default/btwol && . /etc/default/btwol
test -f /etc/default/rcS && . /etc/default/rcS

. /lib/lsb/init-functions


SSD_OPTIONS="--oknodo --quiet --exec $DAEMON -- $BTWOL_OPTIONS"


set -e

case $1 in
  start)
	log_daemon_msg "Starting $DESC"

	if test "$BTWOL_ENABLED" = 0; then
		log_progress_msg "disabled. see /etc/default/btwol"
		log_end_msg 0
		exit 0
	fi

	start-stop-daemon --start --background $SSD_OPTIONS
  ;;
  stop)
	log_daemon_msg "Stopping $DESC"
	if test "$BTWOL_ENABLED" = 0; then
		log_progress_msg "disabled."
		log_end_msg 0
		exit 0
	fi

	start-stop-daemon --stop $SSD_OPTIONS
  ;;
  restart|force-reload)
	$0 stop
	sleep 1
	$0 start
  ;;
  status)
	status_of_proc "$DAEMON" "$DESC" && exit 0 || exit $?
  ;;
  *)
	echo "Usage: $NAME {start|stop|restart|force-reload|status}" >&2
	exit 1
	;;
esac

exit 0

# vim:noet
