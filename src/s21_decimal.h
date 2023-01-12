#ifndef SRC_S21_DECIMAL_H_
#define SRC_S21_DECIMAL_H_

#define TRUE 0
#define FALSE 1

typedef enum {
  s21_NORMAL_VALUE = 0,
  s21_INFINITY = 1,
  s21_NEGATIVE_INFINITY = 2,
  s21_NAN = 3
} value_type_t;

typedef struct {
  unsigned int bits[4];
  value_type_t value_type;
} s21_decimal;

void set_scale(s21_decimal *a, int scale);
void setBit(s21_decimal *d, int i);
s21_decimal s21_add(s21_decimal a, s21_decimal b);
s21_decimal s21_sub(s21_decimal a, s21_decimal b);
s21_decimal s21_mul(s21_decimal a, s21_decimal b);
s21_decimal s21_sub(s21_decimal a, s21_decimal b);
s21_decimal s21_div(s21_decimal a, s21_decimal b);
s21_decimal s21_mod(s21_decimal a, s21_decimal b);
int s21_is_less(s21_decimal a, s21_decimal b);
int s21_is_less_or_equal(s21_decimal a, s21_decimal b);
int s21_is_greater(s21_decimal a, s21_decimal b);
int s21_is_greater_or_equal(s21_decimal a, s21_decimal b);
int s21_is_equal(s21_decimal a, s21_decimal b);
int s21_is_not_equal(s21_decimal a, s21_decimal b);
int s21_from_int_to_decimal(int src, s21_decimal *dst);
int s21_from_float_to_decimal(float src, s21_decimal *dst);
int s21_from_decimal_to_int(s21_decimal src, int *dst);
int s21_from_decimal_to_float(s21_decimal src, float *dst);
s21_decimal s21_floor(s21_decimal a);
s21_decimal s21_round(s21_decimal a);
s21_decimal s21_truncate(s21_decimal a);
s21_decimal s21_negate(s21_decimal a);

void equalize(s21_decimal *a, s21_decimal *b, int *res_scale);
void normalize(s21_decimal *a);
s21_decimal s21_div_ten(s21_decimal a);
void decimal_byte_to_byte(s21_decimal a);
void set_scale(s21_decimal *a, int i);
s21_decimal s21_div_int(s21_decimal a, s21_decimal b, int *intCount,
                        int *fracCount);
s21_decimal extractIntegralPart(s21_decimal a, int length);
int getDegree(s21_decimal dec);
int sign(s21_decimal a);
void decimal_pow(s21_decimal *a, int power);
void print_decimal(s21_decimal a);
void normal_verbose_mul(s21_decimal a, s21_decimal b, s21_decimal *res);

#endif  // SRC_S21_DECIMAL_H_
