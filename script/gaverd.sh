#!/bin/bash

### BEGIN INIT INFO
# Provides:          gvd
# Required-Start:    $network
# Required-Stop:     $network 
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: start GaVer daemon (gvd)
### END INIT INFO

#
# Load the module to parse the config file 
#
source /opt/gaver/bin/config-parser.sh;

CONFIG_FILE="/opt/gaver/etc/gvd.ini"

config_parser $CONFIG_FILE;
# Parse Config file
config.section.Bin;
bin=$gvd;
pid=$pid;
config.section.GaVer;
cfg_addr=$addr;
cfg_port=$port;
cfg_mtu=$mtu;
cfg_os=$overal_bps;
cfg_ss=$socket_bps;
cfg_rmem=$rmem;
cfg_wmem=$wmem;
cfg_netstat=$netstat;
cfg_error=$error;
config.section.Api;
cfg_listen=$listen_api;

# Ser if the daemon are there
test -x $bin || exit 0

function make_arg()
{
    args="";
    if [ "($cfg_addr)" != "" ]
    then
	args=$args"-a $cfg_addr "
    fi
    if [ "($cfg_port)" != "" ]
    then
	args=$args"-p $cfg_port "
    fi
    if [ "($cfg_mtu)" != "" ]
    then
	args=$args"-m $cfg_mtu "
    fi
    if [ "($cfg_os)" != "" ]
    then
	args=$args"-s $cfg_os "
    fi
    if [ "($cfg_ss)" != "" ]
    then
	args=$args"-S $cfg_ss "
    fi
    if [ "($cfg_rmem)" != "" ]
    then
	args=$args"-r $cfg_rmem "
    fi
    if [ "($cfg_wmem)" != "" ]
    then
	args=$args"-w $cfg_wmem "
    fi
    if [ "($cfg_listen)" != "" ]
    then
	args=$args"-l $cfg_listen "
    fi
    if [ "($pid)" != "" ]
    then
	args=$args"-P $pid "
    fi
    if [ "($cfg_netstat)" != "" ]
    then
	args=$args"-n $cfg_netstat "
    fi
    if [ "($cfg_error)" != "" ]
    then
	args=$args"-e $cfg_error "
    fi
    echo $args
}
ARG=`make_arg`

if [ $# -eq 2 ]
then
    if [ $2 == "debug" ]
    then
	echo $ARG
        exit 0
    fi
fi

case $1 in
    start)
	echo "Starting GaVer"
	start-stop-daemon --start --exec $bin --pidfile $pid --umask 022 -- $ARG
	;;
    stop)
	echo "Stoping GaVer"
	start-stop-daemon --stop --pidfile $pid
	if [ $? == "0" ]
	then
	    rm -f $pid
	fi
	;;
    *)
	echo "Usage: /etc/init.d/gaverd {start|stop}"
	exit 1
	;;
esac
