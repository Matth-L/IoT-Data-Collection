import base64
import json
import logging
import os
from datetime import datetime
import queue
import asyncio

import discord
import requests
from discord import ButtonStyle
from discord.ui import Button, View
from flask import Flask, request, jsonify

from twilio.rest import Client as TwilioClient

# Configuration TTN
TTN_CONFIG = {
    "tenant": os.environ.get("TTN_TENANT"),
    "application_id": os.environ.get("TTN_APPLICATION_ID"),
    "device_id": os.environ.get("TTN_DEVICE_ID"),
    "api_key": os.environ.get("MQTT_PASSWORD")
}

# Configuration Discord
DISCORD_TOKEN = os.environ.get("DISCORD_TOKEN")
DISCORD_CHANNEL_ID = int(os.environ.get("DISCORD_CHANNEL_ID"))  # ID du salon Discord

# Configuration Twilio
TWILIO_ACCOUNT_SID = os.environ.get("TWILIO_SID")
TWILIO_TOKEN = os.environ.get("TWILIO_TOKEN")
TWILIO_ORIGIN_NUMBER = os.environ.get("TWILIO_ORIGIN_NUMBER")
TWILIO_DESTINATION_NUMBER = os.environ.get("TWILIO_DESTINATION_NUMBER")

# Boutons de rétroaction
COMMANDS = {
    "✅ OK": {"payload": [0x00, 0x00], "description": "✅ OK"},
    "⚠️ Warning": {"payload": [0x00, 0x01], "description": "⚠️ Warning"},
    "🚨 Alert": {"payload": [0x00, 0x02], "description": "🚨 Alert"}
}

# Logs terminal
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

# Init Flask
app = Flask(__name__)

# Message queue pour traiter les alertes
message_queue = queue.Queue()

# Twilio client
TWILIO_SMS_CLIENT = TwilioClient(TWILIO_ACCOUNT_SID, TWILIO_TOKEN)

# Envoi d'un SMS via Twilio
def send_sms_twilio(message):
    try:
        TWILIO_SMS_CLIENT.messages.create(
            body=message,
            from_=TWILIO_ORIGIN_NUMBER,
            to=TWILIO_DESTINATION_NUMBER
        )
        logger.info("SMS sent successfully")
    except Exception as e:
        logger.error(f"Failed to send SMS: {str(e)}")

####################
# Discord bot setup
####################
intents = discord.Intents.default()
intents.message_content = True
intents.guilds = True
bot = discord.Client(intents=intents)

