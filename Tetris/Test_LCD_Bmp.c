#include "NUC1xx.h"
#include "LCD_Driver.h"
#include "Driver/DrvGPIO.h"
#include "Driver/DrvSYS.h"
//#include<stdarg.h>

#define RandMax 63949
#define PRIME1 29137
#define PRIME2 12345
#define PRIME3 23

uint16_t rd_meter;

typedef struct
{
	short int offset_x;
	short int offset_y;
}element;

typedef struct
{
	element block1;
	element block2;
	element block3;
	element block4;
}object;

extern  SPI_T * SPI_PORT[4]={SPI0, SPI3, SPI2, SPI3};
volatile uint16_t new_T0_v=1,old_T0_v=0;
uint16_t itrpt_T0=0;
uint16_t restart_meter=0;
uint16_t kb[9] = {0};
GPIO_T *tGPIO_C,*tGPIO_E;

uint16_t seg_num[10] = {0x82,0xee,0x07,0x46,0x6a,0x52,0x1a,0xe2,0x02,0x62};
uint16_t seg_monitor[4] = {0xff10,0xff20,0xff40,0xff80};
uint16_t kf_delay=0,kf_d_t=0;
uint16_t s_del=3,n_del=15,l_del=40;
uint16_t delay;

uint8_t duty_cycle = 50;

unsigned char number8x8[10][8] = {
	{0x00,0x7E,0xA1,0x91,0x89,0x85,0x7E,0x00},
	{0x00,0x00,0x84,0x82,0xFF,0x80,0x80,0x00},
	{0x00,0x00,0xC2,0xA1,0x91,0x89,0x86,0x00},
	{0x00,0x42,0x81,0x89,0x89,0x76,0x00,0x00},
	{0x00,0x30,0x2C,0x22,0xFF,0x20,0x00,0x00},
	{0x4F,0x89,0x89,0x89,0x89,0x71,0x00,0x00},
	{0x00,0x7E,0x91,0x89,0x89,0x89,0x70,0x00},
	{0x00,0x00,0x03,0xC1,0x31,0x0D,0x07,0x00},
	{0x00,0x76,0x89,0x89,0x89,0x89,0x76,0x00},
	{0x00,0x0E,0x91,0x91,0x89,0x7E,0x00,0x00}
};
unsigned char number[22][8] = {
//zero(0)
{0xC0,0x20,0x10,0x10,0x20,0xC0,0x00,0x00},
{0x3F,0x40,0x80,0x80,0x40,0x3F,0x00,0x00},
//one(1)
{0x20,0xF0,0x00,0x00,0x00,0x00,0x00,0x00},
{0x80,0xFF,0x80,0x00,0x00,0x00,0x00,0x00},
//two(2)
{0x60,0x10,0x10,0x10,0x20,0xC0,0x00,0x00},
{0xC0,0xA0,0x90,0x88,0x86,0xC1,0x00,0x00},
//three(3)
{0x20,0x10,0x10,0x10,0xE0,0x00,0x00,0x00},
{0x80,0x80,0x82,0x83,0x44,0x38,0x00,0x00},
//four(4)
{0x00,0x00,0x80,0x60,0xF0,0x00,0x00,0x00},
{0x18,0x16,0x11,0x10,0xFF,0x10,0x00,0x00},
//five(5)
{0x00,0xF0,0x10,0x10,0x10,0x10,0x00,0x00},
{0x80,0x81,0x81,0x81,0x42,0x3C,0x00,0x00},
//six(6)
{0x80,0x40,0x20,0x10,0x10,0x10,0x00,0x00},
{0x3F,0x42,0x81,0x81,0x42,0x3C,0x00,0x00},
//seven(7)
{0x70,0x10,0x10,0x10,0x90,0x70,0x00,0x00},
{0x00,0x00,0xE0,0x1C,0x03,0x00,0x00,0x00},
//eight(8)
{0xC0,0x20,0x10,0x10,0x20,0xC0,0x00,0x00},
{0x38,0x45,0x82,0x82,0x45,0x38,0x00,0x00},
//nine(9)
{0xC0,0x20,0x10,0x10,0x20,0xC0,0x00,0x00},
{0x83,0x84,0x88,0x48,0x24,0x1F,0x00,0x00},
//add(10)
{0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00},
{0x08,0x08,0x08,0xFF,0x08,0x08,0x08,0x00}};

