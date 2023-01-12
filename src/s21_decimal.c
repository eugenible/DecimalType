#include "s21_decimal.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int getBit(s21_decimal d, int i) {
  unsigned int mask = 1u << (i % 32);
  return d.bits[i / 32] & mask;
}

int getBitFloat(float a, int i) {
  unsigned int mask = 1u << i;
  unsigned int b = *((unsigned int *)&a);
  return (b & mask);
}

int getBitInt(int a, int i) {
  unsigned int mask = 1u << i;
  return (a & mask);
}

void setBit(s21_decimal *d, int i) {
  unsigned int mask = 1u << (i % 32);
  d->bits[i / 32] |= mask;
}

void cleanBit(s21_decimal *d, int i) {
  unsigned int mask = 1u << (i % 32);
  d->bits[i / 32] &= ~mask;
}

int findFirstNonZeroBit(s21_decimal dec, int start_index) {
  int index = -1;
  for (int i = start_index; i > -1; i--) {
    if (!!getBit(dec, i)) {
      index = i;
      break;
    }
  }
  return index;
}

void switchSign(s21_decimal *dec) {
  unsigned int mask = 1u << 31;
  dec->bits[3] ^= mask;
}

int powTwo(int power) {
  int result = 1;
  for (int i = 0; i < power; i++) {
    result *= 2;
  }
  return result;
}

float powTwoFloat(int power) {
  float result = 1.0;
  for (int i = 0; i < power; i++) {
    result *= 2;
  }
  for (int i = power; i < 0; i++) {
    result /= 2;
  }
  return result;
}

void set_scale(s21_decimal *a, int scale) {
  for (int i = 112, j = 0; i < 120; i++, j++) {
    cleanBit(a, i);
    if (!!getBitInt(scale, j)) setBit(a, i);
  }
}

void normalize(s21_decimal *a) {
  int counter = 0;
  int oldScale = getDegree(*a);
  while (!sign(*a) && (counter < oldScale)) {
    *a = s21_div_ten(*a);
    counter++;
  }
  int newScale = oldScale - counter;
  set_scale(a, newScale);
}

void equalize(s21_decimal *a, s21_decimal *b, int *res_scale) {
  int new_scale = 0;
  if (getDegree(*a) > getDegree(*b)) {
    new_scale = getDegree(*a) - getDegree(*b);
    if (res_scale) *res_scale = getDegree(*a);
    decimal_pow(b, new_scale);
  } else {
    new_scale = getDegree(*b) - getDegree(*a);
    if (res_scale) *res_scale = getDegree(*b);
    decimal_pow(a, new_scale);
  }
}

int check_decimal(s21_decimal src) {
  int flag = 0;
  for (int i = 96; i < 112; i++) {
    if ((!!getBit(src, i)) == 1) flag = 1;
  }
  for (int i = 121; i < 127; i++) {
    if ((!!getBit(src, i)) == 1) flag = 1;
  }
  return flag;
}

int check_bits_zero_left(s21_decimal dec) {
  int flag = 0;
  for (int i = 32; i <= 95; i++) {
    if (!!getBit(dec, i)) {
      flag = 1;
      break;
    }
  }
  return flag;
}

int getDegree(s21_decimal dec) {
  int res = 0;
  for (int i = 112; i < 120; i++) {
    res += !!getBit(dec, i) * powTwo(i - 112);
  }
  return res;
}

int getDegreeFloat(float a) {
  int res = 0;
  for (int i = 23; i < 31; i++) {
    res += !!getBitFloat(a, i) * powTwo(i - 23);
  }
  return res;
}

int checkIfZero(s21_decimal a) {
  int flag = 1;
  for (int i = 0; i < 96; i++) {
    if (!!getBit(a, i)) {
      flag = 0;
      break;
    }
  }
  return flag;
}

s21_decimal invert(s21_decimal a, int max_bit) {
  s21_decimal res = {{0u, 0u, 0u, 0u}, s21_NORMAL_VALUE};
  s21_decimal one = {{0b1, 0u, 0u, 0u}, s21_NORMAL_VALUE};
  for (int i = 0; i <= max_bit; i++) {
    if (!getBit(a, i)) setBit(&res, i);
  }
  res = s21_add(res, one);
  return res;
}

