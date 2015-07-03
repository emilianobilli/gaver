import ConfigParser
import io


config_str = open("../etc/gaver.cfg", 'r').read()
config = ConfigParser.RawConfigParser(allow_no_value=True)
config.readfp(io.BytesIO(config_str))

print config.get("GaVer", "pid_file")
