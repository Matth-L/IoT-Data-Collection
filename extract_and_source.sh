#!/bin/bash 

eval $(grep -E '^export ' /shared-config/influx_env.sh)

exec grafana-server --config /etc/grafana/grafana.ini