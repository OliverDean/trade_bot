import pandas as pd
from binance.client import Client
import datetime
import os
import json
import requests

# Load configuration from a JSON file or environment variables
def load_config():
    print('Loading configuration...')
    try:
        with open('config.json') as config_file:
            return json.load(config_file)
    except:
        return {
            'api_key': os.getenv('API_KEY'),
            'api_secret': os.getenv('API_SECRET'),
            'start_date': os.getenv('START_DATE'),
        }

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
        return bclient.get_historical_klines(symbol, interval, start_date.strftime("%d %b %Y %H:%M:%S"), end_date.strftime("%d %b %Y %H:%M:%S"), 1000)
    except requests.exceptions.RequestException as req_err:
        print(f'Request Error: {req_err}')
    except requests.exceptions.HTTPError as http_err:
        print(f'HTTP Error: {http_err}')
    except requests.exceptions.ConnectionError as conn_err:
        print(f'Error Connecting: {conn_err}')
    except requests.exceptions.Timeout as timeout_err:
        print(f'Timeout Error: {timeout_err}')
    except Exception as e:
        print(f'An error occurred: {e}')
    return None

# Convert raw data to DataFrame and save to CSV
def process_and_save_data(raw_data, symbol):
    print('Processing data and saving to CSV...')
    try:
        data = pd.DataFrame(raw_data, columns = ['timestamp', 'open', 'high', 'low', 'close', 'volume', 'close_time', 'quote_av', 'trades', 'tb_base_av', 'tb_quote_av', 'ignore' ])
        data['timestamp'] = pd.to_datetime(data['timestamp'], unit='ms')
        data.set_index('timestamp', inplace=True)
        filename = '{}_MinuteBars.csv'.format(symbol)
        data.to_csv(filename)
    except Exception as e:
        print(f'Error processing and saving data: {e}')

# Main function to coordinate the extraction
def binanceBarExtractor(symbol):
    print('working...')
    config = load_config()
    bclient = create_client(config['api_key'], config['api_secret'])
    if bclient is None:
        return
    start_date = datetime.datetime.strptime(config['start_date'], '%d %b %Y')
    today = datetime.datetime.today()
    raw_data = fetch_data(bclient, symbol, Client.KLINE_INTERVAL_1MINUTE, start_date, today)
    if raw_data is None:
        return
    process_and_save_data(raw_data, symbol)
    print('finished!')

if __name__ == '__main__':
    binanceBarExtractor('ETHUSDT')
