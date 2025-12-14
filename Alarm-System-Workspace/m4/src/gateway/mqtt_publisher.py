"""
MQTT Publisher

Publisher class for sending telemetry data to MQTT broker.
"""

import json
from mqtt_client import MQTTClient

class MQTTPublisher(MQTTClient):
    """MQTT publisher for sending telemetry data"""

    def publish(self, topic, payload):
        """
        Publish message to MQTT topic.

        Args:
            topic: MQTT topic string
            payload: Dictionary to be JSON-encoded, or string
        """
        if isinstance(payload, dict):
            message = json.dumps(payload)
        else:
            message = str(payload)

        result = self.client.publish(topic, message, qos=1)

        if result.rc == 0:
            print(f"Published to {topic}: {message}")
        else:
            print(f"ERROR: Failed to publish to {topic}")
