source config-parser.sh;

config_parser "example.ini";
config.section.GaVer;
speed=$overal_speed;
config.section.Api;
listen=$listen;
echo "$speed";
echo "$listen";

if [ "$speed" != "" ]; then
    echo "Hola";
fi