unsigned char letter[52][8] = {
//A(0)
{0x00,0x00,0x80,0x70,0x80,0x00,0x00,0x00},
{0x80,0xF0,0x8F,0x08,0x8F,0xF0,0x80,0x00},
//B(1)
{0x10,0xF0,0x10,0x10,0x10,0x20,0xC0,0x00},
{0x80,0xFF,0x82,0x82,0x82,0x45,0x38,0x00},
//C(2)
{0xC0,0x20,0x10,0x10,0x10,0x20,0x70,0x00},
{0x3F,0x40,0x80,0x80,0x80,0x80,0x40,0x00},
//D(3)
{0x10,0xF0,0x10,0x10,0x10,0x60,0x80,0x00},
{0x80,0xFF,0x80,0x80,0x80,0x60,0x1F,0x00},
//E(4)
{0x10,0xF0,0x10,0x10,0x10,0x10,0x70,0x00},
{0x80,0xFF,0x82,0x82,0x82,0x87,0xE0,0x00},
//F(5)
{0x10,0xF0,0x10,0x10,0x10,0x10,0x70,0x00},
{0x80,0xFF,0x82,0x02,0x02,0x07,0x00,0x00},
//G(6)
{0xC0,0x20,0x10,0x10,0x20,0x70,0x00,0x00},
{0x3F,0x40,0x80,0x80,0x84,0x7C,0x04,0x00},
//H(7)
{0x10,0xF0,0x10,0x00,0x10,0xF0,0x10,0x00},
{0x80,0xFF,0x82,0x02,0x82,0xFF,0x80,0x00},
//I(8)
{0x10,0xF0,0x10,0x00,0x00,0x00,0x00,0x00},
{0x80,0xFF,0x80,0x00,0x00,0x00,0x00,0x00},
//J(9)
{0x00,0x00,0x10,0xF0,0x10,0x00,0x00,0x00},
{0x40,0x80,0x80,0x7F,0x00,0x00,0x00,0x00},
//K(10)
{0x10,0xF0,0x10,0x80,0x50,0x30,0x10,0x00},
{0x80,0xFF,0x83,0x0C,0xB0,0xC0,0x80,0x00},
//L(11)
{0x10,0xF0,0x10,0x00,0x00,0x00,0x00,0x00},
{0x80,0xFF,0x80,0x80,0x80,0xC0,0x20,0x00},
//M(12)
{0x10,0xF0,0x80,0x00,0x80,0xF0,0x10,0x00},
{0x80,0xFF,0x83,0x3C,0x83,0xFF,0x80,0x00},
//N(13)
{0x10,0xF0,0xC0,0x00,0x10,0xF0,0x10,0x00},
{0x80,0xFF,0x80,0x0F,0x30,0xFF,0x00,0x00},
//O(14)
{0x80,0x60,0x10,0x10,0x10,0x60,0x80,0x00},
{0x1F,0x60,0x80,0x80,0x80,0x60,0x1F,0x00},
//P(15)
{0x10,0xF0,0x10,0x10,0x10,0x20,0xC0,0x00},
{0x80,0xFF,0x84,0x04,0x04,0x02,0x01,0x00},
//Q(16)
{0xE0,0x18,0x04,0x04,0x04,0x18,0xE0,0x00},
{0x07,0x18,0x20,0x20,0x60,0x98,0x87,0x00},
//R(17)
{0x10,0xF0,0x10,0x10,0x10,0x20,0xC0,0x00},
{0x80,0xFF,0x84,0x0C,0x34,0xC2,0x81,0x00},
//S(18)
{0xC0,0x20,0x10,0x10,0x10,0x20,0x70,0x00},
{0xE0,0x41,0x82,0x82,0x84,0x48,0x30,0x00},
//T(19)
{0x30,0x10,0x10,0xF0,0x10,0x10,0x30,0x00},
{0x00,0x00,0x80,0xFF,0x80,0x00,0x00,0x00},
//U(20)
{0x10,0xF0,0x10,0x00,0x10,0xF0,0x10,0x00},
{0x00,0x7F,0x80,0x80,0x80,0x7F,0x00,0x00},
//V(21)
{0x10,0xF0,0x10,0x00,0x10,0xF0,0x10,0x00},
{0x00,0x01,0x1E,0xE0,0x1E,0x01,0x00,0x00},
//W(22)
{0x10,0xF0,0x10,0xC0,0x10,0xF0,0x10,0x00},
{0x00,0x0F,0xF0,0x0F,0xF0,0x0F,0x00,0x00},
//X(23)
{0x10,0x70,0x90,0x00,0x90,0x70,0x10,0x00},
{0x80,0xE0,0x99,0x06,0x99,0xE0,0x80,0x00},
//Y(24)
{0x10,0x70,0x90,0x00,0x90,0x70,0x10,0x00},
{0x00,0x00,0x83,0xFC,0x83,0x00,0x00,0x00},
//Z(25)
{0x70,0x10,0x10,0x10,0x10,0xD0,0x30,0x00},
{0xC0,0xB0,0x88,0x86,0x81,0x80,0xE0,0x00}
};

unsigned matter[17][10] = 
{
	{0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x09},
	{0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x09},
	{0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x09},
	{0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x09},
	{0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x09},
	{0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x09},
	{0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x09},
	{0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x09},
	{0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x09},
	{0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x09},
	{0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x09},
	{0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x09},
	{0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x09},
	{0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x09},
	{0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x09},
	{0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x09},
	{0x09,0x09,0x09,0x09,0x09,0x09,0x09,0x09,0x09,0x09},
};

object tetris[7] = {
	//T0
	{{0,0},{1,0},{2,0},{-1,0}},
	//T1
	{{0,0},{1,0},{1,-1},{-1,0}},
	//T2
	{{0,0},{1,0},{-1,0},{-1,-1}},
	//T3
	{{0,0},{0,-1},{-1,0},{-1,-1}},
	//T4
	{{0,0},{1,0},{0,-1},{-1,-1}},
	//T5
	{{0,0},{1,0},{0,-1},{-1,0}},
	//T6
	{{0,0},{1,-1},{0,-1},{-1,0}}
};

unsigned square[8] = {
	0xFF	,0x81	,0x81	,0x81	,0x81	,0x81	,0x81	,0xFF
};

//----------------------------------------------------------------------------
//  Prototype
//----------------------------------------------------------------------------
void InitTIMER0(void);
void TMR0_IRQHandler(void);
void Initial_Lcd(void);
void draw(uint8_t,uint8_t,unsigned char);
void Lprintf(uint16_t ,uint16_t ,uint16_t );
void lprintf(uint16_t ,uint16_t ,uint16_t );
uint16_t ipow(uint16_t ,uint16_t );
void kscanf(uint16_t *);
void iacpy(uint16_t *,uint16_t *,uint16_t);
uint16_t iacmp(uint16_t *,uint16_t *,uint16_t);
void iaclear(uint16_t *,uint16_t);
uint16_t ilog(uint16_t);
//uint16_t SK(uint16_t count,...);
uint16_t ifprime(uint16_t);
void InitTIMER1(void);
void TMR1_IRQHandler(void);
void ELP(uint16_t ,uint16_t ,uint16_t );
void rotate(void);
short int test_bingo(short int y);
void elim_p(short int y);
void t_draw(short int x,short int y);
void redraw(void);
uint16_t _rand(void); 
void matter_clear(void);
uint16_t n_kb(void);
void InitPWM(void);
void PWM4_Freq(uint32_t PWM_frequency, uint8_t PWM_duty);


