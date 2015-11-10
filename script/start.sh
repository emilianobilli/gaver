#
# Load the module to parse the config file 
#
source config-parser.sh;
config_parser "gvconfig.ini";

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
config.section.Api;
cfg_listen=$listen_api;

echo $bind

if [ -x $bin ]
then 
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
else
    echo "Unable to find $bin"
fi

echo $args
start-stop-daemon --start --exec "$bin $arg"
