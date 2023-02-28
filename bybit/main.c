#include "aggregator_bybit.c"
#include <time.h>

int main(int argc, char **argv) {

  if (argc != 4) {
    printf("Wrong number of parameters, please insert args in this order\n");
    printf("1) input_file_name 2) output_file_name 3) frequency in sec\n");
    printf("1 min = 60s\n5min  = 300s\n10min = 600s\n15min = 900s\n45min =\n1h "
           "   = 3600s\n2h    = 7200\n4h    = 14400\n1d    = 86400s\n");
    return (-1);
  }

  char *in_file = argv[1];
  char *out_file = argv[2];
  int frequency = atoi(argv[3]);

  FILE *input = fopen(in_file, "r");
  FILE *output = fopen(out_file, "w");

  if (input == NULL) {
    printf("Can't open input file: %s\n", in_file);
    return (-1);
  } else {
    printf("Dataset loaded...\n");
  }

  if (output == NULL) {
    printf("Can't create output file: %s\n", out_file);
    return (-1);
  } else {
    printf("Output file created...\n");
  }

  // aggregate the passed file
  aggregate(input, output, frequency);

  fclose(input);
  fclose(output);
  return 0;
}