int s21_from_decimal_to_float(s21_decimal src, float *dst) {
  int flag = 0;
  float tmp = 0;
  float degree = (float)getDegree(src) * (-1);
  if ((check_decimal(src)) || (src.value_type != s21_NORMAL_VALUE)) {
    flag = 1;
  } else {
    for (int i = 0; i < 96; i++) {
      if (!!getBit(src, i)) tmp += powTwoFloat(i);
    }
    *dst = (float)tmp * pow(10, degree);
    if (!!getBit(src, 127)) *dst *= (-1);
  }
  return flag;
}

int check_degree(float x) {
  char str[100] = {};
  char *istr;
  char copy[100];
  char copy2[100];
  int flag = 0;
  int degree = 0;
  int counter = 0;
  long long int check = 0;
  double mm = 0;
  char *help = NULL;
  snprintf(str, sizeof(str), "%f", x);
  snprintf(copy, sizeof(copy), "%s", str);
  istr = strtok_r(copy, ".", &help);
  if (istr != NULL) {
    istr = strtok_r(NULL, ".", &help);
  }
  snprintf(copy2, sizeof(copy2), "%s", istr);
  while (*istr) {
    if (*istr == 48) {
      counter++;
    } else {
      degree += 1;
      degree += counter;
      counter = 0;
    }
    istr++;
  }
  return degree;
}

int s21_from_float_to_decimal(float src, s21_decimal *dst) {
  for (int i = 0; i < 4; i++) {
    dst->bits[i] = 0u;
  }
  dst->value_type = s21_NORMAL_VALUE;

  int flag = 0, iterator = 0, count = 0, not_empty_mant = 0;

  for (int i = 0; i < 23; i++) {
    if (!!(getBitFloat(src, i))) not_empty_mant = 1;
  }

  if ((getDegreeFloat(src) == 255)) {
    if (not_empty_mant) {
      dst->value_type = s21_NAN;
    } else {
      if (!!getBitFloat(src, 31)) dst->value_type = s21_NEGATIVE_INFINITY;
      if (!getBitFloat(src, 31)) dst->value_type = s21_INFINITY;
    }
    flag = 1;
  }

  if (!flag) {  // НАДО !!! мб бесконечность, но будет сега!!!
    iterator = check_degree(src);
    if (iterator != 0) src *= pow(10, iterator);
    int j = 0;
    for (int i = 112; i < 120; i++) {
      if (!!getBitInt(iterator, j)) {
        setBit(dst, i);
      }
      j++;
    }

    int power = getDegreeFloat(src) - 127;
    int power_power = power;
    if (power > 95) {
      if (!!getBitFloat(src, 31))
        dst->value_type = s21_NEGATIVE_INFINITY;
      else
        dst->value_type = s21_INFINITY;
      flag = 1;
      power_power = power - power % 95;
    }
    setBit(dst, power_power);
    for (int i = 0; i < power; i++) {
      int bit = !!getBitFloat(src, 22 - i);
      if (bit) setBit(dst, power_power - i - 1);
    }
  }
  if (!!getBitFloat(src, 31)) setBit(dst, 127);
  return flag;
}

int s21_from_decimal_to_int(s21_decimal src, int *dst) {
  int flag = 0;
  if ((check_decimal(src)) || (src.value_type != s21_NORMAL_VALUE)) {
    flag = 1;
  } else if (check_bits_zero_left(src)) {
    flag = 1;
  } else {
    int degree = getDegree(src);
    *dst = src.bits[0];
    *dst /= pow(10, degree);
    if ((!!getBit(src, 127))) {
      *dst *= (-1);
    }
  }
  return flag;
}

int s21_from_int_to_decimal(int src, s21_decimal *dst) {
  int flag = 0;
  for (int i = 0; i < 4; i++) {
    dst->bits[i] = 0u;
  }
  if (src < 0) {
    src = src * (-1);
    setBit(dst, 127);
  }
  for (int i = 0; i < 31; i++) {
    if (!!getBitInt(src, i)) setBit(dst, i);
  }
  dst->value_type = s21_NORMAL_VALUE;
  return flag;
}