uint16_t sd = 0,rd=0;
object D,temp_ob;
object nD;
uint16_t rd_sq,rd_sq_temp;
short int D_x=4,D_y=0,level=16;
short int sum_grid,elim_y_max=0,elim_y_min=15;
uint16_t POINT=0,point_size;
uint16_t bz_1=0,bz_2=0,bz_3=0,bz_delay=0,bz_d_t=0,bz_d_m=99;
uint16_t glance=0;
//----------------------------------------------------------------------------
//  MAIN function
//----------------------------------------------------------------------------
int32_t main (void)
{
	uint16_t i;
	uint16_t circle;
	
	UNLOCKREG();
	SYSCLK->PWRCON.XTL12M_EN = 1;//啟動12MHZ的時脈計數器
	//SYSCLK->PWRCON.XTL32K_EN = 1;
	//SYSCLK->PWRCON.OSC22M_EN = 1;
	//SYSCLK->PWRCON.OSC10K_EN = 1;
	LOCKREG();
	
	tGPIO_E = (GPIO_T *)((uint32_t)GPIOA + (4*0x40));
	tGPIO_C = (GPIO_T *)((uint32_t)GPIOA + (2*0x40));
	delay=n_del;
	
	//main_loop
	while(1)
	{
		Initial_Lcd();
		ELP(15,56,0);ELP(11,64,0);ELP(18,72,0);
		ELP(15,48,2);ELP(17,56,2);ELP(4,64,2);ELP(18,72,2);ELP(18,80,2);
		ELP(0,56,4);ELP(13,64,4);ELP(24,72,4);
		ELP(10,56,6);ELP(4,64,6);ELP(24,72,6);
		//start_game
		while(1)
		{
			kscanf(kb);
			if(kb[0]||kb[1]||kb[2]||kb[3]||kb[4]||kb[5]||kb[6]||kb[7]||kb[8])
			{
				break;
			}
			++sd;
		}
		Initial_Lcd();
		iaclear(kb,8);
		rd_meter=sd;
		rd_sq = _rand() % 7;
		D=tetris[rd_sq];
		rd_sq_temp = _rand() % 7;
		nD=tetris[rd_sq_temp];
		
		InitTIMER0();
		InitPWM();
		new_T0_v=1;old_T0_v=0;
		circle=0;
		//tetris_game_loop
		while(1)
		{
			D_y = new_T0_v;
			//Dead_point
			if(level<=2)
			{
				break;
			}
			
			//itrpt
			if(itrpt_T0)
			{
				itrpt_T0=0;
				
				//clear_old_D_Position
				matter[D_y-1][D_x+1]=0;
				matter[D_y+D.block2.offset_y-1][D_x+D.block2.offset_x+1]=0;
				matter[D_y+D.block3.offset_y-1][D_x+D.block3.offset_x+1]=0;
				matter[D_y+D.block4.offset_y-1][D_x+D.block4.offset_x+1]=0;
				
				//test_end
				sum_grid=0;
				sum_grid+=(matter[D_y][D_x+1]+matter[D_y+D.block2.offset_y][D_x+D.block2.offset_x+1]+matter[D_y+D.block3.offset_y][D_x+D.block3.offset_x+1]+matter[D_y+D.block4.offset_y][D_x+D.block4.offset_x+1]);
				if(sum_grid)
				{
					bz_1=1;
					matter[D_y-1][D_x+1]=0x01;
					matter[D_y+D.block2.offset_y-1][D_x+D.block2.offset_x+1]=0x01;
					matter[D_y+D.block3.offset_y-1][D_x+D.block3.offset_x+1]=0x01;
					matter[D_y+D.block4.offset_y-1][D_x+D.block4.offset_x+1]=0x01;
					
					//elim
					if(D_y-1>elim_y_max)elim_y_max=D_y-1;if(D_y-1<elim_y_min)elim_y_min=D_y-1;
					if(D_y-1+D.block2.offset_y>elim_y_max)elim_y_max=D_y-1+D.block2.offset_y;if(D_y-1+D.block2.offset_y<elim_y_min)elim_y_min=D_y-1+D.block2.offset_y;
					if(D_y-1+D.block3.offset_y>elim_y_max)elim_y_max=D_y-1+D.block3.offset_y;if(D_y-1+D.block3.offset_y<elim_y_min)elim_y_min=D_y-1+D.block3.offset_y;
					if(D_y-1+D.block4.offset_y>elim_y_max)elim_y_max=D_y-1+D.block4.offset_y;if(D_y-1+D.block4.offset_y<elim_y_min)elim_y_min=D_y-1+D.block4.offset_y;
					for(i=elim_y_max;i>=elim_y_min;--i)
					{
						if(test_bingo(i))
						{
							elim_p(i);
							rd=1;
							++i;
							++POINT;
						}
					}
					
					if(rd)
					{
						rd=0;
						bz_2=1;
						Initial_Lcd();
						redraw();
						elim_y_max=0;
						elim_y_min=15;
					}
					
					//reset
					new_T0_v=1;old_T0_v=0;
					rd_sq = rd_sq_temp;
					rd_sq_temp = _rand() % 7;
					D=nD;
					nD=tetris[rd_sq_temp];
					//level
					if(D_y<level)level=D_y;if((D_y+D.block2.offset_y)<=level)level=D_y+D.block2.offset_y;if((D_y+D.block3.offset_y)<=level)level=D_y+D.block3.offset_y;if((D_y+D.block4.offset_y)<=level)level=D_y+D.block4.offset_y;
					D_x=4;
					continue;
				}
				
				//D_Positon_set
				matter[D_y][D_x+1]=0x01;
				matter[D_y+D.block2.offset_y][D_x+D.block2.offset_x+1]=0x01;
				matter[D_y+D.block3.offset_y][D_x+D.block3.offset_x+1]=0x01;
				matter[D_y+D.block4.offset_y][D_x+D.block4.offset_x+1]=0x01;
				//clear_old_D_Lcd
				for(i=(D_y-1)*8;i<(D_y)*8;++i)
				{
					draw(D_x,i,0);
					draw(D_x+D.block2.offset_x,i+(D.block2.offset_y*8),0);
					draw(D_x+D.block3.offset_x,i+(D.block3.offset_y*8),0);
					draw(D_x+D.block4.offset_x,i+(D.block4.offset_y*8),0);
				}
			}
			
			//bz
			if(bz_2)
			{
				bz_1=0;
				bz_2=0;
				bz_d_m=5;
				bz_delay=1;
				PWM4_Freq(392,duty_cycle);
			}else if(bz_1)
			{
				bz_1=0;
				bz_d_m=2;
				bz_delay=1;
				PWM4_Freq(494,duty_cycle);
			}
			//bz_delay
			if(bz_delay)
			{
				++bz_d_t;
			}
			if(bz_d_t>10*bz_d_m)
			{
				bz_d_t=0;
				bz_d_m=99;
				bz_delay=0;
				PWM4_Freq(0,duty_cycle);
			}
			
			//state
			for(i=(D_y)*8;i<(D_y+1)*8;++i)
			{
				draw(D_x,i,square[i-(D_y*8)]);
				draw(D_x+D.block2.offset_x,i+(D.block2.offset_y*8),square[i-(D_y*8)]);
				draw(D_x+D.block3.offset_x,i+(D.block3.offset_y*8),square[i-(D_y*8)]);
				draw(D_x+D.block4.offset_x,i+(D.block4.offset_y*8),square[i-(D_y*8)]);
			}
			
			//keyboard
			iaclear(kb,9);
			
			if(!kf_delay)
			{
				delay=n_del;
				kscanf(kb);
				kf_d_t=0;
			}
			if(kf_delay)
			{
				++kf_d_t;
			}
			if(kf_d_t>(10*delay))
			{
				kf_delay=0;
			}
			
			
			//rotation
			if(kb[4])
			{
				//clear_old_D_Position
				matter[D_y][D_x+1]=0;
				matter[D_y+D.block2.offset_y][D_x+D.block2.offset_x+1]=0;
				matter[D_y+D.block3.offset_y][D_x+D.block3.offset_x+1]=0;
				matter[D_y+D.block4.offset_y][D_x+D.block4.offset_x+1]=0;
				//test_empty_grid
				rotate();
				sum_grid=0;
				sum_grid+=(matter[D_y][D_x+1]+matter[D_y+temp_ob.block2.offset_y][D_x+temp_ob.block2.offset_x+1]+matter[D_y+temp_ob.block3.offset_y][D_x+temp_ob.block3.offset_x+1]+matter[D_y+temp_ob.block4.offset_y][D_x+temp_ob.block4.offset_x+1]);
				if(sum_grid==0)
				{
					for(i=(D_y)*8;i<(D_y+1)*8;++i)
					{
						draw(D_x,i,0);
						draw(D_x+D.block2.offset_x,i+(D.block2.offset_y*8),0);
						draw(D_x+D.block3.offset_x,i+(D.block3.offset_y*8),0);
						draw(D_x+D.block4.offset_x,i+(D.block4.offset_y*8),0);
					}
					D=temp_ob;
					for(i=(D_y)*8;i<(D_y+1)*8;++i)
					{
						draw(D_x,i,square[i-(D_y*8)]);
						draw(D_x+D.block2.offset_x,i+(D.block2.offset_y*8),square[i-(D_y*8)]);
						draw(D_x+D.block3.offset_x,i+(D.block3.offset_y*8),square[i-(D_y*8)]);
						draw(D_x+D.block4.offset_x,i+(D.block4.offset_y*8),square[i-(D_y*8)]);
					}
				}
				//D_Positon_set
				matter[D_y][D_x+1]=0x01;
				matter[D_y+D.block2.offset_y][D_x+D.block2.offset_x+1]=0x01;
				matter[D_y+D.block3.offset_y][D_x+D.block3.offset_x+1]=0x01;
				matter[D_y+D.block4.offset_y][D_x+D.block4.offset_x+1]=0x01;
			}
			
			//move
			if(kb[1])
			{
				//clear_old_D_Position
				matter[D_y][D_x+1]=0;
				matter[D_y+D.block2.offset_y][D_x+D.block2.offset_x+1]=0;
				matter[D_y+D.block3.offset_y][D_x+D.block3.offset_x+1]=0;
				matter[D_y+D.block4.offset_y][D_x+D.block4.offset_x+1]=0;
				//test_empty_grid
				sum_grid=0;
				sum_grid+=(matter[D_y][D_x]+matter[D_y+D.block2.offset_y][D_x+D.block2.offset_x]+matter[D_y+D.block3.offset_y][D_x+D.block3.offset_x]+matter[D_y+D.block4.offset_y][D_x+D.block4.offset_x]);
				if(sum_grid==0)
				{
					for(i=(D_y)*8;i<(D_y+1)*8;++i)
					{
						draw(D_x,i,0);
						draw(D_x+D.block2.offset_x,i+(D.block2.offset_y*8),0);
						draw(D_x+D.block3.offset_x,i+(D.block3.offset_y*8),0);
						draw(D_x+D.block4.offset_x,i+(D.block4.offset_y*8),0);
					}
					D_x=D_x-1;
					for(i=(D_y)*8;i<(D_y+1)*8;++i)
					{
						draw(D_x,i,square[i-(D_y*8)]);
						draw(D_x+D.block2.offset_x,i+(D.block2.offset_y*8),square[i-(D_y*8)]);
						draw(D_x+D.block3.offset_x,i+(D.block3.offset_y*8),square[i-(D_y*8)]);
						draw(D_x+D.block4.offset_x,i+(D.block4.offset_y*8),square[i-(D_y*8)]);
					}
				}
				//D_Positon_set
				matter[D_y][D_x+1]=0x01;
				matter[D_y+D.block2.offset_y][D_x+D.block2.offset_x+1]=0x01;
				matter[D_y+D.block3.offset_y][D_x+D.block3.offset_x+1]=0x01;
				matter[D_y+D.block4.offset_y][D_x+D.block4.offset_x+1]=0x01;
			}
			else if(kb[7])
			{
				//clear_old_D_Position
				matter[D_y][D_x+1]=0;
				matter[D_y+D.block2.offset_y][D_x+D.block2.offset_x+1]=0;
				matter[D_y+D.block3.offset_y][D_x+D.block3.offset_x+1]=0;
				matter[D_y+D.block4.offset_y][D_x+D.block4.offset_x+1]=0;
				//test_empty_grid
				sum_grid=0;
				sum_grid+=(matter[D_y][D_x+2]+matter[D_y+D.block2.offset_y][D_x+D.block2.offset_x+2]+matter[D_y+D.block3.offset_y][D_x+D.block3.offset_x+2]+matter[D_y+D.block4.offset_y][D_x+D.block4.offset_x+2]);
				if(sum_grid==0)
				{
					for(i=(D_y)*8;i<(D_y+1)*8;++i)
					{
						draw(D_x,i,0);
						draw(D_x+D.block2.offset_x,i+(D.block2.offset_y*8),0);
						draw(D_x+D.block3.offset_x,i+(D.block3.offset_y*8),0);
						draw(D_x+D.block4.offset_x,i+(D.block4.offset_y*8),0);
					}
					D_x=D_x+1;
					for(i=(D_y)*8;i<(D_y+1)*8;++i)
					{
						draw(D_x,i,square[i-(D_y*8)]);
						draw(D_x+D.block2.offset_x,i+(D.block2.offset_y*8),square[i-(D_y*8)]);
						draw(D_x+D.block3.offset_x,i+(D.block3.offset_y*8),square[i-(D_y*8)]);
						draw(D_x+D.block4.offset_x,i+(D.block4.offset_y*8),square[i-(D_y*8)]);
					}
				}
				//D_Positon_set
				matter[D_y][D_x+1]=0x01;
				matter[D_y+D.block2.offset_y][D_x+D.block2.offset_x+1]=0x01;
				matter[D_y+D.block3.offset_y][D_x+D.block3.offset_x+1]=0x01;
				matter[D_y+D.block4.offset_y][D_x+D.block4.offset_x+1]=0x01;
			}
			
			//quick_drop
			if(kb[5])
			{
				//clear_old_D_Position
				matter[D_y][D_x+1]=0;
				matter[D_y+D.block2.offset_y][D_x+D.block2.offset_x+1]=0;
				matter[D_y+D.block3.offset_y][D_x+D.block3.offset_x+1]=0;
				matter[D_y+D.block4.offset_y][D_x+D.block4.offset_x+1]=0;
				//test_empty_grid
				sum_grid=0;
				sum_grid+=(matter[D_y+1][D_x+1]+matter[D_y+D.block2.offset_y+1][D_x+D.block2.offset_x+1]+matter[D_y+D.block3.offset_y+1][D_x+D.block3.offset_x+1]+matter[D_y+D.block4.offset_y+1][D_x+D.block4.offset_x+1]);
				if(sum_grid==0)
				{
					for(i=(D_y)*8;i<(D_y+1)*8;++i)
					{
						draw(D_x,i,0);
						draw(D_x+D.block2.offset_x,i+(D.block2.offset_y*8),0);
						draw(D_x+D.block3.offset_x,i+(D.block3.offset_y*8),0);
						draw(D_x+D.block4.offset_x,i+(D.block4.offset_y*8),0);
					}
					D_y=D_y+1;++new_T0_v;
					
					for(i=(D_y)*8;i<(D_y+1)*8;++i)
					{
						draw(D_x,i,square[i-(D_y*8)]);
						draw(D_x+D.block2.offset_x,i+(D.block2.offset_y*8),square[i-(D_y*8)]);
						draw(D_x+D.block3.offset_x,i+(D.block3.offset_y*8),square[i-(D_y*8)]);
						draw(D_x+D.block4.offset_x,i+(D.block4.offset_y*8),square[i-(D_y*8)]);
					}
				}
				//D_Positon_set
				matter[D_y][D_x+1]=0x01;
				matter[D_y+D.block2.offset_y][D_x+D.block2.offset_x+1]=0x01;
				matter[D_y+D.block3.offset_y][D_x+D.block3.offset_x+1]=0x01;
				matter[D_y+D.block4.offset_y][D_x+D.block4.offset_x+1]=0x01;
			}
			
			
			//game_over
		/*	if(SK(2,kb[2],kb[6]))
			{
				break;
			}*/
			//glance
			if(kb[8])
			{
				if(glance)
				{
					Initial_Lcd();
					redraw();
				}
				glance=(glance+1)%2;
			}
			if(glance)
			{
				for(i=(1)*8;i<(2)*8;++i)
				{
					draw(1,i,square[i-(1*8)]);
					draw(1+nD.block2.offset_x,i+(nD.block2.offset_y*8),square[i-(1*8)]);
					draw(1+nD.block3.offset_x,i+(nD.block3.offset_y*8),square[i-(1*8)]);
					draw(1+nD.block4.offset_x,i+(nD.block4.offset_y*8),square[i-(1*8)]);
				}
			}
			
			//seg_print
			if(n_kb())
			{
				point_size=ilog(POINT);
				tGPIO_C->DOUT = seg_monitor[0];
				tGPIO_E->DOUT = seg_num[POINT%10];
				if(POINT>=10&&(circle%(point_size+1))==1)
				{
					tGPIO_E->DOUT = 0xff;
					tGPIO_C->DOUT = seg_monitor[1];
					tGPIO_E->DOUT = seg_num[(POINT/10)%10];
				}
				if(POINT>=100&&(circle%(point_size+1))==2)
				{
					tGPIO_E->DOUT = 0xff;
					tGPIO_C->DOUT = seg_monitor[2];
					tGPIO_E->DOUT = seg_num[(POINT/100)%10];
				}
				if(POINT>=1000&&(circle%(point_size+1))==3)
				{
					tGPIO_E->DOUT = 0xff;
					tGPIO_C->DOUT = seg_monitor[3];
					tGPIO_E->DOUT = seg_num[(POINT/1000)%10];
				}
			}
			
			circle=(circle+1)%1000;
		}
		Initial_Lcd();
		ELP(6,32,2);ELP(0,40,2);ELP(12,48,2);ELP(4,56,2);
		ELP(14,72,2);ELP(21,80,2);ELP(4,88,2);ELP(17,96,2);
		//game_over
		while(1)
		{
			if(restart_meter==5)
			{
				restart_meter=0;
				//restart
				matter_clear();
				new_T0_v=1;old_T0_v=0;
				itrpt_T0=0;iaclear(kb,9);rd=0;D_x=4;D_y=0;level=16;elim_y_max=0;elim_y_min=15;POINT=0;
				tGPIO_E->DOUT = 0xff;
				break;
			}
			if(itrpt_T0)
			{
				itrpt_T0=0;
				++restart_meter;
			}
		}
	}
}

