/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/

#include <stdio.h>
#include <math.h>
#include <float.h> // For constants like DBL_MAX
#include "ql_application.h"
#include "ql_rtos.h"

static unsigned char isDcsMode = 1; //DCS mode as default

static int fpu_instruction_set_test(void)
{
    float a = 1.5f;
    float b = 2.5f;
    float result;

    // test add
    result = a + b;
    printf("Addition: %f + %f = %f\n", a, b, result);

    // test subtration
    result = a - b;
    printf("Subtraction: %f - %f = %f\n", a, b, result);

    // test multiplication
    result = a * b;
    printf("Multiplication: %f * %f = %f\n", a, b, result);

    // test devision
    result = a / b;
    printf("Division: %f / %f = %f\n", a, b, result);

    // test sqrt
    result = sqrt(b);
    printf("Square Root: sqrt(%f) = %f\n", b, result);

    // test sin
    result = sin(a);
    printf("Sine: sin(%f) = %f\n", a, result);

    // test cos
    result = cos(b);
    printf("Cosine: cos(%f) = %f\n", b, result);

    return 0;
}

#define NUM_OPERATIONS 10000000
static int fpu_performance_test(void)
{
    unsigned int start, end;
	// note: add volatile to avoid compiler optimization
    volatile double cpu_time_used;
    volatile double result = 1.0;
    volatile double operand = 1.000001;
    #define FREQ_S   ((isDcsMode == 1) ? (32768) : (32787))
    int i;

    // test float add performance
    start = ql_rtc_get_ticks();
    for (i = 0; i < NUM_OPERATIONS; i++) {
        result += operand;
    }
    end = ql_rtc_get_ticks();
    cpu_time_used = ((double) (end - start)) / FREQ_S;
    printf("Time taken for addition: %f seconds\n", cpu_time_used);

    result = 1.0;

    // test float multiplication performance
    start = ql_rtc_get_ticks();
    for (i = 0; i < NUM_OPERATIONS; i++) {
        result *= operand;
    }
    end = ql_rtc_get_ticks();
    cpu_time_used = ((double) (end - start)) / FREQ_S;
    printf("Time taken for multiplication: %f seconds\n", cpu_time_used);

    return 0;
}

static int fpu_precision_test(void)
{
    double a = 0.1;
    double b = 0.2;
    double c = 0.3;

    // test add precision
    double sum = a + b;
    if (fabs(sum - c) < 1e-9) {
        printf("Addition precision test passed.\n");
    } else {
        printf("Addition precision test failed: Expected 0.3, got %f\n", sum);
    }

    // test multiplication precision
    double a_mul = 123456789.1;
    double b_mul = 1e-8;
    double expected_mul_result = 1.234567891;
    double mul_result = a_mul * b_mul;

    if (fabs(mul_result - expected_mul_result) < 1e-9) {
        printf("Multiplication precision test passed.\n");
    } else {
        printf("Multiplication precision test failed: Expected %f, got %f\n", expected_mul_result, mul_result);
    }

    return 0;
}

//fpu_test_case
void fpu_test_case(void)
{
	while(1)
	{
		fpu_instruction_set_test();
		fpu_performance_test();
		fpu_precision_test();
		ql_rtos_task_sleep_s(3);
	}
}

//application_init(fpu_test_case,"fpu_test_case",2, 0);