s21_decimal s21_add_int(s21_decimal a, s21_decimal b) {
  s21_decimal res = {{0u, 0u, 0u, 0u}, s21_NORMAL_VALUE};
  int tmp = 0, first = 0, second = 0;
  for (int i = 0; i < 96; i++) {
    first = !!getBit(a, i);
    second = !!getBit(b, i);
    if (first && second) {
      if (tmp) {
        setBit(&res, i);
      }
      tmp = 1;
    } else if (first || second) {
      if (!tmp) {
        setBit(&res, i);
      }
    }
    if (!first && !second) {
      if (tmp) {
        setBit(&res, i);
      }
      tmp = 0;
    }
    if ((tmp == 1) && (i == 95)) {
      res.value_type = s21_INFINITY;
    }
  }
  return res;
}

void shift(s21_decimal *a, int shift) {
  s21_decimal mask = {{0u, 0u, 0u, 0u}, s21_NORMAL_VALUE};
  for (int i = 0; i < 96; i++) {
    if (((shift + i) <= 95) && (!!getBit(*a, i))) setBit(&mask, i + shift);
  }
  for (int i = 0; i < 3; i++) {
    a->bits[i] = 0u;
  }
  for (int i = 0; i < 96; i++) {
    if (!!getBit(mask, i)) setBit(a, i);
  }
}

s21_decimal s21_mul_equal(s21_decimal a, s21_decimal b) {
  s21_decimal res = {{0u, 0u, 0u, 0u}, s21_NORMAL_VALUE};
  s21_decimal tmp = {{0u, 0u, 0u, 0u}, s21_NORMAL_VALUE};
  int new_scale = getDegree(a) + getDegree(b), first = 0, second = 0;
  for (int i = 0; i < 96; i++) {
    tmp = a;
    shift(&tmp, i);
    second = !!getBit(b, i);
    if (second) {
      res = s21_add_int(res, tmp);
    }
  }
  set_scale(&res, new_scale);
  return res;
}

s21_decimal s21_mul(s21_decimal a, s21_decimal b) {
  s21_decimal res = {{0u, 0u, 0u, 0u}, s21_NORMAL_VALUE};
  s21_decimal tmp = {{0u, 0u, 0u, 0u}, s21_NORMAL_VALUE};
  int new_scale = getDegree(a) + getDegree(b), first = 0, second = 0;
  for (int i = 0; i < 96; i++) {
    tmp = a;
    shift(&tmp, i);
    second = !!getBit(b, i);
    if (second) {
      res = s21_add_int(res, tmp);
    }
  }
  set_scale(&res, new_scale);
  if ((!getBit(a, 127)) ^ (!getBit(b, 127))) setBit(&res, 127);

  normal_verbose_mul(a, b, &res);

  normalize(&res);
  return res;
}

void decimal_pow(s21_decimal *a, int power) {
  s21_decimal tmp = {{0b1010, 0u, 0u, 0u}, s21_NORMAL_VALUE};
  while (power > 0) {
    *a = s21_mul_equal(*a, tmp);
    power--;
  }
}

// 0 normal 1 +inf 2 -inf 3 nan
void normal_verbose_add(s21_decimal a, s21_decimal b, s21_decimal *res) {
  if (a.value_type == 1 || b.value_type == 1) {  // a = +inf || b = +inf
    res->value_type = 1;
  }
  if (a.value_type == 2 || b.value_type == 2) {  // a = -inf || b = -inf
    res->value_type = 2;
  }
  if (a.value_type == 3 || b.value_type == 3) {  // a = nan || b = nan
    res->value_type = 3;
  }
  if (a.value_type == 1 && b.value_type == 2)
    res->value_type = 3;  // a = +inf && b = -inf
  if (a.value_type == 1 && b.value_type == 3)
    res->value_type = 3;  // a = +inf && b = nan
  if (a.value_type == 2 && b.value_type == 1)
    res->value_type = 3;  // a = -inf && b = +inf
  if (a.value_type == 2 && b.value_type == 3)
    res->value_type = 3;  // a = -inf && b = nan
}

