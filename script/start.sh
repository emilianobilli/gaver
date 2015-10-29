#
# Load the module to parse the config file 
#
source config-parser.sh;
config_parser "gvconfig.ini";

config.section.GaVer;
cfg_addr=$addr;
cfg_port=$port;
cfg_mtu=$mtu;
config.section.Api;
cfg_listen=$listen;

args="";

if [ "($cfg_addr)" != "" ]
then
    args=$args"-a $cfg_addr "
fi
if [ "($cfg_port)" != "" ]
then
    args=$args"-p $cfg_port "
fi

echo $args

../test_config -a $cfg_addr -p $cfg_port -m $cfg_mtu -l $cfg_listen
