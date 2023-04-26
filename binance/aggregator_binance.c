#include "aggregator_binance.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// write a kline candle to file: open_ts, close_ts, open, high, low, close,
// buy_vol_usdt, sell_vol_usdt, buy_trades, sell_trades
int write_kline(Kline *data, FILE *output_file) {

  int ret = fprintf(
      output_file,
      "%ld,%ld,%.8f,%.8f,%.8f,%.8f,%f,%f,%d,%d,%.8f,%.8f,%.8f,%.8f,%.8f\n",
      data->open_ts, data->close_ts, data->open, data->high, data->low,
      data->close, data->buy_vol_usdt, data->sell_vol_usdt, data->buy_trades,
      data->sell_trades, data->one_k_delta, data->ten_k_delta,
      data->hundred_k_delta, data->one_m_delta, data->ten_m_delta);
  return ret;
}

// write the file header
int write_header(FILE *output_file) {
  int ret = fprintf(output_file,
                    "open_ts,close_ts,open,high,low,close,buy_volume_usdt,sell_"
                    "volume_usdt,buy_trades_count,sell_trades_count,1k_delta,"
                    "10k_delta,100k_delta,1m_delta,10m_delta\n");
  return ret;
}

// utility function to print a Kline candle
void print_candle(Kline *candle) {
  printf("OPEN_TS       CLOSE_TS      OPEN      HIGH      LOW       CLOSE     "
         "BUY_VOL        SELL_VOL\n");
  printf("%ld %ld %lf %lf %lf %lf %lf %lf\n", candle->open_ts, candle->close_ts,
         candle->open, candle->high, candle->low, candle->close,
         candle->buy_vol_usdt, candle->sell_vol_usdt);
  printf("-----------------------------------------------------\n");
}

// function to calculate closing timestamp based on sampling frequency specified
// and current open time
unsigned long long next_ts(unsigned long long timestamp, int freq) {
  unsigned long long rounded_timestamp =
      (timestamp / (freq * 1000)) * (freq * 1000);
  if (timestamp % (freq * 1000) > 0) {
    rounded_timestamp += freq * 1000;
  }
  return rounded_timestamp;
}

int aggregate(FILE *input_file, FILE *output_file, int resample_frequency) {
  // csv line trade data
  long double price = 0;
  float qty = 0;
  float quote_qty = 0;
  long ts = 0;
  int side = 0;
  float one_k_vol = 0;
  float ten_k_vol = 0;
  float hundred_k_vol = 0;
  float one_m_vol = 0;
  float ten_m_vol = 0;

  Kline *candle = (Kline *)malloc(sizeof(Kline));

  if (candle == NULL) {
    printf("cant't allocate kline");
    free(candle);
    return (-1);
  }

  char buffer[2048];
  int row = 0;
  int column = 0;

  while (fgets(buffer, 2048, input_file)) {
    // printf("%s\n", buffer);
    column = 0;
    row++;
    // read data is the csv header
    if (row == 1) {
      if (write_header(output_file) < 0) {
        printf("Error writing header, exiting...");
        free(candle);
        return (-1);
      }
    }

    // Splitting data
    // printf("%s\n", buffer);
    char *csv_line = strtok(buffer, ",");

    while (csv_line) {
      // get the csv_lines of each csv column
      // price column
      if (column == 0) {
        sscanf(csv_line, "%Lf", &price);
      }
      // qty column
      if (column == 1) {
        qty = atof(csv_line);
      }
      // quote qty column
      if (column == 2) {
        quote_qty = atof(csv_line);
      }
      // trade timestamp
      if (column == 3) {
        ts = atof(csv_line);
      }
      // trade side
      if (column == 4) {
        int len = strlen(csv_line);
        // false (len = 6) => order is market buy
        // true (len = 5) => order is market sell
        if (len == 5) {
          side = 1;
        } else if (len == 4) {
          side = -1;
        }
      }

      csv_line = strtok(NULL, ",");
      column++;

      /* AGGREGATION FUNCTION */
      // first data point in the dataset
      if (row == 2) {
        candle->close_ts = next_ts(ts, resample_frequency);
        candle->open_ts = (candle->close_ts - (resample_frequency * 1000));
        candle->open = price;
      }

      if (ts > (candle->close_ts)) {
        candle->close = price;
        // write to file
        int write_code = write_kline(candle, output_file);
        if (write_code < 0) {
          printf("Error writing new line, exiting...");
          free(candle);
          return (-1);
        }
        // reset candle struct
        candle->open_ts = candle->close_ts;
        candle->close_ts = next_ts(ts, resample_frequency);
        candle->open = price;
        candle->high = 0;
        candle->low = 100000000000;
        candle->close = 0;
        candle->buy_vol_usdt = 0;
        candle->sell_vol_usdt = 0;
        candle->buy_trades = 0;
        candle->sell_trades = 0;
        candle->one_k_delta = 0;
        candle->ten_k_delta = 0;
        candle->hundred_k_delta = 0;
        candle->one_m_delta = 0;
        candle->ten_m_delta = 0;
      }

      // update price values
      if (price > (candle->high)) {
        candle->high = price;
      }
      if (price < (candle->low)) {
        candle->low = price;
      }

      // market sell
      if (side == -1) {
        candle->sell_vol_usdt += quote_qty;
        candle->sell_trades += 1;
      } else {
        // market buy
        candle->buy_vol_usdt += quote_qty;
        candle->buy_trades += 1;
      }

      // volume delta

      if (quote_qty <= 1000) {
        candle->one_k_delta += (quote_qty * side);
      } else if (quote_qty > 1000 && quote_qty <= 10000) {
        candle->ten_k_delta += (quote_qty * side);
      } else if (quote_qty > 10000 && quote_qty <= 100000) {
        candle->hundred_k_delta += (quote_qty * side);
      } else if (quote_qty > 100000 && quote_qty <= 1000000) {
        candle->one_m_delta += (quote_qty * side);
      } else if (quote_qty > 1000000) {
        candle->ten_m_delta += (quote_qty * side);
      }
    }
  }
  free(candle);

  return (1);
}