// 0 normal 1 +inf 2 -inf 3 nan
void normal_verbose_sub(s21_decimal a, s21_decimal b, s21_decimal *res) {
  if (a.value_type == 1 || b.value_type == 1) {  // a = +inf || b = +inf
    res->value_type = 3;
    if (a.value_type == 0 || b.value_type == 0) res->value_type = 1;
  }
  if (a.value_type == 2 || b.value_type == 2) {  // a = -inf || b = -inf
    res->value_type = 3;
    if (a.value_type == 0 || b.value_type == 0) res->value_type = 2;
  }
  if (a.value_type == 3 || b.value_type == 3) {  // a = nan || b = nan
    res->value_type = 3;
  }
  if (a.value_type == 1 && b.value_type == 2)
    res->value_type = 3;  // a = +inf && b = -inf
  if (a.value_type == 1 && b.value_type == 3)
    res->value_type = 3;  // a = +inf && b = nan
  if (a.value_type == 2 && b.value_type == 3)
    res->value_type = 3;  // a = -inf && b = nan
  if (a.value_type == 2 && b.value_type == 1)
    res->value_type = 2;  // a = -inf && b = +inf == -inf
}

void normal_verbose_mul(s21_decimal a, s21_decimal b, s21_decimal *res) {
  s21_decimal null = {{0u, 0u, 0u, 0u}, s21_NORMAL_VALUE};

  if (a.value_type == 1 || b.value_type == 1) {  // a = +inf || b = +inf
    res->value_type = 1;
    if (a.value_type == 0 || b.value_type == 0) res->value_type = 1;
  }
  if (a.value_type == 2 || b.value_type == 2) {  // a = -inf || b = -inf
    res->value_type = 1;
    if (a.value_type == 0 || b.value_type == 0) res->value_type = 2;
  }
  if (a.value_type == 3 || b.value_type == 3) {  // a = nan || b = nan
    res->value_type = 3;
  }
  if (a.value_type == 1 && b.value_type == 2)
    res->value_type = 2;  // a = +inf && b = -inf
  if (a.value_type == 1 && b.value_type == 3)
    res->value_type = 3;  // a = +inf && b = nan
  if (a.value_type == 2 && b.value_type == 3)
    res->value_type = 3;  // a = -inf && b = nan
  if (a.value_type == 2 && b.value_type == 1)
    res->value_type = 2;  // a = -inf && b = +inf == -inf

  if (((!s21_is_equal(a, null) && a.value_type == 0) &&
       (b.value_type == 1 || b.value_type == 2)) ||
      ((!s21_is_equal(b, null) && b.value_type == 0) &&
       (a.value_type == 1 || a.value_type == 2)))
    res->value_type = 3;
}

void normal_verbose_div(s21_decimal a, s21_decimal b, s21_decimal *res) {
  s21_decimal null = {{0u, 0u, 0u, 0u}, s21_NORMAL_VALUE};

  if (a.value_type == 1 && b.value_type == 1) res->value_type = 3;
  if (a.value_type == 2 && b.value_type == 2) res->value_type = 3;

  if (a.value_type == 1 && b.value_type == 0) res->value_type = 1;
  if (b.value_type == 1 && a.value_type == 0) {
    res->value_type = 0;
    res->bits[0] = 0;
    res->bits[1] = 0;
    res->bits[2] = 0;
  }
  if (a.value_type == 2 && b.value_type == 0) res->value_type = 2;
  if (b.value_type == 2 && a.value_type == 0) {
    res->value_type = 0;
    res->bits[0] = 0;
    res->bits[1] = 0;
    res->bits[2] = 0;
  }

  if (a.value_type == 3 || b.value_type == 3) {  // a = nan || b = nan
    res->value_type = 3;
  }
  if (a.value_type == 1 && b.value_type == 2)
    res->value_type = 3;  // a = +inf && b = -inf
  if (a.value_type == 1 && b.value_type == 3)
    res->value_type = 3;  // a = +inf && b = nan
  if (a.value_type == 2 && b.value_type == 3)
    res->value_type = 3;  // a = -inf && b = nan
  if (a.value_type == 2 && b.value_type == 1)
    res->value_type = 3;  // a = -inf && b = +inf == -inf

  if (!s21_is_equal(a, null) && a.value_type == 0 && !s21_is_equal(b, null) &&
      b.value_type == 0)
    res->value_type = 3;

  if ((!s21_is_equal(b, null) && b.value_type == 0) &&
      (a.value_type == 1 || a.value_type == 2))
    res->value_type = 3;

  if (!s21_is_equal(b, null) && (b.value_type == 0) && (a.value_type == 0) &&
      s21_is_equal(a, null)) {
    if (!!getBit(a, 127)) {
      res->value_type = 2;
    } else {
      res->value_type = 1;
    }
  }
}

