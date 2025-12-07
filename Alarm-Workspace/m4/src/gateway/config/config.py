import json
from dataclasses import dataclass
from pathlib import Path

@dataclass
class MQTTConfig:
    broker: str
    port: int
    keep_alive: int

@dataclass
class UARTConfig:
    port: str
    baudrate: int

@dataclass
class TopicsConfig:
    command: str

def load_config():
    """Load configuration from config.json file"""
    config_path = Path(__file__).parent / 'config.json'

    if not config_path.exists():
        raise FileNotFoundError(f"Configuration file not found: {config_path}")

    try:
        with open(config_path, 'r') as f:
            config_data = json.load(f)

        return (
            MQTTConfig(**config_data['mqtt']),
            UARTConfig(**config_data['uart']),
            TopicsConfig(**config_data['topics'])
        )
    except Exception as e:
        raise RuntimeError(f"Failed to load configuration: {str(e)}")

# Load configs once when module is imported
mqtt, uart, topics = load_config()
