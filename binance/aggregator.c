#include "aggregator.h"
#include <string.h>

// write a kline candle to file: open_ts, close_ts, open, high, low, close,
// buy_vol_usdt, sell_vol_usdt, buy_trades, sell_trades
int write_kline(Kline *data, FILE *output_file) {
  int ret = fprintf(output_file, "%ld,%ld,%f,%f,%f,%f,%f,%f,%d,%d\n",
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
  printf("%ld %ld %f %f %f %f %f %f\n", candle->open_ts, candle->close_ts,
         candle->open, candle->high, candle->low, candle->close,
         candle->buy_vol_usdt, candle->sell_vol_usdt);
  printf("-----------------------------------------------------\n");
}

// function to calculate closing timestamp based on sampling frequency specified
// and current open time
unsigned long next_timestamp(unsigned long timestamp, unsigned int seconds) {
  return ((timestamp + (seconds * 1000) - 1) / (seconds * 1000)) *
         (seconds * 1000);
}

int aggregate(FILE *input_file, FILE *output_file, int resample_frequency) {
  // csv line trade data
  float price = 0;
  float qty = 0;
  float quote_qty = 0;
  long ts = 0;
  int side = 0;

  Kline *candle = (Kline *)malloc(sizeof(Kline));

  if (candle == NULL) {
    printf("cant't allocate kline");
    free(candle);
    free(input_file);
    free(output_file);
    return (-1);
  }

  char buffer[2048];
  int row = 0;
  int column = 0;

  while (fgets(buffer, 2048, input_file)) {
    column = 0;
    row++;
    // read data is the csv header
    if (row == 1) {
      if (write_header(output_file) < 0) {
        printf("Error writing header, exiting...");
        free(candle);
        free(input_file);
        free(output_file);
        return (-1);
      }
    }

    // Splitting data
    char *csv_line = strtok(buffer, ",");

    while (csv_line) {
      // get the csv_lines of each csv column
      // price column
      if (column == 1) {
        price = atof(csv_line);
      }
      // qty column
      if (column == 2) {
        qty = atof(csv_line);
      }
      // quote qty column
      if (column == 3) {
        quote_qty = atof(csv_line);
      }
      // trade timestamp
      if (column == 4) {
        ts = atof(csv_line);
      }
      // trade side
      if (column == 5) {
        int len = strlen(csv_line);
        // false (len = 5) => order is market buy
        // true (len = 4) => order is market sell
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
        candle->close_ts = next_timestamp(ts, resample_frequency);
        candle->open_ts = (candle->close_ts - (resample_frequency * 1000));
        candle->open = price;
      }

      if (ts > (candle->close_ts)) {
        candle->close = price;
        // write to file
        if (write_kline(candle, output_file) < 0) {
          printf("Error writing new line, exiting...");
          free(candle);
          free(input_file);
          free(output_file);
          return (-1);
        }
        // reset candle struct
        candle->open_ts = candle->close_ts;
        candle->close_ts = next_timestamp(ts, resample_frequency);
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