void normal_verbose_mod(s21_decimal a, s21_decimal b, s21_decimal *res) {
  if (a.value_type == 3 || b.value_type == 3) res->value_type = 3;
  if (a.value_type == 0 && (b.value_type == 1 || b.value_type == 2)) {
    *res = a;
    res->value_type = 0;
  }
  if (b.value_type == 0 && (a.value_type == 1 || a.value_type == 2)) {
    res->value_type = 3;
  }
  if (a.value_type == 1 && b.value_type == 2)
    res->value_type = 3; 
  if (a.value_type == 1 && b.value_type == 3)
    res->value_type = 3;
  if (a.value_type == 2 && b.value_type == 3)
    res->value_type = 3;
  if (a.value_type == 2 && b.value_type == 1)
    res->value_type = 3;
}

void normal_verbose_less(s21_decimal a, s21_decimal b, int *flag) {
  if (a.value_type != 0 || b.value_type != 0) *flag = 1;
  if (b.value_type == 1 && (a.value_type == 0 || a.value_type == 2)) *flag = 0;
  if (b.value_type == 0 && a.value_type == 2) *flag = 0;  // a = +inf, b = n
}

void normal_verbose_equal(s21_decimal a, s21_decimal b, int *flag) {
  if (a.value_type != 0 || b.value_type != 0) *flag = 1;
}

void normal_verbose_less_or_equal(s21_decimal a, s21_decimal b, int *flag) {
  if (a.value_type != 0 || b.value_type != 0) *flag = 1;
  if (b.value_type == 1 && (a.value_type == 0 || a.value_type == 2)) *flag = 0;
  if (b.value_type == 0 && a.value_type == 2) *flag = 0;
}

s21_decimal s21_add(s21_decimal a, s21_decimal b) {
  int flag = 1;
  s21_decimal res = {{0u, 0u, 0u, 0u}, s21_NORMAL_VALUE};

  if (!!getBit(a, 127) && !!getBit(b, 127)) {  // (a < 0) && (b < 0)
    switchSign(&a);
    switchSign(&b);
    res = s21_add(a, b);
    switchSign(&res);
    flag = 0;
  } else if (!getBit(a, 127) && !!getBit(b, 127)) {  // (a > 0) && (b < 0)
    switchSign(&b);
    res = s21_sub(a, b);
    flag = 0;
  } else if (!!getBit(a, 127) && !getBit(b, 127)) {  // (a < 0) && (b > 0)
    switchSign(&a);
    res = s21_sub(b, a);
    flag = 0;
  }

  if (flag) {
    int res_scale = 0;
    equalize(&a, &b, &res_scale);
    res = s21_add_int(a, b);
    set_scale(&res, res_scale);
  }

  if (!!getBit(res, 127) && res.value_type == 1) res.value_type = 2;
  normal_verbose_add(a, b, &res);

  normalize(&res);
  return res;
}

// Если возвращает 0, то делится без остатка, если 1, то делится с остатком
int sign(s21_decimal a) {
  int flag = 0;
  int mas_res[48] = {};
  int odd = 0, even = 0;
  if (!!getBit(a, 0)) flag = 1;
  for (int i = 95, j = 0; i > 0 && !flag; i -= 2, j++) {
    if (!getBit(a, i) && !getBit(a, i - 1)) mas_res[j] = 0b00;
    if (!getBit(a, i) && !!getBit(a, i - 1)) mas_res[j] = 0b01;
    if (!!getBit(a, i) && !getBit(a, i - 1)) mas_res[j] = 0b10;
    if (!!getBit(a, i) && !!getBit(a, i - 1)) mas_res[j] = 0b11;
  }
  for (int i = 0; i < 48; i++) {
    if (i % 2)
      odd += mas_res[i];
    else
      even += mas_res[i];
  }
  if ((odd - even) % 5) flag = 1;
  return flag;
}

