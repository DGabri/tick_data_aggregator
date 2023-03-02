#include "aggregator_bybit.h"
#include <string.h>
#include <time.h>

// write a kline candle to file: open_ts, close_ts, open, high, low, close,
// buy_vol_usdt, sell_vol_usdt, buy_trades, sell_trades
int write_kline(Kline *data, FILE *output_file) {
  int ret = fprintf(output_file, "%ld,%ld,%.8f,%.8f,%.8f,%.8f,%f,%f,%d,%d\n",
                    data->open_ts, data->close_ts, data->open, data->high,
                    data->low, data->close, data->buy_vol_usdt,
                    data->sell_vol_usdt, data->buy_trades, data->sell_trades);
  return ret;
}

// write the file header
int write_header(FILE *output_file) {
  int ret = fprintf(output_file,
                    "open_ts,close_ts,open,high,low,close,buy_volume_usdt,sell_"
                    "volume_usdt,buy_trades_count,sell_trades_count\n");
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
double next_ts(double ts, int freq) {
  time_t sec = (time_t)ts;
  double msec = ts - (double)sec;
  struct tm *tm = localtime(&sec);
  int seconds = tm->tm_sec + (tm->tm_min * 60) + (tm->tm_hour * 3600);
  int remainder = seconds % freq;
  double elapsed = freq - (double)remainder - msec;
  if (elapsed < 0) {
    elapsed += freq;
  }
  double next_ts = ts + elapsed - msec;
  return next_ts;
}

int aggregate(FILE *input_file, FILE *output_file, int resample_frequency) {
  // csv line trade data
  char buffer[2048];
  int row = 0;
  int column = 0;
  long double price = 0;
  float qty = 0;
  float quote_qty = 0;
  long ts = 0;
  int side = 0;

  Kline *candle = (Kline *)malloc(sizeof(Kline));

  if (candle == NULL) {
    printf("cant't allocate kline");
    free(candle);
    return (-1);
  }

  while (fgets(buffer, 2048, input_file)) {
    // printf("%s\n", buffer);
    column = 0;
    row++;

    // read data is the csv header
    if (row == 1) {
      // write the header in the output csv
      if (write_header(output_file) < 0) {
        printf("Error writing header, exiting...");
        free(candle);
        return (-1);
      }
      // skip header as it is not used to create the candlesticks
      continue;
    }

    // Splitting data
    // puts("***************");
    // printf("%s\n", buffer);
    char *csv_line = strtok(buffer, ",");

    while (csv_line) {
      // get the csv_lines of each csv column
      // col 0 = timestamp, col 1 = symbol, col 2 = side,
      // col 3 = size , col 4 = price, col 5 = tickDirection
      // col 6 = matchID, col 7 = grossValue, col 8 = homeNotional (base qty),
      // col 9 = foreignNotional (quote qty = size * price)

      // trade timestamp
      if (column == 0) {
        ts = atoi(csv_line);
      }
      // trade side
      if (column == 2) {
        int len = strlen(csv_line);
        //  Buy (len = 3) => order is market buy
        //  Sell (len = 4) => order is market sell
        if (len == 3) {
          side = 1;
        } else if (len == 4) {
          side = -1;
        }
      }
      // qty column
      if (column == 3) {
        qty = atof(csv_line);
      }
      // price column
      if (column == 4) {
        sscanf(csv_line, "%Lf", &price);
      }
      // tick column
      if (column == 5) {
        char *tick = csv_line;
      }
      // quote qty column
      if (column == 9) {
        quote_qty = atof(csv_line);
      }

      csv_line = strtok(NULL, ",");
      column++;

      /* AGGREGATION FUNCTION */

      // first data point in the dataset
      // calculate the open and close timestamp for the first candle
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
    }
  }

  free(candle);
  return (1);
}
