#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define HEIGHT 600
#define WIDTH 800
#define RIGHT 1
#define UP 0
#define DOWN 2
#define LEFT 3
#define SIZE 3
#define MAX 10

typedef unsigned char uchar;
typedef struct
{
  int x; //x 座標
  int y; //y 座標
} IprPoint;
typedef struct
{
  int max;        // スタックサイズ
  int ptr;        // スタックポインタ
  IprPoint *data; // スタック(先頭要素へのポインタ)
} IprStack;
// 二つの整数を受け取って,構造体に変換する関数
IprPoint iprPoint(int x, int y)
{
  IprPoint p;
  p.x = x;
  p.y = y;
  return p;
}
uchar clip(double x)
{
  if (x >= 256)
    return 255;
  if (x < 0)
    return 0;
  return x;
}
int labeling(uchar binary[][WIDTH], int labelImage[][WIDTH]);
void rgb2hsv(uchar rgb[][WIDTH][3], int hsv[][WIDTH][3]);
void ipr_load_ppm(unsigned char image[][WIDTH][3], const char path[]);
void ipr_save_pgm(unsigned char image[][WIDTH], const char path[]);
void copy_image(uchar src[][WIDTH], uchar dst[][WIDTH]);
void dilate(uchar src[][WIDTH], uchar dst[][WIDTH], int iteration);
void erode(uchar src[][WIDTH], uchar dst[][WIDTH], int iteration);
void opening(uchar src[][WIDTH], uchar dst[][WIDTH], int iteration);
void closing(uchar src[][WIDTH], uchar dst[][WIDTH], int iteration);
int kukei(int labelImage[][WIDTH], int label, int kukei_y[], int kukei_x[], int *kukei_width, int *kukei_height, int *left_up_x, int *left_up_y);
uchar clip(double x);
int length(uchar image[][WIDTH], int m, int n);
double enkeido(int length, int npixel);
int cnt_fing(int labelImage[][WIDTH], int label, int kukei_y[], int kukei_x[], int wid, int hei);
int is_type(int member, int *hand)
{
  char type[3]; /*削除してもらって大丈夫です。*/
  int kekka;
  int i;
  int count[SIZE];

  for (i = 0; i < member; i++)
  {
    count[i] = 0;
  }

  kekka = 0;
  /*
  グー　　＋１
  チョキ　＋２
  パー　　＋４
  （一度出た手のタイプはプラスしない）*/
  for (i = 1; i <= member; i++)
  {
    if (hand[i] == 0)
    {
      if (count[0] == 0)
      {
        kekka += 1;
        count[0] = 1;
      }
    }
    else if (hand[i] == 1)
    {
      if (count[1] == 0)
      {
        kekka += 2;
        count[1] = 1;
      }
    }
    else if (hand[i] == 2)
    {
      if (count[2] == 0)
      {
        kekka += 4;
        count[2] = 1;
      }
    }
    else
    {
      continue;
    }
  }
  printf("kekka = %d\n", kekka); //確認用
  return kekka;
}
char *G_C_P(int *hand, int i)
{
  char *type[MAX];
  /*
  グー 　　０
  チョキ 　１
  パー　 　２　*/
  if (hand[i] == 0)
  {
    type[i] = "G";
  }
  else if (hand[i] == 1)
  {
    type[i] = "C";
  }
  else if (hand[i] == 2)
  {
    type[i] = "P";
  }
  return type[i];
}
char *out_kekka(int kekka, int *hand, int i)
{
  char *janken[MAX];
  /* kekkaが
  1 = 全員グー
  2 = 全員チョキ
  3 = グーが勝ち
  4 = 全員パー
  5 = パーが勝ち
  6 = チョキが勝ち
  7 = グーチョキパーで引き分け */
  if (kekka == 1 || kekka == 2 || kekka == 4 || kekka == 7)
  {
    janken[i] = "E";
  }
  else if (kekka == 3)
  {
    if (hand[i] == 0)
    {
      janken[i] = "W";
    }
    else
    {
      janken[i] = "L";
    }
  }
  else if (kekka == 5)
  {
    if (hand[i] == 2)
    {
      janken[i] = "W";
    }
    else
    {
      janken[i] = "L";
    }
  }
  else if (kekka == 6)
  {
    if (hand[i] == 1)
    {
      janken[i] = "W";
    }
    else
    {
      janken[i] = "L";
    }
  }
  return janken[i];
}

