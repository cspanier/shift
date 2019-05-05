
SQUISH_ALIGNED float part1delta[16][1][4] = {
  {{1.0f / 4.0f, -3.0f / 4.0f, 1.0f / 4.0f, 0.0f}},
  {{1.0f / 4.0f, -3.0f / 4.0f, 1.0f / 4.0f, 0.0f}},
  {{1.0f / 4.0f, -3.0f / 4.0f, 1.0f / 4.0f, 0.0f}},
  {{1.0f / 4.0f, -3.0f / 4.0f, 1.0f / 4.0f, 0.0f}},
  {{1.0f / 4.0f, -3.0f / 4.0f, 1.0f / 4.0f, 0.0f}},
  {{1.0f / 4.0f, -3.0f / 4.0f, 1.0f / 4.0f, 0.0f}},
  {{1.0f / 4.0f, -3.0f / 4.0f, 1.0f / 4.0f, 0.0f}},
  {{1.0f / 4.0f, -3.0f / 4.0f, 1.0f / 4.0f, 0.0f}},
  {{1.0f / 4.0f, -3.0f / 4.0f, 1.0f / 4.0f, 0.0f}},
  {{1.0f / 4.0f, -3.0f / 4.0f, 1.0f / 4.0f, 0.0f}},
  {{1.0f / 4.0f, -3.0f / 4.0f, 1.0f / 4.0f, 0.0f}},
  {{1.0f / 4.0f, -3.0f / 4.0f, 1.0f / 4.0f, 0.0f}},
  {{1.0f / 4.0f, -3.0f / 4.0f, 1.0f / 4.0f, 0.0f}},
  {{1.0f / 4.0f, -3.0f / 4.0f, 1.0f / 4.0f, 0.0f}},
  {{1.0f / 4.0f, -3.0f / 4.0f, 1.0f / 4.0f, 0.0f}},
  {{1.0f / 4.0f, -3.0f / 4.0f, 1.0f / 4.0f, 0.0f}},
};

