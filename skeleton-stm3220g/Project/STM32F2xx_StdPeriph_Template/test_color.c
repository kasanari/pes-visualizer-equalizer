#include <stdio.h>
#include "color.h"

#define GET_RED(col)    (col>>11)
#define GET_GREEN(col)  ((col & 0b0000011111100000)>>5)
#define GET_BLUE(col)    (col & 0b0000000000011111)


typedef enum {
    FAIL = 0,
    SUCCESS = 1
} result_t;




/*
*   @param: red, green, blue are values 0-100
*/
result_t testHSL(char *name, unsigned short hue, unsigned short saturation, unsigned short lightness, int red, int green, int blue)
{
    unsigned int result = getColorFromHSL(hue, saturation, lightness);
    int R = (red    * 31) / 100;
	int G = (green  * 63) / 100;
	int B = (blue   * 31) / 100;
	unsigned int expected = GET_COLOR_16BIT(R, G, B);
    int diff_r = abs(GET_RED(result)   - GET_RED(expected));
    int diff_g = abs(GET_GREEN(result) - GET_GREEN(expected));
    int diff_b = abs(GET_BLUE(result)  - GET_BLUE(expected));
    
    int pass = diff_r <= 1 && diff_g <= 1 && diff_b <= 1;

    printf("-\t\t%s\t\t-", name);
    if (pass) {
        printf("\t\tSUCCESS\n");
    } else {
        printf("\t\tFAIL\n");
        printf("\tresult:\t\t %u, %u, %u\n", GET_RED(result), GET_GREEN(result), GET_BLUE(result));
        printf("\texpected:\t %u, %u, %u\n", GET_RED(expected), GET_GREEN(expected), GET_BLUE(expected));
        printf("\tdiffrence:\t %i, %i, %i\n", diff_r, diff_g, diff_b);
        printf("Test failed for %s\n", name);
    }
    return pass;
}

int main()
{
    printf("====================\n");
    printf("Running test for HSL\n");

    /*                            |        Input            |   Expected [0-100]   |   */
    /*                            | Hue     Satur   Light   | red     green   blue |   */
    /* Test black and white*/
    testHSL("Black",                0,      0,      0,          0,      0,      0);
    testHSL("Black w. sat",         0,      50,     0,          0,      0,      0);
    testHSL("Black w. hue",         68,     0,      0,          0,      0,      0);
    testHSL("White",                0,      0,      100,        100,    100,    100);
    testHSL("White w. sat",         0,      78,     100,        100,    100,    100);
    testHSL("White w. hue",         65,     0,      100,        100,    100,    100);
    /* Test Hue */
    testHSL("red",                  0,      100,    50,         100,    0,      0);
    testHSL("red h+360",            360,    100,    50,         100,    0,      0);
    testHSL("green",                120,    100,    50,         0,      100,    0);
    testHSL("green h+360",          480,    100,    50,         0,      100,    0);
    testHSL("blue",                 240,    100,    50,         0,      0,      100);
    testHSL("Yellow",               60,     100,    50,         100,    100,    0);
    testHSL("Teal",                 180,    100,    50,         0,      100,    100);
    testHSL("Magenta",              300,    100,    50,         100,    0,      100);
    /* Test Saturation */
    testHSL("red s 95",             0,      95,     50,         97,     2,      2);
    testHSL("red s 75",             0,      75,     50,         91,     13,     13);
    testHSL("red s 55",             0,      55,     50,         78,     22,     22);
    testHSL("red s 40",             0,      40,     50,         70,     30,     30);
    testHSL("red s 25",             0,      25,     50,         62,     38,     38);
    testHSL("red s 10",             0,      10,     50,         55,     45,     45);
    testHSL("Grey",                 0,      0,      50,         50,     50,     50);
    /* Test Lightness */
    testHSL("red l 90",             0,      100,    90,         100,    80,     80);
    testHSL("red l 75",             0,      100,    75,         100,    50,     50);
    testHSL("red l 40",             0,      100,    40,         80,     0,      0);
    testHSL("red l 30",             0,      100,    30,         60,     0,      0);
    testHSL("red l 10",             0,      100,    10,         20,     0,      0);

    printf("====================\n");
    return 0;
}