int main(int argc, char *argv[])
{
  static uchar rgb[HEIGHT][WIDTH][3];
  static int hsv[HEIGHT][WIDTH][3];
  static uchar src[HEIGHT][WIDTH];
  static  uchar dst[HEIGHT][WIDTH];
  int x, y, l;
  // PPM 画像を入力する
  ipr_load_ppm(rgb, argv[1]);
  // RGB ==> HSV に変換
  rgb2hsv(rgb, hsv);
  for (int y = 0; y < HEIGHT; y++)
  {
    for (int x = 0; x < WIDTH; x++)
    {
      if (((hsv[y][x][0] >= 0 && hsv[y][x][0] <= 40) ||
           (hsv[y][x][0] >= 330 && hsv[y][x][0] <= 359)) &&
          (hsv[y][x][1] >= 20 && hsv[y][x][1] <= 100))
      {
        src[y][x] = 255;
      }
      else
      {
        src[y][x] = 0;
      }
    }
  }

  closing(src, dst, 1);
  opening(src, dst, 1);
  //PGM画像にHSV変換後を保存

  static int labels[HEIGHT][WIDTH];
  int nlabel = labeling(dst, labels); // nlabel はラベル数 // 面積を計測する
  // nPixels[l] が,ラベル l の画素数
  // l = 0 は背景を意味するので,nlabel + 1 個の要素からなる配列を確保する
  int *nPixels = (int *)calloc(nlabel + 1, sizeof(int));
  for (y = 0; y < HEIGHT; y++)
  {
    for (x = 0; x < WIDTH; x++)
    {
      l = labels[y][x];
      nPixels[l]++;
    }
  }

  for (l = 1; l <= nlabel; l++)
  {
    if (nPixels[l] < 3000)
    {
      for (y = 0; y < HEIGHT; y++)
      {
        for (x = 0; x < WIDTH; x++)
        {
          if (labels[y][x] == l)
          {
            dst[y][x] = 0;
          }
        }
      }
    }
  }

  //ipr_save_pgm(dst, argv[2]);

  // 面積を計測する
  //nPixels[l] が,ラベル l の画素数
  nlabel = labeling(dst, labels); // nlabel はラベル数
  static int kukei_y[3], kukei_x[3];
  int kukei_width[nlabel + 1], kukei_height[nlabel + 1];
  int left_up_x[nlabel + 1];
  int left_up_y[nlabel + 1];
  kukei(labels, nlabel, kukei_y, kukei_x, kukei_width, kukei_height, left_up_x, left_up_y);

  for (int i = 0; i < 3; i++)
  {
    // printf("kukei_width[%d] = %d\n", i, kukei_width[i]);
    //printf("kukei_height[%d] = %d\n", i, kukei_height[i]);
  }
  for (y = 0; y < HEIGHT; y++)
  {
    for (x = 0; x < WIDTH; x++)
    {
      l = labels[y][x];
      nPixels[l]++;
    }
  }
  // 面積
  int max[nlabel + 1];
  int men[nlabel + 1];
  for (int i = 1; i <= nlabel; i++)
  {
    max[i] = 1;
    men[i] = 0;
  }
  for (int i = 1; i <= nlabel; i++)
  {
    for (int l = 1; l <= nlabel; l++)
    {
      if (nPixels[l] > nPixels[max[i]])
      {
        max[i] = l;
      }
    }
    men[i] = nPixels[max[i]];
    nPixels[max[i]] = 0;
  }
  for (int i = 1; i <= nlabel; i++)
  {
    //printf("%d\n", men[i]);
  }
  //printf("ラベル数＝%d\n", nlabel);

  int len;
  double enk[nlabel + 1];
  for (int i = 1; i <= nlabel; i++)
  {
    len = length(dst, kukei_y[i], kukei_x[i]);
    // printf("周囲長=%d\n", len);
    enk[i] = enkeido(len, men[i]);
  }
   for (int i = 1; i <= nlabel; i++)
  {
    //printf("円形度=%f\n", enk[i]);
    } 

  /*指の数を数える*/
  int fing[nlabel + 1];

  for (int i = 1; i <= nlabel; i++)
  {
    fing[i] = cnt_fing(labels, i, kukei_x, kukei_y, kukei_width[i], kukei_height[i]); //
    // printf("%dの指の数%d\n", i, fing[i]);
  }
  int order[nlabel + 1];

  for (int i = 1; i <= nlabel; i++)
  {
    order[i] = i;
  }
  for (int k = 1; k <= nlabel; k++)
  {
    for (int i = 1; i <= nlabel; i++)
    {
      for (int j = 1; j < k; j++)
      {
        if (i == order[j])
        {
          goto SKIP;
        }
      }
      if (left_up_x[order[k]] > left_up_x[i])
      {
        for (int o = k + 1; o <= nlabel; o++)
        {
          order[o] = order[k];
        }
        order[k] = i;
      }
    SKIP:
      printf("");
    }
  }

  for (int i = 1; i <= nlabel; i++){
    printf("order[%d] = %d\n", i, order[i]);
  }

  int hand[nlabel + 1];
  int i;
  int kekka; /* 勝敗パターン判定（数字） */

  for (i = 1; i <= nlabel; i++)
  {
    if (enk[i] > 0.5)
    {
      hand[i] = 0;
    }
    else if (fing[i] >= 4)
    {
      hand[i] = 2;
    }
    else
    {
      hand[i] = 1;
    }
  }
  kekka = is_type(nlabel, hand);

  for (i = 1; i <= nlabel; i++)
  {
    printf("%d %d %d %d %s %s\n", left_up_x[order[i]], left_up_y[order[i]], kukei_width[order[i]], kukei_height[order[i]], G_C_P(hand, order[i]), out_kekka(kekka, hand, order[i]));
  }

  return 0;
}

