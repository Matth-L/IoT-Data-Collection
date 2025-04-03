#!/bin/bash

# Start InfluxDB in the background
echo "Starting InfluxDB in the background..."
influxd &
INFLUXD_PID=$!

# Create shared config directory
mkdir -p /shared-config

# Wait for InfluxDB to be ready
echo "Waiting for InfluxDB to start..."
for i in {1..60}; do  # Increased timeout to 5 minutes
  if influx ping &>/dev/null; then
    echo "Testing if InfluxDB is fully operational..."
    sleep 10  # Give it a bit more time to be fully ready
    echo "InfluxDB is up!"
    break
  fi
  echo "Waiting for InfluxDB... ($i/60)"
  sleep 5
done

# Run initial setup
echo "Setting up InfluxDB..."
influx setup \
  --username admin \
  --password uqacuqac \
  --token admin_token \
  --org iot-uqac \
  --bucket iot \
  --force

# Create all-access token and extract it
echo "Creating all-access token..."
influx auth create --org iot-uqac --description "All-access-token" --all-access > /shared-config/token_output.txt
export TOKEN=$(awk 'NR==2 {print $3}' /shared-config/token_output.txt)


# Create telegraf config and extract the ID
echo "Creating Telegraf configuration..."
influx telegrafs create --org iot-uqac --name "my-telegraf-config" --description "Imported Telegraf configuration" --file /etc/telegraf/telegraf.conf > /shared-config/telegraf-config.txt
export CONFIGID=$(awk 'NR==2 {print $1}' /shared-config/telegraf-config.txt)


# Save variables to a file that can be sourced by the telegraf container
echo "Saving configuration variables for Telegraf..."
cat > /shared-config/influx_env.sh << EOF
export INFLUX_TOKEN="$TOKEN"
export TELEGRAF_CONFIG_ID="$CONFIGID"
export INFLUX_ORG="iot-uqac"
telegraf --config http://influxdb:8086/api/v2/telegrafs/$CONFIGID
EOF

echo "Telegraf configuration variables saved to /shared-config/influx_env.sh"
cat /shared-config/influx_env.sh

# Create a file to indicate setup is complete (used by healthcheck)
echo "Configuration complete!"
touch /shared-config/config_ready

# Wait for the InfluxDB process to finish
wait $INFLUXD_PID 