//----------------------------------------------------------------------------
//  SUB function
//----------------------------------------------------------------------------
void InitTIMER0(void)
{
	/* Step 1. Enable and Select Timer clock source */          
		SYSCLK->CLKSEL1.TMR0_S = 0;	//Select 12Mhz for Timer0 clock source 
    SYSCLK->APBCLK.TMR0_EN = 1;	//Enable Timer0 clock source

	/* Step 2. Select Operation mode */	
	TIMER0->TCSR.MODE=1;		//Select periodic mode for operation mode

	/* Step 3. Select Time out period = (Period of timer clock input) * (8-bit Prescale + 1) * (24-bit TCMP)*/
	TIMER0->TCSR.PRESCALE=255;	// Set Prescale [0~255]
	TIMER0->TCMPR = 27650;		// Set TCMPR [0~16777215]
								// (1/12000000)*(255+1)*(2765)= 125.01usec or 7999.42Hz

	/* Step 4. Enable interrupt */
	TIMER0->TCSR.IE = 1;
	NVIC_EnableIRQ(TMR0_IRQn);	//Enable Timer0 Interrupt

	/* Step 5. Enable Timer module */
	TIMER0->TCSR.CRST = 1;		//Reset up counter
	TIMER0->TCSR.CEN = 1;		//Enable Timer0

  	//TIMER0->TCSR.TDR_EN=1;		// Enable TDR function
}


