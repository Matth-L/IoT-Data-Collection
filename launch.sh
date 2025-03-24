#!/bin/bash

echo "Checking if monitoring already exists"
if ! (docker network ls | grep -q "monitoring");then
	echo "Creating ..."
	docker network create monitoring
fi
echo "Monitoring network : on "

echo "Checking if volumes grafana already exists"
if ! (docker volume ls| grep -q "grafana-volume");then
	echo "Creating ..."
	docker volume create grafana-volume
fi

echo "Checking if volumes influxdb already exists"
if ! (docker volume ls| grep -q "influxdb-volume");then
	echo "Creating ..."
	docker volume create influxdb-volume
fi

echo "Volumes created"


docker run --rm \
  -e INFLUXDB_DB=telegraf -e INFLUXDB_ADMIN_ENABLED=true \
  -e INFLUXDB_ADMIN_USER=admin \
  -e INFLUXDB_ADMIN_PASSWORD=supersecretpassword \
  -e INFLUXDB_HTTP_AUTH_ENABLED=true \
  -e INFLUXDB_USER=telegraf -e INFLUXDB_USER_PASSWORD=secretpassword \
  -v influxdb-volume:/var/lib/influxdb influxdb /init-influxdb.sh

docker compose up -d
