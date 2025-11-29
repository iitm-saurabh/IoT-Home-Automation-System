#!/usr/bin/env python3
"""
IoT Home Automation System - Sensor Data Collector
File: sensor_collector.py
Description: Collects temperature and humidity data from 8051 MCU via UART
             and publishes to MQTT broker
"""

import serial
import json
import time
import logging
from datetime import datetime
import paho.mqtt.client as mqtt

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

# Configuration
UART_PORT = '/dev/ttyUSB0'  # Serial port (Windows: 'COM3', Linux: '/dev/ttyUSB0')
BAUD_RATE = 9600
MQTT_BROKER = 'localhost'
MQTT_PORT = 1883
MQTT_TOPIC_TEMP = 'home/room1/temperature'
MQTT_TOPIC_HUM = 'home/room1/humidity'
MQTT_TOPIC_STATUS = 'home/room1/status'

class SensorCollector:
    def __init__(self):
        """Initialize sensor collector with UART and MQTT"""
        self.serial_conn = None
        self.mqtt_client = mqtt.Client()
        self.running = True
        
    def setup_uart(self):
        """Setup UART connection"""
        try:
            self.serial_conn = serial.Serial(
                port=UART_PORT,
                baudrate=BAUD_RATE,
                timeout=1,
                bytesize=serial.EIGHTBITS,
                stopbits=serial.STOPBITS_ONE,
                parity=serial.PARITY_NONE
            )
            logger.info(f"UART connection established on {UART_PORT}")
            return True
        except serial.SerialException as e:
            logger.error(f"Failed to open UART port: {e}")
            return False
    
    def setup_mqtt(self):
        """Setup MQTT connection"""
        try:
            self.mqtt_client.on_connect = self.on_mqtt_connect
            self.mqtt_client.on_disconnect = self.on_mqtt_disconnect
            self.mqtt_client.connect(MQTT_BROKER, MQTT_PORT, keepalive=60)
            self.mqtt_client.loop_start()
            logger.info(f"Connecting to MQTT broker at {MQTT_BROKER}:{MQTT_PORT}")
            return True
        except Exception as e:
            logger.error(f"Failed to setup MQTT: {e}")
            return False
    
    def on_mqtt_connect(self, client, userdata, flags, rc):
        """MQTT connection callback"""
        if rc == 0:
            logger.info("MQTT: Connected successfully")
            self.mqtt_client.publish(MQTT_TOPIC_STATUS, "online")
        else:
            logger.error(f"MQTT: Connection failed with code {rc}")
    
    def on_mqtt_disconnect(self, client, userdata, rc):
        """MQTT disconnection callback"""
        if rc != 0:
            logger.warning(f"MQTT: Unexpected disconnection (code {rc})")
    
    def parse_sensor_data(self, data_string):
        """Parse sensor data from UART
        Expected format: TEMP:XX HUM:XX STATUS:X
        """
        try:
            parts = data_string.strip().split()
            sensor_data = {}
            
            for part in parts:
                if part.startswith('TEMP:'):
                    sensor_data['temperature'] = float(part[5:])
                elif part.startswith('HUM:'):
                    sensor_data['humidity'] = float(part[4:])
                elif part.startswith('STATUS:'):
                    sensor_data['status'] = int(part[7:])
            
            if 'temperature' in sensor_data and 'humidity' in sensor_data:
                return sensor_data
        except Exception as e:
            logger.error(f"Error parsing sensor data: {e}")
        
        return None
    
    def read_sensor_data(self):
        """Read data from microcontroller via UART"""
        try:
            if self.serial_conn.in_waiting:
                data = self.serial_conn.readline().decode('utf-8')
                return data
        except Exception as e:
            logger.error(f"Error reading UART: {e}")
        
        return None
    
    def publish_data(self, sensor_data):
        """Publish sensor data to MQTT broker"""
        try:
            timestamp = datetime.now().isoformat()
            
            # Publish temperature
            temp_payload = json.dumps({
                'value': sensor_data['temperature'],
                'timestamp': timestamp,
                'unit': '°C'
            })
            self.mqtt_client.publish(MQTT_TOPIC_TEMP, temp_payload)
            
            # Publish humidity
            hum_payload = json.dumps({
                'value': sensor_data['humidity'],
                'timestamp': timestamp,
                'unit': '%'
            })
            self.mqtt_client.publish(MQTT_TOPIC_HUM, hum_payload)
            
            logger.info(f"Published - Temp: {sensor_data['temperature']}°C, "
                       f"Humidity: {sensor_data['humidity']}%")
        except Exception as e:
            logger.error(f"Error publishing data: {e}")
    
    def request_sensor_reading(self):
        """Request sensor reading from MCU via UART"""
        try:
            self.serial_conn.write(b'R')  # Send 'R' command
            logger.debug("Requested sensor reading from MCU")
        except Exception as e:
            logger.error(f"Error requesting sensor reading: {e}")
    
    def run(self):
        """Main collector loop"""
        logger.info("Starting sensor data collector...")
        
        if not self.setup_uart():
            logger.error("Failed to setup UART")
            return
        
        if not self.setup_mqtt():
            logger.error("Failed to setup MQTT")
            return
        
        # Give MQTT time to connect
        time.sleep(2)
        
        logger.info("Sensor collector running. Press Ctrl+C to stop.")
        
        try:
            while self.running:
                # Request sensor reading
                self.request_sensor_reading()
                time.sleep(1)  # Wait for MCU to process
                
                # Read response
                data = self.read_sensor_data()
                if data:
                    sensor_data = self.parse_sensor_data(data)
                    if sensor_data:
                        self.publish_data(sensor_data)
                
                time.sleep(4)  # Wait before next reading (5 seconds total interval)
        
        except KeyboardInterrupt:
            logger.info("Stopping sensor collector...")
            self.stop()
    
    def stop(self):
        """Stop the collector"""
        self.running = False
        
        if self.mqtt_client:
            self.mqtt_client.publish(MQTT_TOPIC_STATUS, "offline")
            self.mqtt_client.loop_stop()
            self.mqtt_client.disconnect()
        
        if self.serial_conn and self.serial_conn.is_open:
            self.serial_conn.close()
        
        logger.info("Sensor collector stopped")

def main():
    """Main entry point"""
    collector = SensorCollector()
    collector.run()

if __name__ == "__main__":
    main()