void TMR0_IRQHandler() // Timer0 interrupt subroutine 
{
	old_T0_v=new_T0_v;
	new_T0_v=(new_T0_v )+1;
	itrpt_T0=1;
	TIMER0->TISR.TIF =1; 	
}

uint16_t ipow(uint16_t n,uint16_t i)
{
	uint16_t j;
	if(i == 0)
	{
		return 0;
	}
	for(j=0;j<i-1;++j)
	{
		n = n*n;
	}
	return n;
}

void Initial_Lcd()
{
		uint8_t x, y;
		SYSCLK->APBCLK.SPI3_EN  =1;			 //enable spi function 
		SYS->IPRSTC2.SPI3_RST   =1;			 //reset spi function
		SYS->IPRSTC2.SPI3_RST   =0;

		/* set GPIO to SPI mode*/
		SYS->GPDMFP.SPI3_SS0 	=1;
		SYS->GPDMFP.SPI3_CLK 	=1;
		//SYS->GPDMFP.SPI3_MISO0 	=1;
		SYS->GPDMFP.SPI3_MOSI0 	=1;

		SPI_PORT[3]->CNTRL.CLKP = 1;							//CLKP HIGH IDLE
		SPI_PORT[3]->CNTRL.TX_BIT_LEN = 9;						//TX LEGTH 9
		SPI_PORT[3]->CNTRL.TX_NEG = 1;							//SET TX_NEG
		SPI_PORT[3]->DIVIDER.DIVIDER=0X03;					    //SET DIV

		SPI_PORT[3]->SSR.SSR=1;									//ENABLE SLAVE SELECT
		// Set BR
		SPI_PORT[3]->TX[0] =0xEB;
		SPI_PORT[3]->CNTRL.GO_BUSY = 1;
			while ( SPI_PORT[3]->CNTRL.GO_BUSY == 1 );
		// Set PM
		SPI_PORT[3]->SSR.SSR=0;

		SPI_PORT[3]->SSR.SSR=1;
		//outp32(SPI3_Tx0, 0x81);	
		SPI_PORT[3]->TX[0] =0x81;
		SPI_PORT[3]->CNTRL.GO_BUSY = 1;
			while ( SPI_PORT[3]->CNTRL.GO_BUSY == 1 );
		SPI_PORT[3]->TX[0] =0xa0;
		SPI_PORT[3]->CNTRL.GO_BUSY = 1;
			while ( SPI_PORT[3]->CNTRL.GO_BUSY == 1 );
		SPI_PORT[3]->SSR.SSR=0;

		SPI_PORT[3]->SSR.SSR=1;
		//outp32(SPI3_Tx0, 0xC0);
		SPI_PORT[3]->TX[0] =0xc0;	
		SPI_PORT[3]->CNTRL.GO_BUSY = 1;
			while ( SPI_PORT[3]->CNTRL.GO_BUSY == 1 );
		// Set Display Enable
		SPI_PORT[3]->SSR.SSR=0;

		SPI_PORT[3]->SSR.SSR=1;
		SPI_PORT[3]->TX[0] = 0XAF;
		SPI_PORT[3]->CNTRL.GO_BUSY = 1;
			while ( SPI_PORT[3]->CNTRL.GO_BUSY == 1 );
		for (y=0; y< 128; y++) 
		{
			for (x=0; x< 8; x++) 
			{
					SPI_PORT[3]->TX[0] = 0xB0 | x;	
					SPI_PORT[3]->CNTRL.GO_BUSY = 1;
						while ( SPI_PORT[3]->CNTRL.GO_BUSY == 1 );	 //check data out?

						
					//cloumn MSB
					SPI_PORT[3]->TX[0] =0x10 |((129-y)>>4)&0xF;
					SPI_PORT[3]->CNTRL.GO_BUSY = 1;
						while ( SPI_PORT[3]->CNTRL.GO_BUSY == 1 );	  //check data out?
							
							
					// cloumn LSB
					SPI_PORT[3]->TX[0] =0x00 | ((129-y) & 0xF);		
					SPI_PORT[3]->CNTRL.GO_BUSY = 1;
						while ( SPI_PORT[3]->CNTRL.GO_BUSY == 1 );	  //check data out?
							
					// Write Data
					//SPI_PORT[3]->SSR.SSR=1;	   //chip select
					SPI_PORT[3]->TX[0] =0x100 | 0;    	//write data
					SPI_PORT[3]->CNTRL.GO_BUSY = 1;
						while ( SPI_PORT[3]->CNTRL.GO_BUSY == 1 ); //check data out?
			}
		}
}