int cnt_fing(int labelImage[][WIDTH], int label, int kukei_y[], int kukei_x[], int wid, int hei) //縦と横の黒から白に変わる回数を調べる。その最大値を返す。
{
  static int vercnt[WIDTH], sidecnt[HEIGHT], m, n, flag, maxcnt;

  for (m = 0; m < WIDTH; m++)
  {
    vercnt[m] = 0;
  }
  for (n = 0; n < HEIGHT; n++)
  {
    sidecnt[n] = 0;
  }

  for (m = 0; m < WIDTH; m++)
  {
    for (n = 0; n < HEIGHT; n++)
    {
      if (labelImage[n + 1][m] == label && labelImage[n][m] != label)
      {
        //for( m)
        vercnt[m]++;
      }
    }
  }
  for (m = 0; m < HEIGHT; m++)
  {
    for (n = 0; n < WIDTH; n++)
    {
      if (labelImage[m][n + 1] == label && labelImage[m][n] != label)
      {
        sidecnt[m]++;
      }
    }
  }
  maxcnt = vercnt[0];
  if (wid > hei)
  {
    for (m = 0; m < WIDTH; m++)
    {
      if (maxcnt < vercnt[m])
      {
        maxcnt = vercnt[m];
      }
    }
  }
  else
  {
    for (n = 0; n < HEIGHT; n++)
    {
      if (maxcnt < sidecnt[n])
      {
        maxcnt = sidecnt[n];
      }
    }
  }

  return maxcnt;
}

int StackAlloc(IprStack *s, int max)
{
  s->ptr = 0;
  s->data = (IprPoint *)calloc(max, sizeof(IprPoint));
  if (s->data == NULL)
  {
    fprintf(stderr, "スタックの確保に失敗しました.\n");
    s->max = 0;
    return -1;
  }
  s->max = max;
  return 0;
}
void StackFree(IprStack *s)
{
  if (s->data != NULL)
  {
    free(s->data);
    s->max = s->ptr = 0;
  }
}
int StackPush(IprStack *s, IprPoint x)
{
  if (s->ptr >= s->max)
  {
    fprintf(stderr, "スタックが一杯です.\n");
    return -1;
  }
  s->data[s->ptr++] = x;
  return 0;
}
int StackPop(IprStack *s, IprPoint *x)
{
  if (s->ptr <= 0)
  {
    fprintf(stderr, "スタックは空です.\n");
    return -1;
  }
  *x = s->data[--s->ptr];
  return 0;
}

int StackIsEmpty(IprStack *s)
{
  return (s->ptr <= 0);
}
int labeling(uchar binary[][WIDTH], int labelImage[][WIDTH])
{
  int x, y, n, xx, yy;
  IprStack stack;
  StackAlloc(&stack, HEIGHT * WIDTH);
  IprPoint neighbor[8] = {
      {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1}};
  // ラベルとラベル画像の初期化
  int label = 0;
  for (y = 0; y < HEIGHT; y++)
  {
    for (x = 0; x < WIDTH; x++)
    {
      labelImage[y][x] = 0;
    }
  }
  for (y = 0; y < HEIGHT; y++)
  {
    for (x = 0; x < WIDTH; x++)
    {
      if (binary[y][x] != 0 && labelImage[y][x] == 0 /* 二値画像 binary[y][x] が図形で,かつ,
                  ラベルが付いていない */
      )
      {
        label++;
        labelImage[y][x] = label;
        StackPush(&stack, iprPoint(x, y) /* スタックにプッシュ */);
        while (!StackIsEmpty(&stack) /* スタックが空でない */)
        {
          IprPoint p;
          StackPop(&stack, &p /* スタックをポップ */);
          for (n = 0; n < 8; n++)
          {
            xx = p.x + neighbor[n].x;
            yy = p.y + neighbor[n].y;
            if (xx >= 0 && xx < WIDTH &&
                yy >= 0 && yy < HEIGHT &&
                binary[yy][xx] != 0 &&
                labelImage[yy][xx] == 0)
            {
              labelImage[yy][xx] = label;
              /* ラベルを付ける */
              StackPush(&stack, iprPoint(xx, yy) /* スタックにプッシュ*/);
            }
          }
        }
      }
    }
  }

  return label;
}