s21_decimal s21_sub_int(s21_decimal a, s21_decimal b) {
  s21_decimal null = {{0u, 0u, 0u, 0u}, s21_NORMAL_VALUE};
  s21_decimal res = {{0u, 0u, 0u, 0u}, s21_NORMAL_VALUE};
  s21_decimal second = {{0u, 0u, 0u, 0u}, s21_NORMAL_VALUE};
  int max_bit = findFirstNonZeroBit(a, 95);
  second = invert(b, max_bit);
  res = s21_add_int(a, second);
  for (int i = 0; i <= max_bit; i++) {
    if (!!getBit(res, i)) setBit(&null, i);
  }
  return null;
}

int s21_is_equal(s21_decimal a, s21_decimal b) {
  int flag = 1;
  if (checkIfZero(a) && checkIfZero(b)) flag = 0;
  if (flag && (!!getBit(a, 127) == !!getBit(b, 127))) {
    equalize(&a, &b, NULL);
    for (int i = 0; i < 96; i++) {
      if (!!getBit(a, i) != !!getBit(b, i)) break;
      if (i == 95) flag = 0;
    }
  }

  normal_verbose_equal(a, b, &flag);
  return flag;
}

int s21_is_not_equal(s21_decimal a, s21_decimal b) {
  int flag = !s21_is_equal(a, b);
  normal_verbose_equal(a, b, &flag);
  return flag;
}

int s21_is_less(s21_decimal a, s21_decimal b) {
  int flag = 1, index_a = 0, index_b = 0, new_scale = 0;
  if (checkIfZero(a) && checkIfZero(b)) {
  } else if (!!getBit(a, 127) && !getBit(b, 127)) {  // (a < 0) && (b > 0)
    flag = 0;
  } else if (!getBit(a, 127) && !!getBit(b, 127)) {   // (a > 0) && (b < 0)
  } else if (!!getBit(a, 127) && !!getBit(b, 127)) {  // (a < 0) && (b < 0)
    switchSign(&a);
    switchSign(&b);
    flag = s21_is_less(b, a);
  } else {
    equalize(&a, &b, NULL);

    for (int i = 95; i >= 0; i--) {
      index_a = findFirstNonZeroBit(a, i);
      index_b = findFirstNonZeroBit(b, i);
      if (index_a < index_b) {
        flag = 0;
        break;
      } else if (index_a > index_b) {
        break;
      }
    }
  }

  normal_verbose_less(a, b, &flag);
  return flag;
}

int s21_is_greater(s21_decimal a, s21_decimal b) {
  int flag = s21_is_less(b, a);
  normal_verbose_less(b, a, &flag);
  return flag;
}

int s21_is_greater_or_equal(s21_decimal a, s21_decimal b) {
  int flag = s21_is_greater(a, b) && s21_is_equal(a, b);
  normal_verbose_less_or_equal(b, a, &flag);
  return flag;
}

int s21_is_less_or_equal(s21_decimal a, s21_decimal b) {
  int flag = s21_is_less(a, b) && s21_is_equal(a, b);
  normal_verbose_less_or_equal(a, b, &flag);
  return flag;
}

s21_decimal s21_sub(s21_decimal a, s21_decimal b) {
  int flag = 1;
  s21_decimal res = {{0u, 0u, 0u, 0u}, s21_NORMAL_VALUE};
  if (!!getBit(a, 127) && !!getBit(b, 127)) {  // (a < 0) && (b < 0)
    switchSign(&a);
    switchSign(&b);
    res = s21_sub(b, a);
    flag = 0;
  } else if (!getBit(a, 127) && !!getBit(b, 127)) {  // (a > 0) && (b < 0)
    switchSign(&b);
    res = s21_add(a, b);
    flag = 0;
  } else if (!!getBit(a, 127) && !getBit(b, 127)) {  // (a < 0) && (b > 0)
    switchSign(&a);
    res = s21_add(a, b);
    switchSign(&res);
    flag = 0;
  }

  // !s21_is_less(a, b) = A < B
  if (flag && !s21_is_less(a, b)) {
    res = s21_sub(b, a);
    switchSign(&res);
    flag = 0;
  }

  if (flag) {
    int res_scale = 0;
    equalize(&a, &b, &res_scale);
    res = s21_sub_int(a, b);
    set_scale(&res, res_scale);
  }

  normal_verbose_sub(a, b, &res);

  normalize(&res);
  return res;
}

s21_decimal s21_negate(s21_decimal a) {
  s21_decimal res = a;
  if (res.value_type == 1) res.value_type = 2;
  if (res.value_type == 2) res.value_type = 1;
  switchSign(&res);
  normalize(&res);
  return res;
}