void draw(uint8_t x,uint8_t y,unsigned char data)
{
			SPI_PORT[3]->TX[0] = 0xB0 | x;	
			SPI_PORT[3]->CNTRL.GO_BUSY = 1;
				while ( SPI_PORT[3]->CNTRL.GO_BUSY == 1 );	 //check data out?

				
			//cloumn MSB
			SPI_PORT[3]->TX[0] =0x10 |((129-y)>>4)&0xF;
			SPI_PORT[3]->CNTRL.GO_BUSY = 1;
				while ( SPI_PORT[3]->CNTRL.GO_BUSY == 1 );	  //check data out?
					
					
			// cloumn LSB
			SPI_PORT[3]->TX[0] =0x00 | ((129-y) & 0xF);		
			SPI_PORT[3]->CNTRL.GO_BUSY = 1;
				while ( SPI_PORT[3]->CNTRL.GO_BUSY == 1 );	  //check data out?
					
			// Write Data
			SPI_PORT[3]->TX[0] =0x100 | data;    	//write data
			SPI_PORT[3]->CNTRL.GO_BUSY = 1;
				while ( SPI_PORT[3]->CNTRL.GO_BUSY == 1 ); //check data out?
}

void Lprintf(uint16_t i,uint16_t a,uint16_t b)
{
	uint16_t y;
	if(ilog(i) != 0)
	{
		Lprintf(i/10,a-8,b);
		i = i % 10;
	}
	for (y=a;y<a+8;++y)
	{
		draw(b,y,number[i*2][y-a]);
	}
	for (y=a;y<a+8;++y)
	{
		draw(b+1,y,number[i*2+1][y-a]);
	}
}