SQUISH_ALIGNED float part1inits[16][16][4] = {{/* 1 begin iterations */
                                               {0.25f, 0.25f, 0.25f}},
                                              {/* 2 begin iterations */
                                               {0.25f, 1.25f, 0.25f},
                                               {1.0f, 1.0f, 0.0f}},
                                              {/* 3 begin iterations */
                                               {0.25f, 2.25f, 0.25f},
                                               {1.0f, 2.0f, 0.0f},
                                               {2.0f, 1.0f, 0.0f}},
                                              {/* 4 begin iterations */
                                               {0.25f, 3.25f, 0.25f},
                                               {1.0f, 3.0f, 0.0f},
                                               {2.0f, 2.0f, 0.0f},
                                               {3.0f, 1.0f, 0.0f}},
                                              {/* 5 begin iterations */
                                               {0.25f, 4.25f, 0.25f},
                                               {1.0f, 4.0f, 0.0f},
                                               {2.0f, 3.0f, 0.0f},
                                               {3.0f, 2.0f, 0.0f},
                                               {4.0f, 1.0f, 0.0f}},
                                              {/* 6 begin iterations */
                                               {0.25f, 5.25f, 0.25f},
                                               {1.0f, 5.0f, 0.0f},
                                               {2.0f, 4.0f, 0.0f},
                                               {3.0f, 3.0f, 0.0f},
                                               {4.0f, 2.0f, 0.0f},
                                               {5.0f, 1.0f, 0.0f}},
                                              {/* 7 begin iterations */
                                               {0.25f, 6.25f, 0.25f},
                                               {1.0f, 6.0f, 0.0f},
                                               {2.0f, 5.0f, 0.0f},
                                               {3.0f, 4.0f, 0.0f},
                                               {4.0f, 3.0f, 0.0f},
                                               {5.0f, 2.0f, 0.0f},
                                               {6.0f, 1.0f, 0.0f}},
                                              {/* 8 begin iterations */
                                               {0.25f, 7.25f, 0.25f},
                                               {1.0f, 7.0f, 0.0f},
                                               {2.0f, 6.0f, 0.0f},
                                               {3.0f, 5.0f, 0.0f},
                                               {4.0f, 4.0f, 0.0f},
                                               {5.0f, 3.0f, 0.0f},
                                               {6.0f, 2.0f, 0.0f},
                                               {7.0f, 1.0f, 0.0f}},
                                              {/* 9 begin iterations */
                                               {0.25f, 8.25f, 0.25f},
                                               {1.0f, 8.0f, 0.0f},
                                               {2.0f, 7.0f, 0.0f},
                                               {3.0f, 6.0f, 0.0f},
                                               {4.0f, 5.0f, 0.0f},
                                               {5.0f, 4.0f, 0.0f},
                                               {6.0f, 3.0f, 0.0f},
                                               {7.0f, 2.0f, 0.0f},
                                               {8.0f, 1.0f, 0.0f}},
                                              {/* 10 begin iterations */
                                               {0.25f, 9.25f, 0.25f},
                                               {1.0f, 9.0f, 0.0f},
                                               {2.0f, 8.0f, 0.0f},
                                               {3.0f, 7.0f, 0.0f},
                                               {4.0f, 6.0f, 0.0f},
                                               {5.0f, 5.0f, 0.0f},
                                               {6.0f, 4.0f, 0.0f},
                                               {7.0f, 3.0f, 0.0f},
                                               {8.0f, 2.0f, 0.0f},
                                               {9.0f, 1.0f, 0.0f}},
                                              {/* 11 begin iterations */
                                               {0.25f, 10.25f, 0.25f},
                                               {1.0f, 10.0f, 0.0f},
                                               {2.0f, 9.0f, 0.0f},
                                               {3.0f, 8.0f, 0.0f},
                                               {4.0f, 7.0f, 0.0f},
                                               {5.0f, 6.0f, 0.0f},
                                               {6.0f, 5.0f, 0.0f},
                                               {7.0f, 4.0f, 0.0f},
                                               {8.0f, 3.0f, 0.0f},
                                               {9.0f, 2.0f, 0.0f},
                                               {10.0f, 1.0f, 0.0f}},
                                              {/* 12 begin iterations */
                                               {0.25f, 11.25f, 0.25f},
                                               {1.0f, 11.0f, 0.0f},
                                               {2.0f, 10.0f, 0.0f},
                                               {3.0f, 9.0f, 0.0f},
                                               {4.0f, 8.0f, 0.0f},
                                               {5.0f, 7.0f, 0.0f},
                                               {6.0f, 6.0f, 0.0f},
                                               {7.0f, 5.0f, 0.0f},
                                               {8.0f, 4.0f, 0.0f},
                                               {9.0f, 3.0f, 0.0f},
                                               {10.0f, 2.0f, 0.0f},
                                               {11.0f, 1.0f, 0.0f}},
                                              {/* 13 begin iterations */
                                               {0.25f, 12.25f, 0.25f},
                                               {1.0f, 12.0f, 0.0f},
                                               {2.0f, 11.0f, 0.0f},
                                               {3.0f, 10.0f, 0.0f},
                                               {4.0f, 9.0f, 0.0f},
                                               {5.0f, 8.0f, 0.0f},
                                               {6.0f, 7.0f, 0.0f},
                                               {7.0f, 6.0f, 0.0f},
                                               {8.0f, 5.0f, 0.0f},
                                               {9.0f, 4.0f, 0.0f},
                                               {10.0f, 3.0f, 0.0f},
                                               {11.0f, 2.0f, 0.0f},
                                               {12.0f, 1.0f, 0.0f}},
                                              {/* 14 begin iterations */
                                               {0.25f, 13.25f, 0.25f},
                                               {1.0f, 13.0f, 0.0f},
                                               {2.0f, 12.0f, 0.0f},
                                               {3.0f, 11.0f, 0.0f},
                                               {4.0f, 10.0f, 0.0f},
                                               {5.0f, 9.0f, 0.0f},
                                               {6.0f, 8.0f, 0.0f},
                                               {7.0f, 7.0f, 0.0f},
                                               {8.0f, 6.0f, 0.0f},
                                               {9.0f, 5.0f, 0.0f},
                                               {10.0f, 4.0f, 0.0f},
                                               {11.0f, 3.0f, 0.0f},
                                               {12.0f, 2.0f, 0.0f},
                                               {13.0f, 1.0f, 0.0f}},
                                              {/* 15 begin iterations */
                                               {0.25f, 14.25f, 0.25f},
                                               {1.0f, 14.0f, 0.0f},
                                               {2.0f, 13.0f, 0.0f},
                                               {3.0f, 12.0f, 0.0f},
                                               {4.0f, 11.0f, 0.0f},
                                               {5.0f, 10.0f, 0.0f},
                                               {6.0f, 9.0f, 0.0f},
                                               {7.0f, 8.0f, 0.0f},
                                               {8.0f, 7.0f, 0.0f},
                                               {9.0f, 6.0f, 0.0f},
                                               {10.0f, 5.0f, 0.0f},
                                               {11.0f, 4.0f, 0.0f},
                                               {12.0f, 3.0f, 0.0f},
                                               {13.0f, 2.0f, 0.0f},
                                               {14.0f, 1.0f, 0.0f}},
                                              {
                                                /* 16 begin iterations */
                                                {0.25f, 15.25f, 0.25f},
                                                {1.0f, 15.0f, 0.0f},
                                                {2.0f, 14.0f, 0.0f},
                                                {3.0f, 13.0f, 0.0f},
                                                {4.0f, 12.0f, 0.0f},
                                                {5.0f, 11.0f, 0.0f},
                                                {6.0f, 10.0f, 0.0f},
                                                {7.0f, 9.0f, 0.0f},
                                                {8.0f, 8.0f, 0.0f},
                                                {9.0f, 7.0f, 0.0f},
                                                {10.0f, 6.0f, 0.0f},
                                                {11.0f, 5.0f, 0.0f},
                                                {12.0f, 4.0f, 0.0f},
                                                {13.0f, 3.0f, 0.0f},
                                                {14.0f, 2.0f, 0.0f},
                                                {15.0f, 1.0f, 0.0f},
                                              }};