int kukei(int labelImage[][WIDTH], int label, int kukei_y[], int kukei_x[], int *kukei_width, int *kukei_height, int *left_up_x, int *left_up_y)
{
  int i, x, y;
  int right_down_x[label + 1], right_down_y[label + 1]; //left_up_xは左上のx座標
  for (i = 1; i <= label; i++)                          //全てのラベルでおこなう
  {
    for (x = 0; x < WIDTH; x++)
    {
      for (y = 0; y < HEIGHT; y++)
      {
        if (labelImage[y][x] == i)
        {
          left_up_x[i] = x;
          goto END; //ENDにとぶ
        }
      }
    }
  END: //END

    for (y = 0; y < HEIGHT; y++)
    {
      for (x = 0; x < WIDTH; x++)
      {
        if (labelImage[y][x] == i)
        {
          left_up_y[i] = y;
          goto END1;
        }
      }
    }
  END1:

    for (x = WIDTH - 1; x >= 0; x--)
    {
      for (y = 0; y < HEIGHT; y++)
      {
        if (labelImage[y][x] == i)
        {
          right_down_x[i] = x;
          goto END2;
        }
      }
    }
  END2:

    for (y = HEIGHT - 1; y >= 0; y--)
    {
      for (x = 0; x < WIDTH; x++)
      {
        if (labelImage[y][x] == i)
        {
          right_down_y[i] = y;
          goto END3;
        }
      }
    }
  END3:
    // printf("矩形番号：%3d,左上x座標：%3d,y座標：%3d,右下x座標：%3d,y座標：%3d\n", i, left_up_x[i], left_up_y[i], right_down_x[i], right_down_y[i]);

    kukei_y[i] = left_up_y[i];
    // printf("%d\n", kukei_y[i]);
    kukei_x[i] = left_up_x[i];
    // printf("%d\n", kukei_x[i]);

    kukei_width[i] = right_down_x[i] - left_up_x[i];
    kukei_height[i] = right_down_y[i] - left_up_y[i];
  }
  //矩形出力終わり
  return 0;
}