void lprintf(uint16_t i,uint16_t a,uint16_t b)
{
	uint16_t y;
	if(ilog(i) != 0)
	{
		lprintf(i/10,a-8,b);
		i = i % 10;
	}
	for (y=a;y<a+8;++y)
	{
		draw(b,y,number8x8[i][y-a]);
	}
}



void kscanf(uint16_t a[])
{
	uint16_t i,j,k,s = 0,t = 1;
	GPIO_T * tGPIO_A;
	uint16_t act[3]={0xfffb,0xfffd,0xfffe};
	uint16_t act2[3]={0x8,0x10,0x20};
	
	uint32_t u32Reg;
	uint32_t u32Reg_temp;
	
	tGPIO_A = (GPIO_T *)((uint32_t)GPIOA + (0*0x40));
	u32Reg = (uint32_t)&GPIOA->PIN + (0*0x40);
	
	while(s<t)
	{
		for(i=0;i<3;++i)
		{
			tGPIO_A->DOUT = act[i];
			u32Reg_temp = inpw(u32Reg);
			for(j=0;j<3;++j)
			{
				k = i + (j*3);
				if(((u32Reg_temp & act2[j]) == 0) && (a[k] == 0))
				{
					a[k] = 1;
					++t;
					kf_delay=1;
					if(k==4)delay=l_del;if(k==5)delay=s_del;
					/*
					if(k==4)delay=l_del;if(k==5)delay=s_del;
					tGPIO_E->DOUT = 0xff;
					DrvSYS_Delay(10000*delay);
					delay=n_del;
					*/
				}
			}
		}
		++s;
	}
}

void iacpy(uint16_t a[],uint16_t b[],uint16_t i)
{
	uint16_t j;
	for(j=0;j<i;++j)
	{
		a[j] = b[j];
	}
}