SQUISH_ALIGNED float part1factors[16][151] = {
  {/* 1 begin iterations */
   FLT_MAX},
  {/* 2 begin iterations */
   4.0f, FLT_MAX, 1.0f, 4.0f},
  {/* 3 begin iterations */
   2.0f, 2.0f, FLT_MAX, 0.5f, 2.0f / 3.0f, 2.0f, 0.5f, 2.0f},
  {/* 4 begin iterations */
   4.0f / 3.0f, 1.0f, 4.0f / 3.0f, FLT_MAX, 1.0f / 3.0f, 4.0f / 11.0f, 0.5f,
   4.0f / 3.0f, 0.25f, 4.0f / 11.0f, 1.0f, 1.0f / 3.0f, 4.0f / 3.0f},
  {/* 5 begin iterations */
   1.0f, 2.0f / 3.0f, 2.0f / 3.0f, 1.0f, FLT_MAX, 0.25f, 0.25f, 2.0f / 7.0f,
   0.4f, 1.0f, 1.0f / 6.0f, 0.2f, 2.0f / 7.0f, 2.0f / 3.0f, 1.0f / 6.0f, 0.25f,
   2.0f / 3.0f, 0.25f, 1.0f},
  {/* 6 begin iterations */
   0.8f,         0.5f,        4.0f / 9.0f,  0.5f,         0.8f,
   FLT_MAX,      0.2f,        4.0f / 21.0f, 0.2f,         4.0f / 17.0f,
   1.0f / 3.0f,  0.8f,        1.0f / 8.0f,  4.0f / 29.0f, 1.0f / 6.0f,
   4.0f / 17.0f, 0.5f,        1.0f / 9.0f,  4.0f / 29.0f, 0.2f,
   4.0f / 9.0f,  1.0f / 8.0f, 4.0f / 21.0f, 0.5f,         0.2f,
   0.8f},
  {/* 7 begin iterations */
   2.0f / 3.0f,  0.4f,         1.0f / 3.0f,  1.0f / 3.0f,  0.4f,
   2.0f / 3.0f,  FLT_MAX,      1.0f / 6.0f,  2.0f / 13.0f, 2.0f / 13.0f,
   1.0f / 6.0f,  0.2f,         2.0f / 7.0f,  2.0f / 3.0f,  1.0f / 10.0f,
   2.0f / 19.0f, 2.0f / 17.0f, 1.0f / 7.0f,  0.2f,         0.4f,
   1.0f / 12.0f, 2.0f / 21.0f, 2.0f / 17.0f, 1.0f / 6.0f,  1.0f / 3.0f,
   1.0f / 12.0f, 2.0f / 19.0f, 2.0f / 13.0f, 1.0f / 3.0f,  1.0f / 10.0f,
   2.0f / 13.0f, 0.4f,         1.0f / 6.0f,  2.0f / 3.0f},
  {/* 8 begin iterations */
   4.0f / 7.0f,  1.0f / 3.0f,  4.0f / 15.0f, 0.25f,        4.0f / 15.0f,
   1.0f / 3.0f,  4.0f / 7.0f,  FLT_MAX,      1.0f / 7.0f,  4.0f / 31.0f,
   1.0f / 8.0f,  4.0f / 31.0f, 1.0f / 7.0f,  4.0f / 23.0f, 0.25f,
   4.0f / 7.0f,  1.0f / 12.0f, 4.0f / 47.0f, 1.0f / 11.0f, 4.0f / 39.0f,
   1.0f / 8.0f,  4.0f / 23.0f, 1.0f / 3.0f,  1.0f / 15.0f, 4.0f / 55.0f,
   1.0f / 12.0f, 4.0f / 39.0f, 1.0f / 7.0f,  4.0f / 15.0f, 1.0f / 16.0f,
   4.0f / 55.0f, 1.0f / 11.0f, 4.0f / 31.0f, 0.25f,        1.0f / 15.0f,
   4.0f / 47.0f, 1.0f / 8.0f,  4.0f / 15.0f, 1.0f / 12.0f, 4.0f / 31.0f,
   1.0f / 3.0f,  1.0f / 7.0f,  4.0f / 7.0f},
  {/* 9 begin iterations */
   0.5f,         2.0f / 7.0f,  2.0f / 9.0f,  0.2f,         0.2f,
   2.0f / 9.0f,  2.0f / 7.0f,  0.5f,         FLT_MAX,      1.0f / 8.0f,
   1.0f / 9.0f,  2.0f / 19.0f, 2.0f / 19.0f, 1.0f / 9.0f,  1.0f / 8.0f,
   2.0f / 13.0f, 2.0f / 9.0f,  0.5f,         1.0f / 14.0f, 1.0f / 14.0f,
   2.0f / 27.0f, 2.0f / 25.0f, 1.0f / 11.0f, 1.0f / 9.0f,  2.0f / 13.0f,
   2.0f / 7.0f,  1.0f / 18.0f, 1.0f / 17.0f, 2.0f / 31.0f, 2.0f / 27.0f,
   1.0f / 11.0f, 1.0f / 8.0f,  2.0f / 9.0f,  1.0f / 20.0f, 1.0f / 18.0f,
   2.0f / 31.0f, 2.0f / 25.0f, 1.0f / 9.0f,  0.2f,         1.0f / 20.0f,
   1.0f / 17.0f, 2.0f / 27.0f, 2.0f / 19.0f, 0.2f,         1.0f / 18.0f,
   1.0f / 14.0f, 2.0f / 19.0f, 2.0f / 9.0f,  1.0f / 14.0f, 1.0f / 9.0f,
   2.0f / 7.0f,  1.0f / 8.0f,  0.5f},
  {/* 10 begin iterations */
   4.0f / 9.0f,  0.25f,        4.0f / 21.0f, 1.0f / 6.0f,  4.0f / 25.0f,
   1.0f / 6.0f,  4.0f / 21.0f, 0.25f,        4.0f / 9.0f,  FLT_MAX,
   1.0f / 9.0f,  4.0f / 41.0f, 1.0f / 11.0f, 4.0f / 45.0f, 1.0f / 11.0f,
   4.0f / 41.0f, 1.0f / 9.0f,  4.0f / 29.0f, 0.2f,         4.0f / 9.0f,
   1.0f / 16.0f, 4.0f / 65.0f, 1.0f / 16.0f, 4.0f / 61.0f, 1.0f / 14.0f,
   4.0f / 49.0f, 1.0f / 10.0f, 4.0f / 29.0f, 0.25f,        1.0f / 21.0f,
   4.0f / 81.0f, 1.0f / 19.0f, 4.0f / 69.0f, 1.0f / 15.0f, 4.0f / 49.0f,
   1.0f / 9.0f,  4.0f / 21.0f, 1.0f / 24.0f, 4.0f / 89.0f, 1.0f / 20.0f,
   4.0f / 69.0f, 1.0f / 14.0f, 4.0f / 41.0f, 1.0f / 6.0f,  1.0f / 25.0f,
   4.0f / 89.0f, 1.0f / 19.0f, 4.0f / 61.0f, 1.0f / 11.0f, 4.0f / 25.0f,
   1.0f / 24.0f, 4.0f / 81.0f, 1.0f / 16.0f, 4.0f / 45.0f, 1.0f / 6.0f,
   1.0f / 21.0f, 4.0f / 65.0f, 1.0f / 11.0f, 4.0f / 21.0f, 1.0f / 16.0f,
   4.0f / 41.0f, 0.25f,        1.0f / 9.0f,  4.0f / 9.0f},
  {/* 11 begin iterations */
   0.4f,         2.0f / 9.0f,  1.0f / 6.0f,  1.0f / 7.0f,  2.0f / 15.0f,
   2.0f / 15.0f, 1.0f / 7.0f,  1.0f / 6.0f,  2.0f / 9.0f,  0.4f,
   FLT_MAX,      1.0f / 10.0f, 2.0f / 23.0f, 2.0f / 25.0f, 1.0f / 13.0f,
   1.0f / 13.0f, 2.0f / 25.0f, 2.0f / 23.0f, 1.0f / 10.0f, 1.0f / 8.0f,
   2.0f / 11.0f, 0.4f,         1.0f / 18.0f, 2.0f / 37.0f, 2.0f / 37.0f,
   1.0f / 18.0f, 1.0f / 17.0f, 2.0f / 31.0f, 2.0f / 27.0f, 1.0f / 11.0f,
   1.0f / 8.0f,  2.0f / 9.0f,  1.0f / 24.0f, 2.0f / 47.0f, 2.0f / 45.0f,
   1.0f / 21.0f, 1.0f / 19.0f, 2.0f / 33.0f, 2.0f / 27.0f, 1.0f / 10.0f,
   1.0f / 6.0f,  1.0f / 28.0f, 2.0f / 53.0f, 2.0f / 49.0f, 1.0f / 22.0f,
   1.0f / 19.0f, 2.0f / 31.0f, 2.0f / 23.0f, 1.0f / 7.0f,  1.0f / 30.0f,
   2.0f / 55.0f, 2.0f / 49.0f, 1.0f / 21.0f, 1.0f / 17.0f, 2.0f / 25.0f,
   2.0f / 15.0f, 1.0f / 30.0f, 2.0f / 53.0f, 2.0f / 45.0f, 1.0f / 18.0f,
   1.0f / 13.0f, 2.0f / 15.0f, 1.0f / 28.0f, 2.0f / 47.0f, 2.0f / 37.0f,
   1.0f / 13.0f, 1.0f / 7.0f,  1.0f / 24.0f, 2.0f / 37.0f, 2.0f / 25.0f,
   1.0f / 6.0f,  1.0f / 18.0f, 2.0f / 23.0f, 2.0f / 9.0f,  1.0f / 10.0f,
   0.4f},
  {/* 12 begin iterations */
   4.0f / 11.0f,  0.2f,          0.148148149f,  1.0f / 8.0f,   0.114285715f,
   1.0f / 9.0f,   0.114285715f,  1.0f / 8.0f,   0.148148149f,  0.2f,
   4.0f / 11.0f,  FLT_MAX,       1.0f / 11.0f,  0.0784313753f, 1.0f / 14.0f,
   0.0677966103f, 1.0f / 15.0f,  0.0677966103f, 1.0f / 14.0f,  0.0784313753f,
   1.0f / 11.0f,  0.114285715f,  1.0f / 6.0f,   4.0f / 11.0f,  1.0f / 20.0f,
   0.0481927693f, 1.0f / 21.0f,  0.0481927693f, 1.0f / 20.0f,  0.0533333346f,
   1.0f / 17.0f,  0.0677966103f, 1.0f / 12.0f,  0.114285715f,  0.2f,
   0.0370370373f, 0.0373831764f, 1.0f / 26.0f,  0.0404040404f, 0.0434782617f,
   0.0481927693f, 1.0f / 18.0f,  0.0677966103f, 1.0f / 11.0f,  0.148148149f,
   0.0312499981f, 0.032520324f,  0.0344827585f, 0.0373831764f, 1.0f / 24.0f,
   0.0481927693f, 1.0f / 17.0f,  0.0784313753f, 1.0f / 8.0f,   0.0285714287f,
   0.0305343494f, 1.0f / 30.0f,  0.0373831764f, 0.0434782617f, 0.0533333346f,
   1.0f / 14.0f,  0.114285715f,  0.0277777761f, 0.0305343494f, 0.0344827585f,
   0.0404040404f, 1.0f / 20.0f,  0.0677966103f, 1.0f / 9.0f,   0.0285714287f,
   0.032520324f,  1.0f / 26.0f,  0.0481927693f, 1.0f / 15.0f,  0.114285715f,
   0.0312499981f, 0.0373831764f, 1.0f / 21.0f,  0.0677966103f, 1.0f / 8.0f,
   0.0370370373f, 0.0481927693f, 1.0f / 14.0f,  0.148148149f,  1.0f / 20.0f,
   0.0784313753f, 0.2f,          1.0f / 11.0f,  4.0f / 11.0f},
  {/* 13 begin iterations */
   1.0f / 3.0f,   2.0f / 11.0f,  2.0f / 15.0f,  1.0f / 9.0f,   1.0f / 10.0f,
   2.0f / 21.0f,  2.0f / 21.0f,  1.0f / 10.0f,  1.0f / 9.0f,   2.0f / 15.0f,
   2.0f / 11.0f,  1.0f / 3.0f,   FLT_MAX,       1.0f / 12.0f,  1.0f / 14.0f,
   2.0f / 31.0f,  2.0f / 33.0f,  1.0f / 17.0f,  1.0f / 17.0f,  2.0f / 33.0f,
   2.0f / 31.0f,  1.0f / 14.0f,  1.0f / 12.0f,  2.0f / 19.0f,  2.0f / 13.0f,
   1.0f / 3.0f,   1.0f / 22.0f,  0.0434782617f, 2.0f / 47.0f,  2.0f / 47.0f,
   0.0434782617f, 1.0f / 22.0f,  0.048780486f,  2.0f / 37.0f,  1.0f / 16.0f,
   1.0f / 13.0f,  2.0f / 19.0f,  2.0f / 11.0f,  1.0f / 30.0f,  1.0f / 30.0f,
   0.0338983051f, 0.0350877158f, 0.0370370373f, 1.0f / 25.0f,  2.0f / 45.0f,
   0.051282052f,  1.0f / 16.0f,  1.0f / 12.0f,  2.0f / 15.0f,  0.0277777761f,
   0.0285714287f, 0.0298507456f, 0.0317460336f, 0.0344827585f, 1.0f / 26.0f,
   2.0f / 45.0f,  2.0f / 37.0f,  1.0f / 14.0f,  1.0f / 9.0f,   0.0249999985f,
   0.0263157878f, 0.0281690136f, 0.0307692308f, 0.0344827585f, 1.0f / 25.0f,
   0.048780486f,  2.0f / 31.0f,  1.0f / 10.0f,  0.0238095243f, 0.025641026f,
   0.0281690136f, 0.0317460336f, 0.0370370373f, 1.0f / 22.0f,  2.0f / 33.0f,
   2.0f / 21.0f,  0.0238095243f, 0.0263157878f, 0.0298507456f, 0.0350877158f,
   0.0434782617f, 1.0f / 17.0f,  2.0f / 21.0f,  0.0249999985f, 0.0285714287f,
   0.0338983051f, 2.0f / 47.0f,  1.0f / 17.0f,  1.0f / 10.0f,  0.0277777761f,
   1.0f / 30.0f,  2.0f / 47.0f,  2.0f / 33.0f,  1.0f / 9.0f,   1.0f / 30.0f,
   0.0434782617f, 2.0f / 31.0f,  2.0f / 15.0f,  1.0f / 22.0f,  1.0f / 14.0f,
   2.0f / 11.0f,  1.0f / 12.0f,  1.0f / 3.0f},
  {/* 14 begin iterations */
   4.0f / 13.0f,  1.0f / 6.0f,   0.121212117f,  1.0f / 10.0f,  4.0f / 45.0f,
   1.0f / 12.0f,  4.0f / 49.0f,  1.0f / 12.0f,  4.0f / 45.0f,  1.0f / 10.0f,
   0.121212117f,  1.0f / 6.0f,   4.0f / 13.0f,  FLT_MAX,       1.0f / 13.0f,
   4.0f / 61.0f,  1.0f / 17.0f,  0.0547945201f, 1.0f / 19.0f,  0.0519480519f,
   1.0f / 19.0f,  0.0547945201f, 1.0f / 17.0f,  4.0f / 61.0f,  1.0f / 13.0f,
   4.0f / 41.0f,  1.0f / 7.0f,   4.0f / 13.0f,  1.0f / 24.0f,  0.0396039598f,
   1.0f / 26.0f,  0.0380952395f, 1.0f / 26.0f,  0.0396039598f, 1.0f / 24.0f,
   4.0f / 89.0f,  1.0f / 20.0f,  4.0f / 69.0f,  1.0f / 14.0f,  4.0f / 41.0f,
   1.0f / 6.0f,   0.0303030293f, 0.0300751869f, 0.0303030293f, 0.03100775f,
   0.0322580636f, 0.0341880359f, 0.0370370373f, 0.0412371121f, 1.0f / 21.0f,
   4.0f / 69.0f,  1.0f / 13.0f,  0.121212117f,  0.0249999985f, 0.0254777074f,
   0.0263157878f, 0.0275862049f, 0.029411763f,  0.0320000015f, 1.0f / 28.0f,
   0.0412371121f, 1.0f / 20.0f,  4.0f / 61.0f,  1.0f / 10.0f,  0.0222222228f,
   0.0231213868f, 0.024390243f,  0.0261437893f, 0.0285714287f, 0.0320000015f,
   0.0370370373f, 4.0f / 89.0f,  1.0f / 17.0f,  4.0f / 45.0f,  0.020833334f,
   0.0220994484f, 0.0238095243f, 0.0261437893f, 0.029411763f,  0.0341880359f,
   1.0f / 24.0f,  0.0547945201f, 1.0f / 12.0f,  0.0204081628f, 0.0220994484f,
   0.024390243f,  0.0275862049f, 0.0322580636f, 0.0396039598f, 1.0f / 19.0f,
   4.0f / 49.0f,  0.020833334f,  0.0231213868f, 0.0263157878f, 0.03100775f,
   1.0f / 26.0f,  0.0519480519f, 1.0f / 12.0f,  0.0222222228f, 0.0254777074f,
   0.0303030293f, 0.0380952395f, 1.0f / 19.0f,  4.0f / 45.0f,  0.0249999985f,
   0.0300751869f, 1.0f / 26.0f,  0.0547945201f, 1.0f / 10.0f,  0.0303030293f,
   0.0396039598f, 1.0f / 17.0f,  0.121212117f,  1.0f / 24.0f,  4.0f / 61.0f,
   1.0f / 6.0f,   1.0f / 13.0f,  4.0f / 13.0f},
  {/* 15 begin iterations */
   2.0f / 7.0f,   2.0f / 13.0f,  1.0f / 9.0f,   1.0f / 11.0f,  2.0f / 25.0f,
   2.0f / 27.0f,  1.0f / 14.0f,  1.0f / 14.0f,  2.0f / 27.0f,  2.0f / 25.0f,
   1.0f / 11.0f,  1.0f / 9.0f,   2.0f / 13.0f,  2.0f / 7.0f,   FLT_MAX,
   1.0f / 14.0f,  2.0f / 33.0f,  2.0f / 37.0f,  1.0f / 20.0f,  1.0f / 21.0f,
   0.0465116277f, 0.0465116277f, 1.0f / 21.0f,  1.0f / 20.0f,  2.0f / 37.0f,
   2.0f / 33.0f,  1.0f / 14.0f,  1.0f / 11.0f,  2.0f / 15.0f,  2.0f / 7.0f,
   1.0f / 26.0f,  2.0f / 55.0f,  0.0350877158f, 0.0344827585f, 0.0344827585f,
   0.0350877158f, 2.0f / 55.0f,  1.0f / 26.0f,  1.0f / 24.0f,  0.0465116277f,
   2.0f / 37.0f,  1.0f / 15.0f,  1.0f / 11.0f,  2.0f / 13.0f,  0.0277777761f,
   0.0273972601f, 0.0273972601f, 0.0277777761f, 0.0285714287f, 0.0298507456f,
   0.0317460336f, 0.0344827585f, 1.0f / 26.0f,  2.0f / 45.0f,  2.0f / 37.0f,
   1.0f / 14.0f,  1.0f / 9.0f,   0.0227272715f, 0.0229885057f, 0.0235294122f,
   0.024390243f,  0.025641026f,  0.0273972601f, 0.0298507456f, 1.0f / 30.0f,
   1.0f / 26.0f,  0.0465116277f, 2.0f / 33.0f,  1.0f / 11.0f,  0.0199999996f,
   0.0206185561f, 0.0215053763f, 0.0227272715f, 0.024390243f,  0.0266666673f,
   0.0298507456f, 0.0344827585f, 1.0f / 24.0f,  2.0f / 37.0f,  2.0f / 25.0f,
   0.0185185187f, 0.0194174759f, 0.0206185561f, 0.0222222228f, 0.024390243f,
   0.0273972601f, 0.0317460336f, 1.0f / 26.0f,  1.0f / 20.0f,  2.0f / 27.0f,
   0.0178571418f, 0.0190476198f, 0.0206185561f, 0.0227272715f, 0.025641026f,
   0.0298507456f, 2.0f / 55.0f,  1.0f / 21.0f,  1.0f / 14.0f,  0.0178571418f,
   0.0194174759f, 0.0215053763f, 0.024390243f,  0.0285714287f, 0.0350877158f,
   0.0465116277f, 1.0f / 14.0f,  0.0185185187f, 0.0206185561f, 0.0235294122f,
   0.0277777761f, 0.0344827585f, 0.0465116277f, 2.0f / 27.0f,  0.0199999996f,
   0.0229885057f, 0.0273972601f, 0.0344827585f, 1.0f / 21.0f,  2.0f / 25.0f,
   0.0227272715f, 0.0273972601f, 0.0350877158f, 1.0f / 20.0f,  1.0f / 11.0f,
   0.0277777761f, 2.0f / 55.0f,  2.0f / 37.0f,  1.0f / 9.0f,   1.0f / 26.0f,
   2.0f / 33.0f,  2.0f / 13.0f,  1.0f / 14.0f,  2.0f / 7.0f},
  {/* 16 begin iterations */
   4.0f / 15.0f,  1.0f / 7.0f,   4.0f / 39.0f,  1.0f / 12.0f,  4.0f / 55.0f,
   1.0f / 15.0f,  0.0634920672f, 1.0f / 16.0f,  0.0634920672f, 1.0f / 15.0f,
   4.0f / 55.0f,  1.0f / 12.0f,  4.0f / 39.0f,  1.0f / 7.0f,   4.0f / 15.0f,
   FLT_MAX,       1.0f / 15.0f,  4.0f / 71.0f,  1.0f / 20.0f,  0.0459770113f,
   0.0434782617f, 0.042105265f,  1.0f / 24.0f,  0.042105265f,  0.0434782617f,
   0.0459770113f, 1.0f / 20.0f,  4.0f / 71.0f,  1.0f / 15.0f,  4.0f / 47.0f,
   1.0f / 8.0f,   4.0f / 15.0f,  1.0f / 28.0f,  0.0336134471f, 0.0322580636f,
   0.0314960629f, 0.0312499981f, 0.0314960629f, 0.0322580636f, 0.0336134471f,
   1.0f / 28.0f,  0.0388349518f, 0.0434782617f, 0.0506329127f, 1.0f / 16.0f,
   4.0f / 47.0f,  1.0f / 7.0f,   0.025641026f,  0.0251572318f, 0.0249999985f,
   0.0251572318f, 0.025641026f,  0.0264900662f, 0.0277777761f, 0.0296296291f,
   0.0322580636f, 0.0360360369f, 1.0f / 24.0f,  0.0506329127f, 1.0f / 15.0f,
   4.0f / 39.0f,  0.020833334f,  0.0209424086f, 0.0212765951f, 0.0218579229f,
   0.0227272715f, 0.0239520948f, 0.025641026f,  0.0279720277f, 0.0312499981f,
   0.0360360369f, 0.0434782617f, 4.0f / 71.0f,  1.0f / 12.0f,  0.0181818176f,
   0.0186046511f, 0.0192307699f, 0.0201005023f, 0.0212765951f, 0.0228571426f,
   0.0249999985f, 0.0279720277f, 0.0322580636f, 0.0388349518f, 1.0f / 20.0f,
   4.0f / 55.0f,  0.0166666657f, 0.0173160173f, 0.0181818176f, 0.0193236712f,
   0.020833334f,  0.0228571426f, 0.025641026f,  0.0296296291f, 1.0f / 28.0f,
   0.0459770113f, 1.0f / 15.0f,  0.0158730168f, 0.0167364012f, 0.0178571418f,
   0.0193236712f, 0.0212765951f, 0.0239520948f, 0.0277777761f, 0.0336134471f,
   0.0434782617f, 0.0634920672f, 0.0156249991f, 0.0167364012f, 0.0181818176f,
   0.0201005023f, 0.0227272715f, 0.0264900662f, 0.0322580636f, 0.042105265f,
   1.0f / 16.0f,  0.0158730168f, 0.0173160173f, 0.0192307699f, 0.0218579229f,
   0.025641026f,  0.0314960629f, 1.0f / 24.0f,  0.0634920672f, 0.0166666657f,
   0.0186046511f, 0.0212765951f, 0.0251572318f, 0.0312499981f, 0.042105265f,
   1.0f / 15.0f,  0.0181818176f, 0.0209424086f, 0.0249999985f, 0.0314960629f,
   0.0434782617f, 4.0f / 55.0f,  0.020833334f,  0.0251572318f, 0.0322580636f,
   0.0459770113f, 1.0f / 12.0f,  0.025641026f,  0.0336134471f, 1.0f / 20.0f,
   4.0f / 39.0f,  1.0f / 28.0f,  4.0f / 71.0f,  1.0f / 7.0f,   1.0f / 15.0f,
   4.0f / 15.0f}};

extern float part2delta[1][4];
extern float part2inits[152][4];
extern float part2factors[967];