void ipr_load_pgm(unsigned char image[][WIDTH], const char path[])
{
  char magic_number[2];
  int width, height;
  int max_intensity;
  FILE *fp;
  fp = fopen(path, "rb");
  if (fp == NULL)
  {
    fprintf(stderr, "%s が開けませんでした.\n", path);
    exit(1);
  }
  fscanf(fp, "%c%c", &magic_number[0], &magic_number[1]);
  if (magic_number[0] != 'P' || magic_number[1] != '5')
  {
    fprintf(stderr, "%s はバイナリ型 PGM ではありません.\n", path);
    fclose(fp);
    exit(1);
  }
  fscanf(fp, "%d %d", &width, &height);
  if (width != WIDTH || height != HEIGHT)
  {
    fprintf(stderr, "画像のサイズが異なります.\n");
    fprintf(stderr, " 想定サイズ:WIDTH = %d, HEIGHT = %d\n", WIDTH, HEIGHT);
    fprintf(stderr, " 実サイズ: width = %d, height = %d\n",
            width, height);
    fclose(fp);
    exit(1);
  }
  fscanf(fp, "%d", &max_intensity);
  if (max_intensity != 255)
  {
    fprintf(stderr, "最大階調値が不正な値です(%d).\n", max_intensity);
    fclose(fp);
    exit(1);
  }
  fgetc(fp); // 最大階調値の直後の改行コードを読み捨て
  fread(image, sizeof(unsigned char), HEIGHT * WIDTH, fp);
  fclose(fp);
}
void ipr_save_pgm(unsigned char image[][WIDTH], const char path[])
{
  FILE *fp;
  fp = fopen(path, "wb");
  if (fp == NULL)
  {
    fprintf(stderr, "%s が開けませんでした.\n", path);
    exit(1);
  }
  fprintf(fp, "P5\n");
  fprintf(fp, "%d %d\n", WIDTH, HEIGHT);
  fprintf(fp, "255\n");
  fwrite(image, sizeof(unsigned char), HEIGHT * WIDTH, fp);
  fclose(fp);
}
void ipr_load_ppm(uchar image[][WIDTH][3], const char path[])
{
  char magic_number[2];
  int width, height;
  int max_intensity;
  FILE *fp;
  fp = fopen(path, "rb");
  if (fp == NULL)
  {
    fprintf(stderr, "%s が開けませんでした.\n", path);
    exit(1);
  }
  fscanf(fp, "%c%c", &magic_number[0], &magic_number[1]);
  if (magic_number[0] != 'P' || magic_number[1] != '6')
  {
    fprintf(stderr, "%s はバイナリ型 PPM ではありません.\n", path);
    fclose(fp);
    exit(1);
  }
  fscanf(fp, "%d %d", &width, &height);
  if (width != WIDTH || height != HEIGHT)
  {
    fprintf(stderr, "画像のサイズが異なります.\n");
    fprintf(stderr, " 想定サイズ:WIDTH = %d, HEIGHT = %d\n", WIDTH, HEIGHT);
    fprintf(stderr, " 実サイズ: width = %d, height = %d\n",
            width, height);
    fclose(fp);
    exit(1);
  }
  fscanf(fp, "%d", &max_intensity);
  if (max_intensity != 255)
  {
    fprintf(stderr, "最大階調値が不正な値です(%d).\n", max_intensity);
    fclose(fp);
    exit(1);
  }
  fgetc(fp); // 最大階調値の直後の改行コードを読み捨て
  fread(image, sizeof(uchar), HEIGHT * WIDTH * 3, fp);
  fclose(fp);
}
int max3(unsigned char r, unsigned char g, unsigned char b)
{
  int max;
  if (r > g)
  {
    if (r > b)
    {
      max = r;
    }
    else
    {
      max = b;
    }
  }
  else
  {
    if (g > b)
    {
      max = g;
    }
    else
    {
      max = b;
    }
  }
  return max;
}
int min3(unsigned char r, unsigned char g, unsigned char b)
{
  int min;
  if (r < g)
  {
    if (r < b)
    {
      min = r;
    }
    else
    {
      min = b;
    }
  }
  else
  {
    if (g < b)
    {
      min = g;
    }
    else
    {
      min = b;
    }
  }
  return min;
}
void rgb2hsv(unsigned char rgb[][WIDTH][3], int hsv[][WIDTH][3])
{
  int Vmin, Vmax;
  int m, n;
  double den;
  for (m = 0; m < HEIGHT; m++)
  {
    for (n = 0; n < WIDTH; n++)
    {
      Vmax = max3(rgb[m][n][0], rgb[m][n][1], rgb[m][n][2]);
      Vmin = min3(rgb[m][n][0], rgb[m][n][1], rgb[m][n][2]);
      // 明度の計算
      hsv[m][n][2] = 100 * Vmax / 255.0;
      // 彩度の計算
      if (Vmax == 0)
      {
        hsv[m][n][1] = 0;
      }
      else
      {
        hsv[m][n][1] = 100.0 * (Vmax - Vmin) / (double)Vmax;
      }
      // 色相の計算
      if (Vmax == Vmin)
      {
        hsv[m][n][0] = -1;
      }
      else
      {
        den = Vmax - Vmin;
        if (Vmax == rgb[m][n][0])
        {
          hsv[m][n][0] = 60.0 * (rgb[m][n][1] - rgb[m][n][2]) / den;
        }
        else if (Vmax == rgb[m][n][1])
        {
          hsv[m][n][0] = 60.0 * (rgb[m][n][2] - rgb[m][n][0]) / den + 120;
        }
        else
        {
          hsv[m][n][0] = 60.0 * (rgb[m][n][0] - rgb[m][n][2]) / den + 240;
        }
        // 0 〜 360 に収まるように調整
        if (hsv[m][n][0] < 0)
        {
          hsv[m][n][0] += 360;
        }
        if (hsv[m][n][0] >= 360)
        {
          hsv[m][n][0] -= 360;
        }
      }
    }
  }
}
void copy_image(uchar src[][WIDTH], uchar dst[][WIDTH])
{
  for (int y = 0; y < HEIGHT; y++)
  {
    for (int x = 0; x < WIDTH; x++)
    {
      dst[y][x] = src[y][x];
    }
  }
}