uint16_t iacmp(uint16_t a[],uint16_t b[],uint16_t i)
{
	uint16_t j,k = 0;
	for(j=0;j<i;++j)
	{
		if(a[j] == b[j])
		{
			++k;
		}
	}
	if(k == i)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void iaclear(uint16_t a[],uint16_t i)
{
	uint16_t j;
	for(j=0;j<i;++j)
	{
		a[j] = 0;
	}
}

uint16_t ilog(uint16_t i)
{
	uint16_t j = 0;
	if((i/10) != 0)
	{
		++j;
		j += ilog(i/10);
		return j;
	}
	else
	{
		return 0;
	}
}

/*
uint16_t SK(uint16_t count,...)
{
	uint16_t i;
	va_list ap;
	va_start(ap,count);
	for(i=0;i<count;++i)
	{
		if(va_arg(ap,int))
		{
			continue;
		}
		return 0;
	}
	return 1;
}*/

uint16_t ifprime(uint16_t n)
{
	uint16_t count = 0,i,j;
	for(i = 2;count!=n;++i)
	{
		++count;
		for(j=i-1;j!=1;j--)
		{
			if(!(i%j))
			{
				--count;
				break;
			}
		}
	}
	return i-1;
}

void ELP(uint16_t i,uint16_t a,uint16_t b)
{
	uint16_t y;
	for (y=a;y<a+8;++y)
	{
		draw(b,y,letter[i*2][y-a]);
	}
	for (y=a;y<a+8;++y)
	{
		draw(b+1,y,letter[i*2+1][y-a]);
	}
}

void rotate(void)
{
	short int temp;
	temp_ob=D;
	temp=temp_ob.block2.offset_x;
	temp_ob.block2.offset_x = temp_ob.block2.offset_y;temp_ob.block2.offset_y = (-1)*temp;
	temp=temp_ob.block3.offset_x;
	temp_ob.block3.offset_x = temp_ob.block3.offset_y;temp_ob.block3.offset_y = (-1)*temp;
	temp=temp_ob.block4.offset_x;
	temp_ob.block4.offset_x = temp_ob.block4.offset_y;temp_ob.block4.offset_y = (-1)*temp;
}

short int test_bingo(short int y)
{
	uint16_t i;
	for(i=1;i<=8;++i)
	{
		if(!matter[y][i])
		{
			return 0;
		}
	}
	return 1;
}

uint16_t n_kb(void)
{
	uint16_t i;
	for(i=0;i<9;++i)
	{
		if(kb[i])
		{
			return 0;
		}
	}
	return 1;
}

void elim_p(short int y)
{
	short int i,j;
	for(i=1;i<=8;++i)
	{
		for(j=y;j>=1;--j)
		{
			matter[j][i]=matter[j-1][i];
		}
	}
}

void t_draw(short int x,short int y)
{
	short int i;
	for(i=y*8;i<(y+1)*8;++i)
	{
		draw(x,i,square[i-y*8]);
	}
}

void redraw(void)
{
	short int i,j;
	for(i=0;i<16;++i)
	{
		for(j=1;j<=8;++j)
		{
			if(matter[i][j])
			{
				t_draw(j-1,i);
			}
		}
	}
}

uint16_t _rand() 
{ 
	rd_meter = rd_meter * PRIME1 + PRIME2+ipow(rd_meter,PRIME3);
	return rd_meter % RandMax; 
} 

void matter_clear(void)
{
	uint16_t i,j;
	for(i=1;i<=8;++i)
	{
		for(j=0;j<16;++j)
		{
			matter[j][i]=0x00;
		}
	}
}

void InitPWM(void)
{
//-------------------------------------------------------------------------------------
 	// Step 1. GPIO initial, 

	SYS->GPBMFP.TM3_PWM4=1; 	//set GPB_MFP11 for PWM4
	SYS->ALTMFP.PB11_PWM4=1; 	//set PB11_PWM4 for PWM4
	
	
				
//-------------------------------------------------------------------------------------
	// Step 2. Enable and Select PWM clock source

	//-------------------------------------------------------
	SYSCLK->APBCLK.PWM45_EN = 1; //Enable PWM4 & PWM5 clock
	
	//-------------------------------------------------------
	//PWM0 and PWM1 clock source select
	//00=external, 01=32.768k, 10=HCLK, 11=22.1184M
	
	SYSCLK->CLKSEL2.PWM45_S = 0; //Select 12Mhz for PWM clock source

	//-------------------------------------------------------
	//Clock input is divided by (CP + 1)

	PWMB->PPR.CP01=1;		//PWM45	
	
	//-------------------------------------------------------
	// PWM clock = clock source/(Prescaler + 1)/divider
	//PWM Timer 0 Clock Source Divider 
	//000=2, 001=4, 010=8, 011=16, 100=1
	
	PWMB->CSR.CSR0=0;		//PWM4	
				         
//-------------------------------------------------------------------------------------
	
	//PWM-Timer 0 Auto-reload/One-Shot Mode 
	//CNR and CMR will be auto-cleared after setting CH0MOD form 0 to 1.
	//0:One-shot mode, 1:Auto-load mode

	PWMB->PCR.CH0MOD=1;	//PWM4
								
	//-------------------------------------------------------
	//PWM frequency = PWMxy_CLK/[(prescale+1)*(clock divider)*(CNR+1)];
	//Duty ratio = (CMR+1)/(CNR+1).
	//CMR >= CNR: PWM output is always high.
	//CMR < CNR: PWM low width = (CNR-CMR) unit; PWM high width = (CMR+1) unit.
	//CMR = 0: PWM low width = (CNR) unit; PWM high width = 1 unit
	
	PWMB->CNR0=0xFFFF; //PWM4
	
	//-------------------------------------------------------
	//CMR determin the PWM duty

	PWMB->CMR0=0xFFFF; 	//PWM4


	//-------------------------------------------------------
	//PWM-Timer 0 Output Inverter Enable
	//Inverter->0:off, 1:on
	
	PWMB->PCR.CH0INV=0;			//PWM4 
	
	//-------------------------------------------------------
	//PWM-Timer 0 Enable (PWM timer 0 for group A and PWM timer 4 for group B
	//PWM function->0:Disable, 1:Enable
	
	PWMB->PCR.CH0EN=1;	//PWM4			
	//-------------------------------------------------------
	// Enable PWM channel 0 output to pin
	//Output to pin->0:Diasble, 1:Enable
	

	PWMB->POE.PWM0=1;	//PWM4			
	
}

void PWM4_Freq(uint32_t PWM_frequency, uint8_t PWM_duty)
{
	uint8_t  PWM_PreScaler;
	uint16_t PWM_ClockDivider;
	uint16_t CNR0, CMR0;
	uint32_t PWM_Clock;

 	if (PWM_frequency == 0) 
		PWMB->POE.PWM0=0;
	else
	{		
		PWMB->POE.PWM0=1;
		// PWM setting
		if(	SYSCLK->CLKSEL2.PWM45_S == 0)/*外部clock*/
			PWM_Clock = 12000000; // Clock source = 12 MHz
		if(	SYSCLK->CLKSEL2.PWM45_S == 3)/*內部clock*/
			PWM_Clock = 22118400; // Clock source = 22.1184MHz
		
		PWM_PreScaler = 20;    // clock is divided by (PreScaler + 1)

		PWM_ClockDivider = 2;  // 0: 1/2, 1: 1/4, 2: 1/8, 3: 1/16, 4: 1

    //PWM_Freq = PWM_Clock / (PWM_PreScaler + 1) / PWM_ClockDivider / (PWM_CNR0 + 1); 
		CNR0 = PWM_Clock / PWM_frequency / (PWM_PreScaler + 1) / PWM_ClockDivider - 1;

		// Duty Cycle = (CMR0+1) / (CNR0+1)
		CMR0 = (CNR0 +1) * PWM_duty /100  - 1;

		//PWM setting	  
		PWMB->CSR.CSR0 = 4;                // 0: 1/2, 1: 1/4, 2: 1/8, 3: 1/16, 4: 1
		PWMB->PPR.CP01 = PWM_PreScaler;    // set PreScaler
		PWMB->CNR0 = CNR0;	 			   // set CNR0
		PWMB->CMR0 = CMR0;				   // set CMR0
	}
}



