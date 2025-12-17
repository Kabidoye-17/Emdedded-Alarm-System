import json
import os
from dataclasses import dataclass
from pathlib import Path
from typing import Optional, List

@dataclass
class MQTTConfig:
    broker: str
    port: int
    keep_alive: int
    username: str
    password: Optional[str] = None

@dataclass
class UARTConfig:
    baudrate: int

@dataclass
class TopicsConfig:
    command: str
    update: str

@dataclass
class CommandsConfig:
    valid_uart_commands: List[str]
    mqtt_command_payload_key: str

@dataclass
class ProtocolConfig:
    stx: int
    etx: int
    ack: int
    max_data_length: int
    encoding: str

def load_config():
    """Load configuration from config.json file"""
    config_path = Path(__file__).parent / 'config.json'

    if not config_path.exists():
        raise FileNotFoundError(f"Configuration file not found: {config_path}")

    try:
        with open(config_path, 'r') as f:
            config_data = json.load(f)

        mqtt_data = config_data['mqtt']
        mqtt_config = MQTTConfig(
            broker=mqtt_data['broker'],
            port=mqtt_data['port'],
            keep_alive=mqtt_data['keep_alive'],
            username=mqtt_data['username'],
            password=os.getenv('MQTT_PASSWORD', mqtt_data.get('password')),
        )

        return (
            mqtt_config,
            UARTConfig(**config_data['uart']),
            TopicsConfig(**config_data['topics']),
            CommandsConfig(**config_data['commands']),
            ProtocolConfig(**config_data['protocol'])
        )
    except Exception as e:
        raise RuntimeError(f"Failed to load configuration: {str(e)}")

# Load configs once when module is imported
mqtt, uart, topics, commands, protocol = load_config()
