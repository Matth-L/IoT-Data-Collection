# IoT Data Collection Project 🌐📈

This project aims to develop a custom IoT solution to replace the Cayenne myDevice web application used in laboratory settings. The goal is to create a reliable and adaptable interface for monitoring data from a LoRaWAN node, with enhanced alert capabilities suitable for industrial deployment. The project involves setting up a network to collect, store, and visualize sensor data, and generate alerts based on predefined thresholds.

### Key Features and Implementation 🚀

- **Hardware**: MKR1310 board equipped with sensors for:
  - Temperature 
  - Humidity 
  - Motion 

- **Data Flow**:
  - Sensor data is transmitted to The Things Stack.
  - Telegraf retrieves the data and forwards it to InfluxDB for storage.

- **Visualization**:
  - Grafana is used to create customizable dashboards for data visualization.

- **Alert Mechanisms**:
  - Email notifications 📧
  - Discord messages (Discord bot API) 💬
  - SMS alerts (Twilio API integration) 📱


### How to set it up and configure it 🚀

We focused on automating the setup process to ensure minimal configuration is required. To get started, simply run:

```sh
docker compose up
```

Access Grafana using the following address: [http://localhost:3000](http://localhost:3000). For login, use the dummy credentials provided:
- **Login**: `grafadmin`
- **Password**: `grafpw`

![dashboard](./img/dashboard.png)

### Environment Configuration ⚙️

Ensure you have a `.env` file available with the following information:

```env
MQTT_PASSWORD=

# Note: We used Mailjet, so this section may vary based on your provider
SMTP_HOST=
SMTP_PORT=
SMTP_USER=
SMTP_USER_ID=
SMTP_USER_PW=

DISCORD_TOKEN=
DISCORD_CHANNEL_ID=

TTN_TENANT=
TTN_APPLICATION_ID=
TTN_DEVICE_ID=

TWILIO_SID=
TWILIO_TOKEN=
# NB : Numbers in international format with (+) sign and country code (eg : +18004444444)
TWILIO_ORIGIN_NUMBER=
TWILIO_DESTINATION_NUMBER=
```