void dilate(uchar src[][WIDTH], uchar dst[][WIDTH], int iteration)
{
  uchar tmp[HEIGHT][WIDTH];
  int i;
  copy_image(src, tmp);
  for (i = 0; i < iteration; i++)
  {
    /////
    // 四隅の処理
    /////
    // (0, 0)
    if (tmp[0][0] != 0 || tmp[0][1] != 0 ||
        tmp[1][0] != 0 || tmp[1][1] != 0)
    {
      dst[0][0] = 255;
    }
    else
    {
      dst[0][0] = 0;
    }
    // (0, WIDTH - 1)
    if (tmp[0][WIDTH - 2] != 0 || tmp[0][WIDTH - 1] != 0 ||
        tmp[1][WIDTH - 2] != 0 || tmp[1][WIDTH - 1] != 0)
    {
      dst[0][WIDTH - 1] = 255;
    }
    else
    {
      dst[0][WIDTH - 1] = 0;
    }
    // (HEIGHT - 1, 0)
    if (tmp[HEIGHT - 2][0] != 0 || tmp[HEIGHT - 2][1] != 0 ||
        tmp[HEIGHT - 1][0] != 0 || tmp[HEIGHT - 1][1] != 0)
    {
      dst[HEIGHT - 1][0] = 255;
    }
    else
    {
      dst[HEIGHT - 1][WIDTH - 1] = 0;
    }
    // (HEIGHT - 1, WIDTH - 1)
    if (tmp[HEIGHT - 2][WIDTH - 2] != 0 || tmp[HEIGHT - 2][WIDTH - 1] != 0 ||
        tmp[HEIGHT - 1][WIDTH - 2] != 0 || tmp[HEIGHT - 1][WIDTH - 1] != 0)
    {
      dst[HEIGHT - 1][WIDTH - 1] = 255;
    }
    else
    {
      dst[HEIGHT - 1][WIDTH - 1] = 0;
    }
    /////
    // 四隅の処理はここまで
    /////
    /////
    // 四隅を除く端の処理
    /////
    // 上端 (0, 1) -- (0, WIDTN - 2)
    for (int x = 1; x < WIDTH - 2; x++)
    {
      if (tmp[0][x - 1] != 0 || tmp[0][x] != 0 || tmp[0][x + 1] != 0 ||
          tmp[1][x - 1] != 0 || tmp[1][x] != 0 || tmp[1][x + 1] != 0)
      {
        dst[0][x] = 255;
      }
      else
      {
        dst[0][x] = 0;
      }
    }
    // 下端 (HEIGHT - 1, 1) -- (HEIGHT - 1, WIDTH - 2)
    for (int x = 1; x < WIDTH - 2; x++)
    {
      if (tmp[HEIGHT - 2][x - 1] != 0 || tmp[HEIGHT - 2][x] != 0 || tmp[HEIGHT - 2][x + 1] != 0 ||
          tmp[HEIGHT - 1][x - 1] != 0 || tmp[HEIGHT - 1][x] != 0 || tmp[HEIGHT - 1][x + 1] != 0)
      {
        dst[HEIGHT - 1][x] = 255;
      }
      else
      {
        dst[HEIGHT - 1][x] = 0;
      }
    }
    // 左端 (1, 0) -- (HEIGHT - 2, 0)
    for (int y = 1; y < HEIGHT - 2; y++)
    {
      if (tmp[y - 1][0] != 0 || tmp[y - 1][1] != 0 ||
          tmp[y][0] != 0 || tmp[y][1] != 0 ||
          tmp[y + 1][0] != 0 || tmp[y + 1][1] != 0)
      {
        dst[y][0] = 255;
      }
      else
      {
        dst[y][0] = 0;
      }
    }
    // 右端 (1, WIDTH - 1) -- (HEIGHT - 2, WIDTH - 1)
    for (int y = 1; y < HEIGHT - 2; y++)
    {
      if (tmp[y - 1][WIDTH - 2] != 0 || tmp[y - 1][WIDTH - 1] != 0 ||
          tmp[y][WIDTH - 2] != 0 || tmp[y][WIDTH - 1] != 0 ||
          tmp[y + 1][WIDTH - 2] != 0 || tmp[y + 1][WIDTH - 1] != 0)
      {
        dst[y][WIDTH - 1] = 255;
      }
      else
      {
        dst[y][WIDTH - 1] = 0;
      }
    } /////
    // 端の処理はここまで
    /////
    for (int y = 1; y < HEIGHT - 1; y++)
    {
      for (int x = 1; x < WIDTH - 1; x++)
      {
        if (tmp[y - 1][x - 1] != 0 || tmp[y - 1][x] != 0 || tmp[y - 1][x + 1] != 0 ||
            tmp[y][x - 1] != 0 || tmp[y][x] != 0 || tmp[y][x + 1] != 0 ||
            tmp[y + 1][x - 1] != 0 || tmp[y + 1][x] != 0 || tmp[y + 1][x + 1] != 0)
        {
          dst[y][x] = 255;
        }
        else
        {
          dst[y][x] = 0;
        }
      }
    }
    /*
      if(i > 1){
      for (int y = 1; y < HEIGHT - 1; y++) {
      for (int x = 1; x < WIDTH - 1; x++) {
      for(int j = 0;j < i;j++){
      //3x3 より外側
      if(tmp[y - i][x] == 0||tmp[y][x - i] == 0||tmp[y][x + i] == 0||tmp[y + i][x]){
      dst[y][x] = 0;
      }
      else{
      dst[y][x] = 255;
      }
      }
      }
      }
      }*/
    copy_image(dst, tmp);
  }
}
void erode(uchar src[][WIDTH], uchar dst[][WIDTH], int iteration)
{
  uchar tmp[HEIGHT][WIDTH];
  int i;
  copy_image(src, tmp);
  for (i = 0; i < iteration; i++)
  {
    /////
    // 四隅の処理
    /////
    // (0, 0)
    if (tmp[0][0] == 0 || tmp[0][1] == 0 ||
        tmp[1][0] == 0 || tmp[1][1] == 0)
    {
      dst[0][0] = 0;
    }
    else
    {
      dst[0][0] = 255;
    }
    // (0, WIDTH - 1)
    if (tmp[0][WIDTH - 2] == 0 || tmp[0][WIDTH - 1] == 0 ||
        tmp[1][WIDTH - 2] == 0 || tmp[1][WIDTH - 1] == 0)
    {
      dst[0][WIDTH - 1] = 0;
    }
    else
    {
      dst[0][WIDTH - 1] = 255;
    }
    // (HEIGHT - 1, 0)
    if (tmp[HEIGHT - 2][0] == 0 || tmp[HEIGHT - 2][1] == 0 ||
        tmp[HEIGHT - 1][0] == 0 || tmp[HEIGHT - 1][1] == 0)
    {
      dst[HEIGHT - 1][0] = 0;
    }
    else
    {
      dst[HEIGHT - 1][WIDTH - 1] = 255;
    } // (HEIGHT - 1, WIDTH - 1)
    if (tmp[HEIGHT - 2][WIDTH - 2] == 0 || tmp[HEIGHT - 2][WIDTH - 1] == 0 ||
        tmp[HEIGHT - 1][WIDTH - 2] == 0 || tmp[HEIGHT - 1][WIDTH - 1] == 0)
    {
      dst[HEIGHT - 1][WIDTH - 1] = 0;
    }
    else
    {
      dst[HEIGHT - 1][WIDTH - 1] = 255;
    }
    /////
    // 四隅の処理はここまで
    /////
    /////
    // 四隅を除く端の処理
    /////
    // 上端 (0, 1) -- (0, WIDTN - 2)
    for (int x = 1; x < WIDTH - 2; x++)
    {
      if (tmp[0][x - 1] == 0 || tmp[0][x] == 0 || tmp[0][x + 1] == 0 ||
          tmp[1][x - 1] == 0 || tmp[1][x] == 0 || tmp[1][x + 1] == 0)
      {
        dst[0][x] = 0;
      }
      else
      {
        dst[0][x] = 255;
      }
    }
    // 下端 (HEIGHT - 1, 1) -- (HEIGHT - 1, WIDTH - 2)
    for (int x = 1; x < WIDTH - 2; x++)
    {
      if (tmp[HEIGHT - 2][x - 1] == 0 || tmp[HEIGHT - 2][x] == 0 || tmp[HEIGHT - 2][x + 1] == 0 ||
          tmp[HEIGHT - 1][x - 1] == 0 || tmp[HEIGHT - 1][x] == 0 || tmp[HEIGHT - 1][x + 1] == 0)
      {
        dst[HEIGHT - 1][x] = 0;
      }
      else
      {
        dst[HEIGHT - 1][x] = 255;
      }
    }
    // 左端 (1, 0) -- (HEIGHT - 2, 0)
    for (int y = 1; y < HEIGHT - 2; y++)
    {
      if (tmp[y - 1][0] == 0 || tmp[y - 1][1] == 0 ||
          tmp[y][0] == 0 || tmp[y][1] == 0 ||
          tmp[y + 1][0] == 0 || tmp[y + 1][1] == 0)
      {
        dst[y][0] = 0;
      }
      else
      {
        dst[y][0] = 255;
      }
    }
    // 右端 (1, WIDTH - 1) -- (HEIGHT - 2, WIDTH - 1)
    for (int y = 1; y < HEIGHT - 2; y++)
    {
      if (tmp[y - 1][WIDTH - 2] == 0 || tmp[y - 1][WIDTH - 1] == 0 ||
          tmp[y][WIDTH - 2] == 0 || tmp[y][WIDTH - 1] == 0 ||
          tmp[y + 1][WIDTH - 2] == 0 || tmp[y + 1][WIDTH - 1] == 0)
      {
        dst[y][WIDTH - 1] = 0;
      }
      else
      {
        dst[y][WIDTH - 1] = 255;
      }
    }
    /////
    // 端の処理はここまで
    /////
    for (int y = 1; y < HEIGHT - 1; y++)
    {
      for (int x = 1; x < WIDTH - 1; x++)
      {
        if (tmp[y - 1][x - 1] == 0 || tmp[y - 1][x] == 0 || tmp[y - 1][x + 1] == 0 ||
            tmp[y][x - 1] == 0 || tmp[y][x] == 0 || tmp[y][x + 1] == 0 ||
            tmp[y + 1][x - 1] == 0 || tmp[y + 1][x] == 0 || tmp[y + 1][x + 1] == 0)
        {
          dst[y][x] = 0;
        }
        else
        {
          dst[y][x] = 255;
        }
      }
    }
  }
  /*
    if(i > 1){
    for (int y = 1; y < HEIGHT - 1; y++) {
    for (int x = 1; x < WIDTH - 1; x++) {
    for(int j = 0;j < i;j++){
    //3x3 より外側
    if(tmp[y - i][x] == 0||tmp[y][x - i] == 0||tmp[y][x + i] == 0||tmp[y + i][x]){
    dst[y][x] = 0;
    }
    else{
    dst[y][x] = 255;
    }
    }
    }
    }
    }*/
  copy_image(dst, tmp);
}
void opening(uchar src[][WIDTH], uchar dst[][WIDTH], int iteration)
{
  uchar tmp[HEIGHT][WIDTH] = {{0}};
  for (int i = 0; i < 3; i++)
  {
    erode(src, tmp, iteration + i);
    dilate(tmp, dst, iteration + i);
  }
}
void closing(uchar src[][WIDTH], uchar dst[][WIDTH], int iteration)
{
  uchar tmp[HEIGHT][WIDTH] = {{0}};
  for (int i = 0; i < 3; i++)
  {
    dilate(src, tmp, iteration + i);
    erode(tmp, dst, iteration + i);
  }
}

