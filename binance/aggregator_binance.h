#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

/* SPOT COLUMNS */
// trade_Id, price, qty, quoteQty, time, isBuyerMaker, isBestMatch

/* FUTURES COLUMNS */
// trade_Id, price, qty, quoteQty, time, isBuyerMaker

/* aggregation data struct */
typedef struct kline {
    unsigned long open_ts;
    unsigned long close_ts;
    double open;
    double high;
    double low;
    double close;
    float buy_vol_usdt;
    float sell_vol_usdt;
    int buy_trades;
    int sell_trades;
    float one_k_delta;
    float ten_k_delta;
    float hundred_k_delta;
    float one_m_delta;
    float ten_m_delta;
} Kline;

// write a kline candle to file: open_ts, close_ts, open, high, low, close, buy_vol_usdt, sell_vol_usdt, buy_trades, sell_trades
int write_kline(Kline *data, FILE *output_file);

// write the file header
int write_header(FILE *output_file);

// utility function to print a Kline candle
void print_candle(Kline *candle);

// function to calculate closing timestamp based on sampling frequency specified and current open time
unsigned long long  next_ts(unsigned long long timestamp, int freq);

// function to aggregate raw tick data
int aggregate(FILE *input_file, FILE *output_file, int resample_frequency);