s21_decimal s21_div_ten(s21_decimal a) {
  s21_decimal tmp = {{0u, 0u, 0u, 0u}, s21_NORMAL_VALUE};
  s21_decimal res = {{0u, 0u, 0u, 0u}, s21_NORMAL_VALUE};
  s21_decimal b = {{10u, 0u, 0u, 0u}, s21_NORMAL_VALUE};
  int index = findFirstNonZeroBit(a, 95);
  int integralCounter = 0, fractionalCounter = 0;
  int *partCounter = &integralCounter;
  int i = 0;
  while (1) {
    if (((integralCounter + fractionalCounter) == 96) ||
        (checkIfZero(tmp) && (index < 0)))
      break;
    while (s21_is_less(tmp, b) == 0) {
      if ((integralCounter + fractionalCounter) == 96) break;
      if (checkIfZero(tmp) && (index < 0)) break;
      if (index < 0) partCounter = &fractionalCounter;
      shift(&tmp, 1);  // увеличиваем tmp, спуская 1 или 0 из А
      if ((index >= 0) && !!getBit(a, index)) setBit(&tmp, 0);
      if (s21_is_less(tmp, b) == 0 && !checkIfZero(res)) {
        shift(&res, 1);  // добавили 0 в res.
        (*partCounter)++;
      }
      index--;
    }
    if (((integralCounter + fractionalCounter) == 96) ||
        (checkIfZero(tmp) && (index < 0)))
      break;
    tmp = s21_sub(tmp, b);
    shift(&res, 1);
    setBit(&res, 0);
    (*partCounter)++;
  }
  normalize(&res);
  return res;
}

s21_decimal s21_div_int(s21_decimal a, s21_decimal b, int *intCount,
                        int *fracCount) {
  normalize(&a);
  normalize(&b);
  s21_decimal tmp = {{0u, 0u, 0u, 0u}, s21_NORMAL_VALUE};
  s21_decimal res = {{0u, 0u, 0u, 0u}, s21_NORMAL_VALUE};
  int index = findFirstNonZeroBit(a, 95);
  int integralCounter = 0, fractionalCounter = 0;
  int *partCounter = &integralCounter;

  while (1) {
    if (((integralCounter + fractionalCounter) == 96) ||
        (checkIfZero(tmp) && (index < 0)))
      break;
    while (s21_is_less(tmp, b) == 0) {
      if ((integralCounter + fractionalCounter) == 96) break;
      if (checkIfZero(tmp) && (index < 0)) break;
      if (index < 0) partCounter = &fractionalCounter;
      shift(&tmp, 1);  // увеличиваем tmp, спуская 1 или 0 из А
      if ((index >= 0) && !!getBit(a, index)) setBit(&tmp, 0);
      if (s21_is_less(tmp, b) == 0 && !checkIfZero(res)) {
        shift(&res, 1);  // добавили 0 в res.
        (*partCounter)++;
      }
      index--;
    }
    if (((integralCounter + fractionalCounter) == 96) ||
        (checkIfZero(tmp) && (index < 0)))
      break;
    tmp = s21_sub(tmp, b);
    shift(&res, 1);
    setBit(&res, 0);
    (*partCounter)++;
  }
  *intCount = integralCounter;
  *fracCount = fractionalCounter;
  normalize(&res);
  return res;
}

s21_decimal extractIntegralPart(s21_decimal a, int length) {
  s21_decimal res = {{0u, 0u, 0u, 0u}, s21_NORMAL_VALUE};
  int index = findFirstNonZeroBit(a, 95);
  for (int i = length - 1, j = index; i >= 0; i--, j--) {
    if (getBit(a, j)) setBit(&res, i);
  }
  return res;
}

s21_decimal s21_floor(s21_decimal a) {
  s21_decimal one = {{1u, 0u, 0u, 0u}, s21_NORMAL_VALUE};
  s21_decimal zero = {{0u, 0u, 0u, 0u}, s21_NORMAL_VALUE};
  s21_decimal remainder = s21_mod(a, one);
  if (s21_is_greater_or_equal(a, zero) == 1) {
    a = s21_sub(a, one);
  }

  s21_decimal result = s21_sub(a, remainder);
  result.value_type = a.value_type;

  return result;
}