# Gestion des boutons sur Discord
class TTNCommandView(View):
    def __init__(self, alert_data):
        super().__init__(timeout=600) # timeout pour créer une rétroaction
        self.alert_data = alert_data
        
        for cmd_id, cmd_info in COMMANDS.items():
            button = Button(
                label=cmd_id.replace("_", " ").title(),
                style=ButtonStyle.primary, 
                custom_id=cmd_id
            )
            button.callback = self.button_callback
            self.add_item(button)

    async def button_callback(self, interaction):
        command = interaction.data["custom_id"]
        
        if command in COMMANDS:
            await interaction.response.defer(ephemeral=True)
            command_details = COMMANDS[command]
            success, response = send_ttn_downlink(command_details["payload"]) # Envoi du payload à TTN
            
            if success:
                await interaction.followup.send(
                    f"✅ Command '{command}' sent successfully to device!", 
                    ephemeral=True
                )
                embed = interaction.message.embeds[0]
                embed.add_field(
                    name="Action Taken", 
                    value=f"{interaction.user.mention} triggered: {command}", 
                    inline=False
                )
                embed.set_footer(text=f"Last updated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
                await interaction.message.edit(embed=embed, view=None)
            else:
                await interaction.followup.send(
                    f"❌ Failed to send command to device: {response}", 
                    ephemeral=True
                )

# Envoi d'un downlink à TTN
def send_ttn_downlink(payload_bytes):
    try:
        # Lien API TTN
        url = f"https://{TTN_CONFIG['tenant']}.cloud.thethings.network/api/v3/as/applications/{TTN_CONFIG['application_id']}/devices/{TTN_CONFIG['device_id']}/down/push"
        headers = {
            "Authorization": f"Bearer {TTN_CONFIG['api_key']}", # Authentification
            "Content-Type": "application/json"
        }
        frm_payload = base64.b64encode(bytes(payload_bytes)).decode('utf-8')
        data = {
            "downlinks": [
                {
                    "frm_payload": frm_payload,
                    "f_port": 3,
                    "priority": "NORMAL"
                }
            ]
        }
        response = requests.post(url, headers=headers, data=json.dumps(data))
        if response.status_code >= 200 and response.status_code < 300:
            logger.info(f"Downlink successfully sent: {response.text}")
            return True, response.text
        else:
            logger.error(f"Failed to send downlink: {response.status_code} - {response.text}")
            return False, f"{response.status_code} - {response.text}"
    except Exception as e:
        logger.error(f"Exception sending downlink: {str(e)}")
        return False, str(e)

# Create Discord embed(s) from Grafana alert
def create_alert_embeds(alert_data):
    embeds = []

    try:
        alerts = alert_data.get("alerts", [])
        for alert in alerts:
            status = alert.get("status", "unknown").upper()
            labels = alert.get("labels", {})
            annotations = alert.get("annotations", {})
            summary = annotations.get("summary", "No summary provided")
            alert_name = labels.get("alertname", "Unknown Alert")

            values_text = "N/A"
            value_string = alert.get("valueString", "")
            if value_string:
                import re
                matches = re.findall(r"var='(.*?)'.*?value=([0-9.]+)", value_string)
                if matches:
                    values_text = ", ".join([f"{var}={val}" for var, val in matches])

            color = (
                discord.Color.red() if status == "FIRING"
                else discord.Color.green() if status == "RESOLVED"
                else discord.Color.light_grey()
            )

            formatted_message = (
                "===========\n"
                f"{alert_name}\n\n"
                f"{summary}\n\n"
                f"Valeure mesurée : {values_text}\n\n"
                f"Statut : {status}\n\n"
                "Utilisez les boutons pour une rétroaction :\n"
                "==========="
            )

            # Send SMS if alert is firing
            if status == "FIRING":
                send_sms_twilio(f"Grafalerts - {summary}")

            embed = discord.Embed(
                description=formatted_message,
                color=color,
                timestamp=datetime.now()
            )
            embed.set_footer(text=f"Device: {TTN_CONFIG['device_id']}")
            embeds.append(embed)

    except Exception as e:
        logger.error(f"Error creating embeds: {str(e)}")
        embed = discord.Embed(
            title="Alert Received",
            description="Error parsing alert data",
            color=discord.Color.dark_red()
        )
        embeds.append(embed)

    return embeds

# Function to process the message queue
async def process_message_queue():
    global message_queue
    while True:
        try:
            if not message_queue.empty():
                data = message_queue.get(block=False)
                discord_channel = bot.get_channel(DISCORD_CHANNEL_ID)
                if discord_channel:
                    embeds = create_alert_embeds(data)
                    view = TTNCommandView(data)
                    for embed in embeds:
                        await discord_channel.send(embed=embed, view=view)
                    logger.info("Alert sent to Discord")
                else:
                    logger.error(f"Discord channel {DISCORD_CHANNEL_ID} not found")
                message_queue.task_done()
            await asyncio.sleep(1)
        except Exception as e:
            logger.error(f"Error processing message queue: {str(e)}")
            await asyncio.sleep(5)

# Discord bot event handlers
@bot.event
async def on_ready():
    logger.info(f"Discord bot logged in as {bot.user.name} (ID: {bot.user.id})")
    bot.loop.create_task(process_message_queue())

# Flask webhook route
@app.route('/webhook', methods=['POST'])
def webhook():
    global message_queue
    try:
        data = request.json
        logger.info(f"Received webhook: {data}")
        message_queue.put(data)
        return jsonify({"status": "success", "message": "Alert queued for Discord"}), 200
    except Exception as e:
        logger.error(f"Error processing webhook: {str(e)}")
        return jsonify({"status": "error", "message": str(e)}), 500

# Run both the Flask app and Discord bot together
if __name__ == "__main__":
    import threading
    discord_thread = threading.Thread(target=bot.run, args=(DISCORD_TOKEN,))
    discord_thread.daemon = True
    discord_thread.start()
    port = int(os.environ.get('PORT', 5000))
    app.run(host='0.0.0.0', port=port, debug=False)
