"""
MQTT Publisher

Publisher class for sending telemetry data to MQTT broker.
"""

import json
from mqtt.mqtt_client import MQTTClient

class MQTTPublisher(MQTTClient):
    """MQTT publisher for sending data"""

    def publish(self, topic, payload):
        """
        Publish message to MQTT topic.

        Args:
            topic: MQTT topic string
            payload: Dictionary to be JSON-encoded, or string
        """
        # Check connection and attempt reconnect if needed
        if not self.client.is_connected():
            print("MQTT publisher disconnected, attempting reconnect...")
            try:
                self.client.reconnect()
                print("✓ MQTT publisher reconnected")
            except Exception as e:
                print(f"✗ MQTT reconnect failed: {e}")
                return False

        if isinstance(payload, dict):
            message = json.dumps(payload)
        else:
            message = str(payload)

        result = self.client.publish(topic, message, qos=1)

        if result.rc == 0:
            print(f"Published to {topic}: {message}")
            return True
        else:
            print(f"ERROR: Failed to publish to {topic}. Return code: {result.rc}")
            return False
