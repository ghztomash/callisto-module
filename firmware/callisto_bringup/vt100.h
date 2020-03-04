
// -------------- VT100 Defines ----------------------
#define VTBLACK 	0
#define VTRED 		1
#define VTGREEN 	2
#define VTYELLOW 	3
#define VTBLUE	 	4
#define VTMAGENTA 	5
#define VTCYAN 		6
#define VTWHITE 	7

#define VTNORMAL 	0
#define VTBOLD	 	1
#define VTLINE	 	4
#define VTBLINK 	5

#define TAB_INCREMENT 8
#define TABLE_SETUP 1
#define TABLE_ADC 7
#define TABLE_GRAPH 24

#define DRAWCSV 0

#define HIGHLOW(a) (a ? "\033[0;32;40m- HIGH -\033[0;37;40m" : "\033[0;31;40m- LOW -\033[0;37;40m")
// ---------------------------------------------------