s21_decimal s21_round(s21_decimal a) {
  s21_decimal zero_five = {{5u, 0u, 0u, 0u}, s21_NORMAL_VALUE};
  set_scale(&zero_five, 1);
  s21_decimal negative_zero_five = {{5u, 0u, 0u, 0u}, s21_NORMAL_VALUE};
  set_scale(&negative_zero_five, 1);
  setBit(&negative_zero_five, 127);
  s21_decimal one = {{1u, 0u, 0u, 0u}, s21_NORMAL_VALUE};
  s21_decimal zero = {{0u, 0u, 0u, 0u}, s21_NORMAL_VALUE};
  s21_decimal remainder = s21_mod(a, one);

  if (!s21_is_greater(a, zero))
    if (!s21_is_greater(remainder, zero_five)) a = s21_add(a, one);
  if (!s21_is_less(a, zero))
    if (!s21_is_less(remainder, negative_zero_five)) a = s21_sub(a, one);

  s21_decimal result = s21_sub(a, remainder);
  result.value_type = a.value_type;

  return result;
}

s21_decimal s21_truncate(s21_decimal a) {
  s21_decimal one = {{1u, 0u, 0u, 0u}, s21_NORMAL_VALUE};

  s21_decimal result = s21_sub(a, s21_mod(a, one));
  result.value_type = a.value_type;

  return result;
}

s21_decimal s21_mod(s21_decimal a, s21_decimal b) {
  int negative_flag = 0;
  if ((!getBit(a, 127)) ^ (!getBit(b, 127))) negative_flag = 1;
  cleanBit(&a, 127);
  cleanBit(&b, 127);
  normalize(&a);
  normalize(&b);
  int eq = 0;
  equalize(&a, &b, &eq);
  set_scale(&a, 0);
  set_scale(&b, 0);
  int intCount = 0, fracCount = 0, powerCounter = 0;
  s21_decimal res = s21_div_int(a, b, &intCount, &fracCount);
  s21_decimal integral = extractIntegralPart(res, intCount);
  s21_decimal remainder = s21_sub(a, s21_mul(b, integral));
  set_scale(&remainder, eq);
  if (negative_flag) setBit(&remainder, 127);

  normal_verbose_mod(a, b, &remainder);

  return remainder;
}

s21_decimal s21_div(s21_decimal a, s21_decimal b) {
  int negative_flag = 0;
  s21_decimal tmp_a = a;
  s21_decimal tmp_b = b;
  s21_decimal result = {{0u, 0u, 0u, 0u}, s21_NORMAL_VALUE};
  if ((!getBit(a, 127)) ^ (!getBit(b, 127))) negative_flag = 1;

  cleanBit(&a, 127);
  cleanBit(&b, 127);

  normalize(&a);
  normalize(&b);
  int difference = getDegree(a) - getDegree(b);

  set_scale(&a, 0);
  set_scale(&b, 0);
  int intCount = 0, fracCount = 0, powerCounter = 0;
  s21_decimal res = s21_div_int(a, b, &intCount, &fracCount);
  s21_decimal integral = extractIntegralPart(res, intCount);
  s21_decimal remainder = s21_sub(a, s21_mul(b, integral));
  s21_decimal tmp = {{0u, 0u, 0u, 0u}, s21_NORMAL_VALUE}, integralRemainder;

  while (1) {
    if (fracCount == 0) {
      if (powerCounter != 0)
        integralRemainder = extractIntegralPart(tmp, intCount);
      break;
    }
    if (powerCounter == 27) break;
    powerCounter++;
    decimal_pow(&remainder, 1);
    tmp = s21_div_int(remainder, b, &intCount, &fracCount);
  }
  s21_decimal integralOfRem = extractIntegralPart(tmp, intCount);

  decimal_pow(&integral, powerCounter);
  set_scale(&integral, powerCounter);
  set_scale(&integralOfRem, powerCounter);

  result = s21_add(integral, integralOfRem);
  if (difference < 0) {
    for (; difference < 0; difference++) decimal_pow(&result, 1);
  } else {
    int currentScale = getDegree(result);
    set_scale(&result, currentScale + difference);
  }
  if (negative_flag) setBit(&result, 127);

  normal_verbose_div(tmp_a, tmp_b, &result);

  return result;
}
