# NTP
synchronize the time of a computer by NTP

# Usage
Linux env
```
make
./ntp &  //Run in the background,then create default config and log file
```

# Config
the path of ntpconfig.ini file defined in lib_ntp.h
```
ntp_enable = 1
ntp_server = time.nist.gov
ntp_port = 123
ntp_timeout = 18000
```

# Log file
the path of ntp_log file defined in lib_ntp.h
```
[1970-01-06 10:26:53]:start ntp ok
[1970-01-06 10:26:53]:get_ntp_ip 24.56.178.140
[2016-01-25 18:26:48]:set time:1453717608
```