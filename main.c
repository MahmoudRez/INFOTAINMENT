/*
 * SOS.c
 *
 * Created: 10/22/2019 2:03:42 PM
 * Author: Rezk
 */ 

#include "SOS.h"
#include "INFO_LCD.h"
#include "keypad.h"

#define NUMOFQUES 5
#define ZERO 0
#define DONE 3
#define SCORING 2
#define GONE 7

typedef enum { //mapping with the keypad
	APP_NO = 2,
	APP_YES = 1,
	APP_SKIP = 3,
}answers_t;

static answers_t answers[NUMOFQUES+2] = {APP_YES, APP_YES, APP_YES, APP_YES, APP_YES,APP_YES, APP_YES };
static uint8 g_key = NOT_INITIALIZED;
static uint8 question_ptr = NOT_INITIALIZED;
static uint8 score = NOT_READY;
static uint8  LOC_u8DrawPattern[]=
			{
					0b00001,0b00010,0b00011,0b00100,0b00011,0b00100,0b00011,0b00100,//Char0
					0b00011,0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b00000, //Char1
					0b11110,0b00001,0b00000,0b00000,0b00000,0b00000,0b00000,0b00000, //Char2
					0b00000,0b11110,0b01000,0b10001,0b10010,0b10010,0b01100,0b00000, //Char3
					0b00000,0b10000,0b01110,0b00010,0b00010,0b00010,0b00010,0b00010,//char4
					0b00110,0b01000,0b10000,0b00000,0b00000,0b00000,0b00000,0b00000, //Char5
					 //Char6
					 //Char7
			};
typedef enum{
	ON,
	OFF,
	STOP,
}Flag_t;

static Flag_t next_question = ON;
static Flag_t get_answer = OFF;
static Flag_t answer_ready = OFF;

static uint8 questions[NUMOFQUES+2][17] = {"WELCOME ^_^","do you have a car","are you creazy ?", "are you married ?", "are you happy ?","Do you love banana ?", "Score:"};

void QuestionDisplayTask(void)
{
	Enum_LCDState LCD_Status = LCD_Pending;
	static uint8 LOC_ClearStatus=PENDING;
	static uint8 Question_Disp = PENDING;
	if(ON == next_question)
	{

		if(PENDING == LOC_ClearStatus)
		{
			LCD_Status=LCD_Clear();
			if(LCD_Status==LCD_Finished)
			{
				LOC_ClearStatus=FINISHED;
			}
		}
		if(READY==LOC_ClearStatus)
		{

			if(PENDING == Question_Disp)
			{
				LCD_Status= LCD_displayStringRowColumn(questions[question_ptr],0,0);
				if(LCD_Finished == LCD_Status)
				{
					Question_Disp = FINISHED;

				}

			}
			if(READY == Question_Disp)
			{
				LCD_Status = LCD_displayStringRowColumn("1.Yes 2.No 3.Skp", 1, 0);
				if(LCD_Finished == LCD_Status)
				{
					question_ptr++;
					next_question = OFF;
					get_answer = ON;
					Question_Disp = PENDING;
					LOC_ClearStatus=PENDING;
					if(NUMOFQUES+2 == question_ptr){
						question_ptr = ZERO;
						get_answer = STOP;
					}
				}

			}
		}
	}
}

void GetAnswerTask(void){

	Enum_KEYPADState state = PENDING;
	if(ON == get_answer)
	{
		state = KEYPAD_getPressedKey(&g_key);
		if(FINISHED == state)
		{
			answer_ready = ON;
		}
	}
}

void AnswerCheckTask(void)
{
	if (ON == answer_ready)
	{
		answer_ready = OFF;
		get_answer = OFF;
		if (answers[question_ptr-1] == g_key)
		{
			score++;
		}
		next_question = ON;
	}
}

void ScoreDisplay(void){
	uint8 status = PENDING;
	static uint8 LCD_gotoState = PENDING;
	if(STOP == get_answer){
		if(PENDING == LCD_gotoState)
		{
			status = LCD_gotoRowColumn(0,7);
			if(LCD_Finished == status)
			{
				LCD_gotoState = READY;
			}
		}
		else if(READY == LCD_gotoState)
		{
			status = LCD_DispChar(score +48);
			if(LCD_Finished == status){
				get_answer = STOP;
				LCD_gotoState = SCORING;
			}
		}
		else if(SCORING == LCD_gotoState){
			status = LCD_displayStringRowColumn("/5",0,8);
			if(LCD_Finished == status){
				LCD_gotoState = PENDING;
				get_answer = DONE;
			}
		}
	}
}

void BuzzerDisplay(void){
	static uint8 Buzzer_flag = OFF;
	if(DONE == get_answer){
		if(OFF == Buzzer_flag){
			DIO_Write(PIN4,HIGH);
			DIO_Write(PIN5,HIGH);
			DIO_Write(PIN6,HIGH);
			DIO_Write(PIN7,HIGH);
			DIO_Write(PIN3,HIGH);
			Buzzer_flag = ON;
		}
		else if(ON == Buzzer_flag){
			DIO_Write(PIN4,LOW);
			DIO_Write(PIN5,LOW);
			DIO_Write(PIN6,LOW);
			DIO_Write(PIN7,LOW);
			DIO_Write(PIN3,LOW);
			Buzzer_flag = OFF;
			get_answer = GONE;
		}
	}
}

int main(void)
{
	SOS_Init();
	StrTask_t KEYPAD_initTask = {KEYPAD_Init,0,2};
	StrTask_t LCD_initTask={LCD_init,0,2};
	StrTask_t Display_Question = {QuestionDisplayTask,0,2};
	StrTask_t Get_Answer={GetAnswerTask,0,200};
	StrTask_t Score_Update={AnswerCheckTask,0,20};
	StrTask_t Score_Display={ScoreDisplay,0,2};
	StrTask_t BUZZER_Display={BuzzerDisplay,0,500};

	SOS_CreateTask(&KEYPAD_initTask);
	SOS_CreateTask(&LCD_initTask);
	SOS_CreateTask(&Display_Question);
	SOS_CreateTask(&Get_Answer);
	SOS_CreateTask(&Score_Update);
	SOS_CreateTask(&Score_Display);
	SOS_CreateTask(&BUZZER_Display);

	SOS_Scheduler();
}
