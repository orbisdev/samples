#include <stdio.h>
int main()
{
 	printf("%10s %d\n","1000.00 KB",strlen("1000.00 KB"));
 	printf("%10s %d\n","      3 B ",strlen("3 B "));
 	printf("%10s %d\n","33 B ",strlen("33 B "));
 	printf("%10s %d\n","333 B ",strlen("333 B "));
 	printf("%10s %d\n","1022 B ",strlen("1022 B "));
 	printf("%10s %d\n","1.00 KB",strlen("1.00 KB"));
 	printf("%10s %d\n","100.00 KB",strlen("100.00 KB"));
 	printf("%10s %d\n","1000.00 KB",strlen("1000.00 KB"));


}
