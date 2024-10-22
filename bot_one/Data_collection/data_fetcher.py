import os
import json
import datetime
import pandas as pd
from binance.client import Client
from binance.exceptions import BinanceAPIException, BinanceRequestException

# Load configuration from a JSON file
def load_config():
    print('Loading configuration...')
    config_path = 'config/config.json'
    if os.path.exists(config_path):
        with open(config_path) as config_file:
            return json.load(config_file)
    else:
        print(f'Configuration file not found at {config_path}')
        exit(1)

# Connect to the Binance API
def create_client(api_key, api_secret):
    print('Creating Binance client...')
    try:
        return Client(api_key=api_key, api_secret=api_secret)
    except Exception as e:
        print(f'Error creating client: {e}')
        return None

# Fetch data from the Binance API
def fetch_data(bclient, symbol, interval, start_date, end_date):
    print('Fetching data from Binance...')
    try:
        klines = bclient.get_historical_klines(
            symbol,
            interval,
            start_date.strftime("%d %b %Y %H:%M:%S"),
            end_date.strftime("%d %b %Y %H:%M:%S"),
            limit=1000
        )
        return klines
    except BinanceRequestException as bre:
        print(f'Request Error: {bre}')
    except BinanceAPIException as bae:
        print(f'API Error: {bae}')
    except Exception as e:
        print(f'An error occurred: {e}')
    return None

# Convert raw data to DataFrame and save to CSV
def process_and_save_data(raw_data, symbol):
    print('Processing data and saving to CSV...')
    try:
        data = pd.DataFrame(raw_data, columns=[
            'timestamp', 'open', 'high', 'low', 'close', 'volume',
            'close_time', 'quote_asset_volume', 'number_of_trades',
            'taker_buy_base_volume', 'taker_buy_quote_volume', 'ignore'
        ])
        data['timestamp'] = pd.to_datetime(data['timestamp'], unit='ms')
        data.set_index('timestamp', inplace=True)
        filename = f'data/{symbol}_MinuteBars.csv'
        os.makedirs('data', exist_ok=True)
        data.to_csv(filename)
        print(f'Data saved to {filename}')
    except Exception as e:
        print(f'Error processing and saving data: {e}')

# Main function to coordinate the extraction
def binance_bar_extractor():
    print('Starting data extraction...')
    config = load_config()
    bclient = create_client(config['api_key'], config['api_secret'])
    if bclient is None:
        return

    symbol = config.get('symbol', 'BTCUSDT')
    interval = config.get('interval', Client.KLINE_INTERVAL_1MINUTE)
    start_date_str = config.get('start_date', '1 Jan 2021')
    end_date_str = config.get('end_date', datetime.datetime.today().strftime('%d %b %Y'))
    start_date = datetime.datetime.strptime(start_date_str, '%d %b %Y')
    end_date = datetime.datetime.strptime(end_date_str, '%d %b %Y')

    raw_data = fetch_data(bclient, symbol, interval, start_date, end_date)
    if raw_data is None:
        return
    process_and_save_data(raw_data, symbol)
    print('Data extraction finished!')

if __name__ == '__main__':
    binance_bar_extractor()
