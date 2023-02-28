# Tick Data Aggregator

This repo is useful if you want to aggregate raw tick data into candlesticks of your provided frequency i.e. 60 sec etc.
The provided scripts are working on binance and bybit tick data.

You can find my article about these scripts on my medium: https://medium.com/@gabriele.deri/maximizing-trading-potential-how-to-convert-raw-tick-data-to-ohlc-candles-with-buy-and-sell-volume-46558cdcf94a

- To aggregate data please: 

   1. make
   2. download raw tick data from: 
      - Binance: https://data.binance.vision/ 
      - Bibit: https://public.bybit.com/trading/ 
   3. execute main.c with these parameters: ./main tick_data_filename output_file_name.csv resample_frequency

***this script is still being tested so bugs are possible, please report them if you find some***

any help to improve this repo is welcomed!

If you want to donate something here are my wallets:
* BTC(btc network): 1DBJE8eTfo6Bk6uViznu7q75iEFHWdpmJL
* BNB (BEP 20): 0x421ae0ae20e148c01dc211d3cb70a51ad0bc51e9
* ETH (ERC 20): 0x421ae0ae20e148c01dc211d3cb70a51ad0bc51e9
* USDT (TRC 20): TPYKQWjdpLuMMXbNJyj3KVtJjX9AUWPvJh