int length(uchar image[][WIDTH], int m, int n)
{
  int length = 0;
  int i, j;
  int rable[HEIGHT][WIDTH];
  int direction = RIGHT;
  for (i = 0; i < HEIGHT; i++)
  {
    for (j = 0; j < WIDTH; j++)
    {
      rable[i][j] = 0;
    }
  }
  while (image[m][n] == 0)
  {
    if (image[m][n] == 0)
    {
      m++;
    }
  }
  while (rable[m][n] == 0)
  {
    rable[m][n] = 1;
    length++;
    // printf("direction=%d\n", direction);
    if (direction == RIGHT)
    {
      if (image[m - 1][n])
      {
        m--;
        direction = UP;
        continue;
      }
      if (image[m][n + 1])
      {
        n++;
        direction = RIGHT;
        continue;
      }
      if (image[m + 1][n])
      {
        m++;
        direction = DOWN;
        continue;
      }
    }
    if (direction == UP)
    {
      if (image[m][n - 1])
      {
        n--;
        direction = LEFT;
        continue;
      }
      if (image[m - 1][n])
      {
        m--;
        direction = UP;
        continue;
      }
      if (image[m][n + 1])
      {
        n++;
        direction = RIGHT;
        continue;
      }
    }
    if (direction == LEFT)
    {
      if (image[m + 1][n])
      {
        m++;
        direction = DOWN;
        continue;
      }
      if (image[m][n - 1])
      {
        n--;
        direction = LEFT;
        continue;
      }
      if (image[m - 1][n])
      {
        m--;
        direction = UP;
        continue;
      }
    }
    if (direction == DOWN)
    {
      if (image[m][n + 1])
      {
        n++;
        direction = RIGHT;
        continue;
      }
      if (image[m + 1][n])
      {
        m++;
        direction = DOWN;
        continue;
      }
      if (image[m][n - 1])
      {
        n--;
        direction = LEFT;
        continue;
      }
    }
  } //while
  return length;
}
double enkeido(int length, int npixel)
{
  double enkeido;
  enkeido = M_PI * 4 * npixel / (double)(length * length);
  return enkeido;
}



